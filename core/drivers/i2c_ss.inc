;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.10.2024	konstantin@5277.ru			Начало, в процессе разработки
;-----------------------------------------------------------------------------------------------------------------------
;Программный I2C в режиме ведомого, с частотой 10/20КГц
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_SS
.else
.set DEF__C5_DRV_I2C_SS = 1
.message "Included driver software I2C(slave) v0.1"

	.EQU	_DRV_I2C_SS_ST_READY					= 0x00
	.EQU	_DRV_I2C_SS_ST_SEND					= 0x02
	.EQU	_DRV_I2C_SS_ST_SEND_DATA			= 0x03
	.EQU	_DRV_I2C_SS_ST_RECV					= 0x04
	.EQU	_DRV_I2C_SS_ST_RECV_DATA			= 0x05

	.EQU	_DRV_I2C_SS_ADDR						= 0x00			;1B-Адрес устройства
	.EQU	_DRV_I2C_SS_SCL_PORT					= 0x01			;1B-SCL порт
	.EQU	_DRV_I2C_SS_SDA_DA					= 0x02			;1B-Адрес DDR для SDA
	.EQU	_DRV_I2C_SS_SDA_IA					= 0x03			;1B-Адрес PIN для SDA
	.EQU	_DRV_I2C_SS_SDA_PN					= 0x04			;1B-Число пина для SDA
	.EQU	_DRV_I2C_SS_STATE						= 0x05			;1B-Статус драйвера
	.EQU	_DRV_I2C_SS_BUFFER_ADDR				= 0x06			;2B-Адрес буфера приема/передачи
	.EQU	_DRV_I2C_SS_LEN						= 0x08			;1B-Длина данных в буфере
	.EQU	_DRV_I2C_SS_ACCUM						= 0x09			;1B-Аккумулятор

	.EQU	_DRV_I2C_SS_RAM_SIZE					= 0x0a
;--------------------------------------------------------
DRV_I2C_SS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт SDA, TEMP_L-порт SCL,
;TEMP_EH-Адрес устройства
;TEMP_EL-ид драйвера PCINT или
;прерывание C5_IR_INTn(с выключенным 0x07 битом)
;--------------------------------------------------------
	LDI ACCUM,_DRV_I2C_SS_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_I2C_SS_ADDR,TEMP_EH

	;Устанавливаю состояние READY
	LDI ACCUM,_DRV_I2C_SS_ST_READY
	STD Y+_DRV_I2C_SS_STATE,ACCUM

	;Инициализирую порт SCL(IN,LO)
	MOV ACCUM,TEMP_L
	STD Y+_DRV_I2C_SS_SCL_PORT,ACCUM
	MCALL PORT_MODE_LO
	MCALL PORT_SET_LO

	;Инициализирую порт SDA(IN,LO)
	PUSH TEMP_L
	MOV ACCUM,TEMP_H
	MCALL PORT_OFFSETS
	STD Y+_DRV_I2C_SS_SDA_IA,TEMP
	MOV TEMP,TEMP_L
	MCALL REG_BIT_LO
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_I2C_SS_SDA_DA,TEMP_L
	STD Y+_DRV_I2C_SS_SDA_PN,ACCUM
	POP TEMP_L

	MOV TEMP,TEMP_EL
	;Инициализация прерывания для SCL
	SBRC TEMP,0x07
	RJMP DRV_I2C_SS_INIT__PCINT_ENABLE
	MOV TEMP_H,TEMP
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	LDI_Y _DRV_I2C_SS_PROC__EINT_EVENT
	MCALL C5_EINT_ENABLE
	RJMP DRV_I2C_SS_INIT__SEND
DRV_I2C_SS_INIT__PCINT_ENABLE:
	MOV ACCUM,TEMP_L
	LDI_X _DRV_I2C_SS_PROC__PCINT_EVENT
	MOV FLAGS,PID
	MCALL C5_EXEC
DRV_I2C_SS_INIT__SEND:
	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
;TEMP_EH-SEND длина,TEMP_EL-RECV максимальная длина
;OUT: TEMP-DRV_RESULT_OK/ERROR
;TEMP_L-длина полученных данных
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	LDI TEMP,_DRV_I2C_SS_ST_READY
	STD Y+_DRV_I2C_SS_STATE,TEMP

	;Блок передачи
_DRV_I2C_SS_PROC__SEND:
	CPI TEMP_EH,0x00
	BREQ _DRV_I2C_SS_PROC__RECV

	STD Y+_DRV_I2C_SS_LEN,TEMP_EH
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_I2C_SS_BUFFER_ADDR+0x00,ZH
	STD Y+_DRV_I2C_SS_BUFFER_ADDR+0x01,ZL
	;Сброс позиции в буфере
	STD Y+_DRV_I2C_SS_BUFFER_POS,C0x00
	STD Y+_DRV_I2C_SS_BIT_NUM,C0xFF
	LDI TEMP,_DRV_I2C_SS_ST_SENDING
	STD Y+_DRV_I2C_SS_STATE,TEMP
