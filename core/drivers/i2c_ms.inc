;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;21.09.2021	w5277c@gmail.com			Начало, черновой(в процессе)
;13.10.2021	w5277c@gmail.com			В процессе
;-----------------------------------------------------------------------------------------------------------------------
;Программный I2C в режиме ведущего, с частотой 10/20КГц(зависит от TIMERS_SPEED_xxUS) или 100КГц(таймер C)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_MS
.else
.set DEF__C5_DRV_I2C_MS = 1
.message "Included driver software I2C(master) v0.1"

	.include	"./core/drivers/_i2c.inc"
	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./io/port_offsets.inc"
	.include	"./io/reg_bit_hi.inc"
	.include	"./io/reg_bit_lo.inc"
	.include	"./io/reg_bit_inv.inc"
	.include	"./io/reg_bit_get.inc"
	.include	"./io/reg_bit_iset.inc"
	.include	"./core/wait.inc"

	.EQU	_DRV_I2C_MR_ST_READY					= 0x00
	.EQU	_DRV_I2C_MR_ST_START					= 0x01
	.EQU	_DRV_I2C_MR_ST_DATA					= 0x02
	.EQU	_DRV_I2C_MR_ST_STOP					= 0x03

	.EQU	_DRV_I2C_MR_RC_BIT					= 0x04
	.EQU	_DRV_I2C_MR_ERROR_BIT				= 0x06
	.EQU	_DRV_I2C_MR_DONE_BIT					= 0x07

	.EQU	_DRV_I2C_MS_SDA_DA					= 0x00			;1B - Адрес DDR для SDA
	.EQU	_DRV_I2C_MS_SDA_IA					= 0x01			;1B - Адрес PIN для SDA
	.EQU	_DRV_I2C_MS_SDA_PN					= 0x02			;1B - Число пина для SDA
	.EQU	_DRV_I2C_MS_SCL_PA					= 0x03			;1B - Адрес PORT для SCL
	.EQU	_DRV_I2C_MS_SCL_PN					= 0x04			;1B - Число пина для SCL
	.EQU	_DRV_I2C_MS_STATE						= 0x05			;1B - Статус драйвера
	.EQU	_DRV_I2C_MS_TIMER_ID					= 0x06			;1B - ИД программного таймера
	.EQU	_DRV_I2C_MS_ADDR						= 0x07			;1B
	.EQU	_DRV_I2C_MS_ACCUM						= 0x08			;1B
	.EQU	_DRV_I2C_MS_TR_LEN					= 0x09			;1B
	.EQU	_DRV_I2C_MS_RC_LEN					= 0x0a			;1B
	.EQU	_DRV_I2C_MS_SRC_ADDR					= 0x0b			;2B
	.EQU	_DRV_I2C_MS_DST_ADDR					= 0x0d			;2B
	.EQU	_DRV_I2C_MS_BUFFER_POS				= 0x0f			;1B
	.EQU	_DRV_I2C_MS_BIT_NUM					= 0x10			;1B

	.EQU	_DRV_I2C_MS_RAM_SIZE					= 0x11



;--------------------------------------------------------
DRV_I2C_MS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт SDA, TEMP_L-порт SCL
;TEMP_EH-ид таймера(0x83 - используем таймер C)
;TEMP_EL-период для тамйреа С TIMER_C_FREQ_...
;(при использовании)
;--------------------------------------------------------
	LDI ACCUM,_DRV_I2C_MS_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Записываю ID таймера
	STD Y+_DRV_I2C_MS_TIMER_ID,TEMP_EH
	;Устанавливаю состояние READY
	LDI ACCUM,_DRV_I2C_MR_ST_READY
	STD Y+_DRV_I2C_MS_STATE,ACCUM

	;Инициализирую порт SDA(IN,LO)
	PUSH TEMP_L
	MOV ACCUM,TEMP_H
	MCALL PORT_OFFSETS
	STD Y+_DRV_I2C_MS_SDA_IA,TEMP
	MOV TEMP,TEMP_L
	MCALL REG_BIT_LO
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_I2C_MS_SDA_DA,TEMP_L
	STD Y+_DRV_I2C_MS_SDA_PN,ACCUM
	POP TEMP_L
	;Инициализирую порт SCL(OUT,HI)
	MOV ACCUM,TEMP_L
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	MCALL REG_BIT_HI
	STD Y+_DRV_I2C_MS_SCL_PA,TEMP_H
	STD Y+_DRV_I2C_MS_SCL_PN,ACCUM

	;Инициализирую таймер
	MOV TEMP,TEMP_EH
	LDI TEMP_H,high(_DRV_I2C_MS_EVENT)
	LDI TEMP_L,low(_DRV_I2C_MS_EVENT)
	LDI ACCUM,0x01														;0.000050-20КГц/0.000025-40КГц
	MCALL C5_TIMER_SET

	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
