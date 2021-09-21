;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;21.09.2021	w5277c@gmail.com			Начало, черновой.
;-----------------------------------------------------------------------------------------------------------------------
;Программный I2C в режиме ведущего, с частотой 10/20КГц(зависит от TIMERS_SPEED_xxUS)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_MS
.else
.set DEF__C5_DRV_I2C_MS = 1
.message "Included driver software I2C(master) v0.1"

.include	"./core/drivers/_i2c.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./core/wait.inc"


	.EQU	_DRV_I2C_MR_ST_DONE					= 0x00
	.EQU	_DRV_I2C_MR_ST_START					= 0x01
	.EQU	_DRV_I2C_MR_ST_DATA					= 0x02
	.EQU	_DRV_I2C_MR_ST_STOP_LO				= 0x03
	.EQU	_DRV_I2C_MR_ST_STOP_HI				= 0x04

	.EQU	_DRV_I2C_MR_RC_BIT					= 0x04
	.EQU	_DRV_I2C_MR_ERROR_BIT				= 0x07


	.EQU	_DRV_I2C_MS_SDA_PORT					= DRV_I2C_SDA_PORT	;1B - SDA порт
	.EQU	_DRV_I2C_MS_SCL_PORT					= DRV_I2C_SCL_PORT	;1B - SCL порт
	.EQU	_DRV_I2C_MS_STATE						= 0x02			;1B - Статус драйвера
	.EQU	_DRV_I2C_MS_TIMER_ID					= 0x03			;1B - ИД программного таймера
	.EQU	_DRV_I2C_MS_ADDR						= 0x04			;1B
	.EQU	_DRV_I2C_MS_ACCUM						= 0x05			;1B
	.EQU	_DRV_I2C_MS_TR_LEN					= 0x06			;1B
	.EQU	_DRV_I2C_MS_RC_LEN					= 0x07			;1B
	.EQU	_DRV_I2C_MS_SRC_ADDR					= 0x08			;2B
	.EQU	_DRV_I2C_MS_DST_ADDR					= 0x0a			;2B
	.EQU	_DRV_I2C_MS_BUFFER_POS				= 0x0c			;1B
	.EQU	_DRV_I2C_MS_BIT_NUM					= 0x0d			;1B

	.EQU	_DRV_I2C_MS_RAM_SIZE					= 0x0e



;--------------------------------------------------------
DRV_I2C_MS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт SDA, TEMP_L-порт SCL
;TEMP_EH-ид таймера
;--------------------------------------------------------
	LDI ACCUM,_DRV_I2C_MS_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Записываю ID таймера
	STD Y+_DRV_I2C_MS_TIMER_ID,TEMP_EH
	;Устанавливаю состояние READY
	LDI ACCUM,DRV_I2C_STATE_DONE
	STD Y+_DRV_I2C_MS_STATE,ACCUM
	;Инициализирую порт SDA
	MOV ACCUM,TEMP_H
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	STD Y+_DRV_I2C_MS_SDA_PORT,ACCUM
	;Инициализирую порт SCL
	MOV ACCUM,TEMP_L
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	STD Y+_DRV_I2C_MS_SCL_PORT,ACCUM
	;Записываю период таймера
	STD Y+_DRV_I2C_MS_FREQ,TEMP_EL

	;Инициализирую таймер
	MOV TEMP,TEMP_EH
	LDI TEMP_H,high(_DRV_I2C_MS_EVENT)
	LDI TEMP_L,low(_DRV_I2C_MS_EVENT)
	LDI ACCUM,0x01														;0.000050-10КГц/0.000025-20КГц
	MCALL C5_TIMER_SET

	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