_DRV_I2C_SS_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_I2C_SS_STATE
	CPI TEMP,_DRV_I2C_SS_ST_READY
	BRNE _DRV_I2C_SS_PROC__SENDING_WAIT

	;Блок приема
_DRV_I2C_SS_PROC__RECV:
	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_I2C_SS_BUFFER_POS,C0x00
	STD Y+_DRV_I2C_SS_BIT_NUM,C0x00
	LDI TEMP_L,0x00

	CPI TEMP_EL,0x00
	BREQ _DRV_I2C_SS_PROC__END

	STD Y+_DRV_I2C_SS_LEN,TEMP_EL
	;Запоминаем DST
	STD Y+_DRV_I2C_SS_BUFFER_ADDR+0x00,XH
	STD Y+_DRV_I2C_SS_BUFFER_ADDR+0x01,XL
	;Запуск приема
	LDI TEMP,_DRV_I2C_SS_RECV
	STD Y+_DRV_I2C_SS_STATE,TEMP
_DRV_I2C_SS_PROC__RECV_WAIT:
	LDD TEMP,Y+_DRV_I2C_SS_STATE
	CPI TEMP,_DRV_I2C_SS_ST_READY
	BRNE _DRV_I2C_SS_PROC__RECV_WAIT
	;Результат
	LDD TEMP_L,Y+_DRV_I2C_SS_BUFFER_POS

_DRV_I2C_SS_PROC__END:
	POP ACCUM
	POP_T16
	POP Y
	RET

;--------------------------------------------------------
_DRV_I2C_SS_PCINT_EVENT:
;--------------------------------------------------------
;Обработчик смены состояния пина(PCINT/EINT)
;Первый бит FLAGS содержит состояние пина
;--------------------------------------------------------
	SBRC FLAGS,0x00
	RET

_DRV_I2C_SS_EINT_EVENT:
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_I2C_SS_SDA_IA
	LDD ACCUM,Y_DRV_I2C_SS_SDA_PN
	MCALL REG_BIT_GET
	ROL FLAGS

	LDD TEMP,Y+_DRV_I2C_SS_STATE
	CPI TEMP,_DRV_I2C_SS_ST_READY
	BREQ _DRV_I2C_SS_INT_EVENT__END


	CPI TEMP,_DRV_I2C_SS_ST_SEND
	BRNE _DRV_I2C_SS_INT_EVENT__NO_SEND
	SBRS FLAGS,0x00
	RJMP _DRV_I2C_SS_INT_EVENT__ERROR
	LDI TEMP,_DRV_I2C_SS_ST_SENDING
	STD Y+_DRV_I2C_SS_STATE,TEMP
	STD Y+_DRV_I2C_SS_BIT_NUM,C0x00
	RJMP _DRV_I2C_SS_INT_EVENT__NEXT
_DRV_I2C_SS_INT_EVENT__NO_SEND:
	CPI TEMP,_DRV_I2C_SS_ST_SEND_OCTET
	BRNE _DRV_I2C_SS_INT_EVENT__NO_SEND
	LDD TEMP_L,Y+_DRV_I2C_SS_BIT_NUM
	CPI TEMP_L,0x00
	BRNE _DRV_I2C_SS_INT_EVENT__READ_OCTET_SKIP
	LDD ZH,Y+_DRV_I2C_SS_BUFFER_ADDR+0x00
	LDD ZL,Y+_DRV_I2C_SS_BUFFER_ADDR+0x01
	LDD TEMP,Y+_DRV_I2C_SS_BUFFER_POS
	ADD ZL,TEMP
	ADC ZH,C0x00
	LD ACCUM,Z
	ST Y+_DRV_I2C_SS_ACCUM,ACCUM
_DRV_I2C_SS_INT_EVENT__READ_OCTET_SKIP:
	LDD ACCUM,Y+_DRV_I2C_SS_ACCUM
	;TODO SET PORT
	LSL ACCUM
	STD Y+_DRV_I2C_SS_ACCUM,ACCUM
	RJMP _DRV_I2C_SS_INT_EVENT__NEXT




	STD Y+_DRV_I2C_SS_BUFFER_POS,TEMP_H
	STD Y+_DRV_I2C_SS_BIT_NUM,TEMP_L

_DRV_I2C_SS_INT_EVENT__END:
	POP FLAGS
	POP ACCUM
	POP TEMP
	POP_Y
	RET