;TEMP_H-TX длина, TEMP_L-RX максимальная длина
;ACCUM-адрес устройства
;OUT: TEMP-результат(DRV_RESULT_x),
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	;Записываем адрес устройства
	LSL ACCUM
	STD Y+_DRV_I2C_MS_ADDR,ACCUM

	;Запминаем длину TX и RX
	STD Y+_DRV_I2C_MS_TR_LEN,TEMP_H
	STD Y+_DRV_I2C_MS_RC_LEN,TEMP_L

	;Блок передачи
	CPI TEMP_H,0x00
	BREQ _DRV_I2C_MS_PROC__TR_SKIP
	;Запоминаем SRC
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_I2C_MS_SRC_ADDR+0x00,ZH
	STD Y+_DRV_I2C_MS_SRC_ADDR+0x01,ZL
	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_I2C_MS_BUFFER_POS,C0xFF
	LDI TEMP,0x01
	STD Y+_DRV_I2C_MS_BIT_NUM,TEMP
	;Запуск передачи
	STD Y+_DRV_I2C_MS_ACCUM,ACCUM
	;SDA LO(IN,LO)
	LDD TEMP,Y+_DRV_I2C_MS_SDA_DA
	LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
	MCALL REG_BIT_HI
	LDI TEMP,_DRV_I2C_MR_ST_DATA
	STD Y+_DRV_I2C_MS_STATE,TEMP
	;Запускаю таймер
	LDD TEMP,Y+_DRV_I2C_MS_TIMER_ID
	MCALL C5_TIMER_START
	;Жду окончания передачи
_DRV_I2C_MS_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_I2C_MS_STATE
	SBRS TEMP,0x07
	BRNE _DRV_I2C_MS_PROC__SENDING_WAIT
	CPI TEMP,0x80
	BRNE _DRV_I2C_MS_PROC__RC_SKIP
_DRV_I2C_MS_PROC__TR_SKIP:
	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_I2C_MS_BUFFER_POS,C0x00
	STD Y+_DRV_I2C_MS_BIT_NUM,C0x00

	;Блок приема
	CPI TEMP_L,0x00
	BRNE PC+0x02
	RJMP _DRV_I2C_MS_PROC__RC_SKIP
	;Запоминаем DST
	STD Y+_DRV_I2C_MS_DST_ADDR+0x00,XH
	STD Y+_DRV_I2C_MS_DST_ADDR+0x01,XL
	;Включаем бит приема
	LDD TEMP,Y+_DRV_I2C_MS_ADDR
	ORI TEMP,0x01
	STD Y+_DRV_I2C_MS_ADDR,TEMP

	;Запуск приема
	LDI TEMP,DRV_I2C_STATE_RC
	STD Y+_DRV_I2C_MS_STATE,TEMP
	;Запускаю таймер
	LDD TEMP,Y+_DRV_I2C_MS_TIMER_ID
	MCALL C5_TIMER_START
	;Жду окончания приема
_DRV_I2C_MS_PROC__RECEIVING_WAIT:
	LDD TEMP,Y+_DRV_I2C_MS_STATE
	SBRS TEMP,0x07
	BRNE _DRV_I2C_MS_PROC__RECEIVING_WAIT
_DRV_I2C_MS_PROC__RC_SKIP:

	;Результат
	LDI TEMP,DRV_RESULT_OK
	LDD ACCUM,Y+_DRV_I2C_MS_STATE
	SBRC ACCUM,_DRV_I2C_MR_ERROR_BIT
	LDI TEMP,DRV_RESULT_ERROR

	POP ACCUM
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_I2C_MS_EVENT:
;--------------------------------------------------------
;Событие от таймера
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	LDD TEMP_L,Y+_DRV_I2C_MS_BIT_NUM
	LDD TEMP_EH,Y+_DRV_I2C_MS_STATE
	MOV TEMP_EL,TEMP_EH
	ANDI TEMP_EL,0x0f

	;SCL INVERT
	LDD TEMP,Y+_DRV_I2C_MS_SCL_PA
	LDD ACCUM,Y+_DRV_I2C_MS_SCL_PN
	MCALL REG_BIT_INV
	BRCC _DRV_I2C_MS_EVENT__SCL_LO
	CPI TEMP_EL,_DRV_I2C_MR_ST_STOP
	BREQ PC+0x02
	RJMP _DRV_I2C_MS_EVENT__NEXT_BIT
	;Устанавливаем состояние DONE
	ORI TEMP_EL,(1<<_DRV_I2C_MR_DONE_BIT)
	LDD TEMP,Y+_DRV_I2C_MS_TIMER_ID
	MCALL C5_TIMER_STOP
	;SDA HI(IN,LO)
	LDD TEMP,Y+_DRV_I2C_MS_SDA_DA
	LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
	MCALL REG_BIT_LO
	RJMP _DRV_I2C_MS_EVENT__END