;TEMP_H-TX длина, TEMP_L-RX максимальная длина
;ACCUM-адрес устройства
;OUT: TEMP_H-результат(DRV_I2C_STATE_xx),
;TEMP_L-длина полученных данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	;Записываем адрес устройства
	LSL ACCUM
	STD Y+_DRV_I2C_MS_ADDR,ACCUM

	;Блок передачи
	CPI TEMP_H,0x00
	BREQ _DRV_I2C_MS_PROC__TR_SKIP
	;Запминаем длину TX и RX
	STD Y+_DRV_I2C_MS_TR_LEN,TEMP_H
	STD Y+_DRV_I2C_MS_RC_LEN,TEMP_L
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
	STD Y+_DRV_I2C_MS_BIT_NUM,C0x00
	;Запуск передачи
	LDI TEMP,_DRV_I2C_MR_ST_TR_START
	STD Y+_DRV_I2C_MS_STATE,TEMP
	;Запускаю таймер
	LDD TEMP,Y+_DRV_I2C_MS_TIMER_ID
	MCALL C5_TIMER_START
	;Жду окончания передачи
_DRV_I2C_MS_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_I2C_MS_STATE
	CPI TEMP,DRV_I2C_STATE_DONE
	BRNE _DRV_I2C_MS_PROC__SENDING_WAIT
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
	CPI TEMP,DRV_I2C_STATE_DONE
	BRNE _DRV_I2C_MS_PROC__RECEIVING_WAIT
_DRV_I2C_MS_PROC__RC_SKIP:

	;Результат
	LDD TEMP_H,Y+_DRV_UART_S_STATE
	LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS

	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_I2C_MS_EVENT:
;--------------------------------------------------------
;Событие от таймера
;--------------------------------------------------------
	;Обработчики программных таймеров могут не сохранять значения регистров ZH,ZL,TEMP,ACCUM
	PUSH_Y
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH FLAGS

	MCALL C5_RAM_OFFSET

	LDD FLAGS,Y+_DRV_I2C_MS_STATE
	MOV TEMP,FLAGS
	ANDI TEMP,0x0f
	LDD TEMP_H,Y+_DRV_I2C_MS_BUFFER_POS
	LDD TEMP_L,Y+_DRV_I2C_MS_BIT_NUM
	LDD TEMP_EH,Y+_DRV_I2C_MS_SDA_PORT
	LDD TEMP_EL,Y+_DRV_I2C_MS_SCL_PORT

	;SCL LO
	SBRS TEMP_L,0x00
	RJMP PC+0x05
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_LO
	RJMP __DRV_I2C_MS_EVENT__NEXT_BIT

	;START
	CPI TEMP,_DRV_I2C_MR_ST_START
	BRNE _DRV_I2C_MS_EVENT__NOT_START
		CPI TEMP_L,0
		BRNE _DRV_I2C_MS_EVENT__START_NOT_STARTBIT
			;START bit(SDA LO)
			MOV ACCUM,TEMP_EH
			MCALL PORT_MODE_OUT
			LDD TEMP,Y+_DRV_I2C_MS_ADDR
			STD Y+_DRV_I2C_MS_ACCUM,TEMP
			RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__START_NOT_STARTBIT:
		;ACK checking
		CPI TEMP_L,18
		BRNE _DRV_I2C_MS_EVENT__START_NOT_ACK_CHECKING
			MOV ACCUM,TEMP_EH
			MCALL PORT_GET
			BRCS PC+0x03
			;NACK
			LDI FLAGS,_DRV_I2C_MR_ST_STOP_LO|(1<<_DRV_I2C_MR_ERROR_BIT)
			RJMP __DRV_I2C_MS_EVENT__END
			;ACK
			ANDI FLAGS,0xf0
			ORI FLAGS,_DRV_I2C_MR_ST_DATA
			CLR TEMP_L
			RJMP __DRV_I2C_MS_EVENT__END