_DRV_I2C_MS_EVENT__SCL_LO:

	CPI TEMP_EL,_DRV_I2C_MR_ST_DATA
	BRNE _DRV_I2C_MS_EVENT__NOT_TR_DATA
		;ACK checking
		CPI TEMP_L,17
		BRNE _DRV_I2C_MS_EVENT__TR_DATA_NOT_ACK_CHECKING
			;SDA HI,IN
			LDD TEMP,Y+_DRV_I2C_MS_SDA_DA
			LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
			MCALL REG_BIT_LO
			LDD TEMP,Y+_DRV_I2C_MS_SDA_IA
			LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
			MCALL REG_BIT_GET
			BRCC PC+0x03
			;NACK
			LDI TEMP_EH,_DRV_I2C_MR_ST_STOP|(1<<_DRV_I2C_MR_ERROR_BIT)
			RJMP _DRV_I2C_MS_EVENT__TICK
			;ACK
			;SDA LO
			LDD TEMP,Y+_DRV_I2C_MS_SDA_DA
			LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
			MCALL REG_BIT_HI
			LDD ACCUM,Y+_DRV_I2C_MS_BUFFER_POS
			;Есть еще данные?
			LDD TEMP,Y+_DRV_I2C_MS_TR_LEN
			CP ACCUM,TEMP
			BRNE PC+0x04
			ANDI TEMP_EH,0xf0
			ORI TEMP_EH,_DRV_I2C_MR_ST_STOP
			RJMP _DRV_I2C_MS_EVENT__END
			INC ACCUM
			STD Y+_DRV_I2C_MS_BUFFER_POS,ACCUM
			LDD ZH,Y+_DRV_I2C_MS_SRC_ADDR+0x00
			LDD ZL,Y+_DRV_I2C_MS_SRC_ADDR+0x01
			MOV TEMP,ZH
			ANDI ZH,0x7f
			ADD ZL,ACCUM
			ADC ZH,C0x00
			;Считываем с буфера байт и передаем
			SBRS TEMP,0x07
			LD ACCUM,Z
			SBRC TEMP,0x07
			LPM ACCUM,Z
			STD Y+_DRV_I2C_MS_ACCUM,ACCUM
			LDI TEMP_L,0xff
			RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__TR_DATA_NOT_ACK_CHECKING:
		;Databits transfer
		MCALL _DRV_I2C_MS__TR_BIT
		RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_TR_DATA:

	CPI TEMP_EH,_DRV_I2C_MR_ST_DATA|(1<<_DRV_I2C_MR_RC_BIT)
	BRNE _DRV_I2C_MS_EVENT__NOT_RC_DATA
	;TODO...
	RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_RC_DATA:


	;Неизвестное состояние
	LDI TEMP_EH,(1<<_DRV_I2C_MR_ERROR_BIT)
	RJMP _DRV_I2C_MS_EVENT__END
_DRV_I2C_MS_EVENT__TICK:
	;SCL HI
	LDD TEMP,Y+_DRV_I2C_MS_SCL_PA
	LDD ACCUM,Y+_DRV_I2C_MS_SCL_PN
	MCALL REG_BIT_LO
_DRV_I2C_MS_EVENT__NEXT_BIT:
	INC TEMP_L
_DRV_I2C_MS_EVENT__END:
	STD Y+_DRV_I2C_MS_STATE,TEMP_EH
	STD Y+_DRV_I2C_MS_BIT_NUM,TEMP_L

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_EL
	POP TEMP_EH
	RET

;--------------------------------------------------------
_DRV_I2C_MS__TR_BIT:
;--------------------------------------------------------
;Передаем бит
;--------------------------------------------------------
	LDD TEMP,Y+_DRV_I2C_MS_SDA_DA
	LDD ACCUM,Y+_DRV_I2C_MS_SDA_PN
	LDD ZH,Y+_DRV_I2C_MS_ACCUM
	LSL ZH
	MCALL REG_BIT_ISET
	STD Y+_DRV_I2C_MS_ACCUM,ZH
	RET

.endif