_DRV_I2C_MS_EVENT__START_NOT_ACK_CHECKING:
		;Databits transfer
		MCALL _DRV_I2c_MS__TR_BIT
		RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_START:

	CPI FLAGS,_DRV_I2C_MR_ST_DATA
	BRNE _DRV_I2C_MS_EVENT__NOT_TR_DATA
		;ACK checking
		CPI TEMP_L,16
		BRNE _DRV_I2C_MS_EVENT__TR_DATA_NOT_ACK_CHECKING
			MOV ACCUM,TEMP_EH
			MCALL PORT_GET
			BRCC PC+0x03
			;NACK
			LDI FLAGS,_DRV_I2C_MR_ST_STOP_LO|(1<<_DRV_I2C_MR_ERROR_BIT)
			RJMP __DRV_I2C_MS_EVENT__END
			;ACK
			INC TEMP_H
			;Есть еще данные?
			LDD TEMP,Y+_DRV_I2C_MS_TR_LEN
			CP TEMP_H,TEMP
			BRNE PC+0x04
			ANDI FLAGS,0xf0
			ORI FLAGS,_DRV_I2C_MR_ST_STOP_LO
			RJMP __DRV_I2C_MS_EVENT__END
			;Есть, актуализируем аккумулятор
			LDD ZH,Y+_DRV_I2C_MS_SRC_ADDR+0x00
			LDD ZL,Y+_DRV_I2C_MS_SRC_ADDR+0x01
			MOV ACCUM,ZH
			ANDI ZH,0x7f
			ADD ZL,TEMP_H
			ADC ZH,C0x00
			;Считываем с буфера байт и передаем
			SBRS ACCUM,0x07
			LD TEMP,Z
			SBRC ACCUM,0x07
			LPM TEMP,Z
			STD Y+_DRV_I2C_MS_ACCUM,TEMP
			RJMP __DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__TR_DATA_NOT_ACK_CHECKING:
		;Databits transfer
		MCALL _DRV_I2c_MS__TR_BIT
		RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_TR_DATA:

	CPI FLAGS,_DRV_I2C_MR_ST_DATA|(1<<_DRV_I2C_MR_RC_BIT)
	BRNE _DRV_I2C_MS_EVENT__NOT_RC_DATA
	;TODO...
	RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_RC_DATA:

	CPI TEMP,_DRV_I2C_MR_ST_STOP_LO
	BRNE _DRV_I2C_MS_EVENT__NOT_STOP_LO
	MOV ACCUM,TEMP_EH
	MCALL PORT_MODE_OUT
	ANDI FLAGS,0xf0
	ORI FLAGS,_DRV_I2C_MR_ST_STOP_HI
	RJMP _DRV_I2C_MS_EVENT__TICK
_DRV_I2C_MS_EVENT__NOT_STOP_LO:

	CPI TEMP,_DRV_I2C_MR_ST_STOP_HI
	BRNE _DRV_I2C_MS_EVENT__NOT_STOP_HI
	MOV ACCUM,TEMP_EH
	MCALL PORT_MODE_IN
	;Устанавливаем состояние DONE
	ANDI FLAGS,0x80
	LDD TEMP,Y+_DRV_I2C_MS_TIMER_ID
	MCALL C5_TIMER_STOP
	RJMP __DRV_I2C_MS_EVENT__END
_DRV_I2C_MS_EVENT__NOT_STOP_HI:

	;Неизвестное состояние
	LDI FLAGS,(1<<_DRV_I2C_MR_ERROR_BIT)
	RJMP __DRV_I2C_MS_EVENT__END
_DRV_I2C_MS_EVENT__TICK:
	;SCL HI
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_HI
__DRV_I2C_MS_EVENT__NEXT_BIT:
	INC TEMP_L
__DRV_I2C_MS_EVENT__END:
	STD Y+_DRV_I2C_MS_STATE,FLAGS
	STD Y+_DRV_I2C_MS_BUFFER_POS,TEMP_H
	STD Y+_DRV_I2C_MS_BIT_NUM,TEMP_L

	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Y
	RET

;--------------------------------------------------------
_DRV_I2c_MS__TR_BIT:
;--------------------------------------------------------
;Передаем бит
;--------------------------------------------------------
	MOV ACCUM,TEMP_EH
	LDD TEMP,Y+_DRV_I2C_MS_ACCUM
	SBRS TEMP,0x07
	RJMP PC+0x04
	MCALL PORT_MODE_OUT
	RJMP PC+0x03
	MCALL PORT_MODE_IN
	LSL TEMP
	STD Y+_DRV_I2C_MS_ACCUM,TEMP
	RET
