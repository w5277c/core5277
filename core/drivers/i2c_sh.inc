;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.10.2024	konstantin@5277.ru			Начало(черновой, не тестировано)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_SH
.else
.set DEF__C5_DRV_I2C_SH = 1
.message "Included driver hardware I2C slave v0.1"

.include	"./core/drivers/_i2c.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_I2C_SH_LED_PORT							= 0x00	;1B-Порт индикатора активности
	.EQU	_DRV_I2C_SH_POS								= 0x01	;1B-Позиция в буфере
	.EQU	_DRV_I2C_SH_TR_LEN							= 0x02	;1B-Размер данных передачи
	.EQU	_DRV_I2C_SH_BUFFER_SIZE						= 0x03	;2B-Адрес блока буфера
	.EQU	_DRV_I2C_SH_PROC_ADDR						= 0x05	;2B-Адрес процедуры обработчика запроса
	.EQU	_DRV_I2C_SH_PROC_PID							= 0x07	;1B-PID задачи процедуры
	.EQU	_DRV_I2C_SH_BUFFER							= 0x08	;xB-Буфер

;--------------------------------------------------------
DRV_I2C_SH_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-FREQ,TEMP_L-адрес ведомого,
;X-адрес обработчика запроса,TEMP_EH-порт индикатора
;передачи(0xff-не исп),TEMP_EL-размер
;буфера(1<TEMP_EL<0x100-_DRV_I2C_SH_RAM_SIZE),
;FLAGS-PID задачи процедуры
;--------------------------------------------------------
	;Подаем питание на TWI
	_C5_POWER_ON PRTWI

	;Записываем в TWAR I2C адрес
	LSL TEMP_L
	STS TWAR,TEMP_L

	;Выделяем память
	LDI ACCUM,_DRV_I2C_SH_BUFFER
	ADD ACCUM,TEMP_EL
	MCALL C5_RAM_REALLOC

	;Записываем размер буфера
	STD Y+_DRV_I2C_SH_BUFFER_SIZE,TEMP_EL

	;Записываем адрес и PID процедуры обработки запроса
	STD Y+_DRV_I2C_SH_PROC_ADDR+0x00,XH
	STD Y+_DRV_I2C_SH_PROC_ADDR+0x01,XL
	STD Y+_DRV_I2C_SH_PROC_PID,FLAGS

	;Инициализируем порт LED
	STD Y+_DRV_I2C_SH_LED_PORT,TEMP_EH
	CPI TEMP_EH,0xff
	BREQ PC+0x01+_MCALL_SIZE*2
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Настройка порта SDA
	LDI ACCUM,SDA
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	;Настройка порта SCL
	LDI ACCUM,SCL
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	;Задаем частоту
	STS TWBR,TEMP_H
	;Prescaller x1
	LDI TEMP,(0<<TWPS1)|(0<<TWPS0)
	STS TWSR,TEMP

	LDI TEMP_H,high(_DRV_I2C_SH_EVENT)
	LDI TEMP_L,low(_DRV_I2C_SH_EVENT)
	LDI ACCUM,C5_IR_TWI
	MCALL C5_IR_VECTOR_SET

	MCALL C5_READY

;--------------------------------------------------------
;Основной код, запуск TWI
;OUT: TEMP-DRV_RESULT_OK
;--------------------------------------------------------
	LDI TEMP,(1<<TWEA)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP

	LDI TEMP,DRV_RESULT_OK
	RET

;--------------------------------------------------------
_DRV_I2C_SH_EVENT:
;--------------------------------------------------------
;Событие от шины
;--------------------------------------------------------
	PUSH_Y
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	;Получаем состояние TWI
	LDS TEMP,TWSR
	ANDI TEMP,0xf8

	;TWI проверка на старт передачи
	CPI TEMP,TWI_SR_SLA_W
	BREQ _DRV_I2C_SH_EVENT__RC_START
	CPI TEMP,TWI_SR_SLA_W_AL
	BREQ _DRV_I2C_SH_EVENT__RC_START
	CPI TEMP,TWI_SR_GC
	BREQ _DRV_I2C_SH_EVENT__RC_START
	CPI TEMP,TWI_SR_GC_AL
	BREQ _DRV_I2C_SH_EVENT__RC_START
	CPI TEMP,TWI_SR_SLA_R
	BREQ _DRV_I2C_SH_EVENT__TR_START
	CPI TEMP,TWI_SR_SLA_R_AL
	BREQ _DRV_I2C_SH_EVENT__TR_START

	CPI TEMP,TWI_SR_RC_ACK
	BREQ _DRV_I2C_SH_EVENT__RC_DATA
	CPI TEMP,TWI_SR_GC_RC_ACK
	BREQ _DRV_I2C_SH_EVENT__RC_DATA
	CPI TEMP,TWI_SR_RC_NACK
	BREQ _DRV_I2C_SH_EVENT__RC_DATA
	CPI TEMP,TWI_SR_GC_RC_NACK
	BREQ _DRV_I2C_SH_EVENT__RC_DATA

	CPI TEMP,TWI_SR_TR_ACK
	BREQ _DRV_I2C_SH_EVENT__TR_DATA

	CPI TEMP,TWI_SR_RC_STOP
	BREQ _DRV_I2C_SH_EVENT__STOP
	CPI TEMP,TWI_SR_TR_STOP
	BREQ _DRV_I2C_SH_EVENT__STOP

	RJMP _DRV_I2C_SH_EVENT__NACK

_DRV_I2C_SH_EVENT__RC_START:
	STD Y+_DRV_I2C_SH_TR_LEN,C0x00
	LDD ACCUM,Y+_DRV_I2C_SH_LED_PORT
	CPI ACCUM,0xFF
	BREQ PC+0x01+_MCALL_SIZE
	MCALL PORT_SET_HI
	STD Y+_DRV_I2C_SH_POS,C0x00
	RJMP _DRV_I2C_SH_EVENT__ACK

_DRV_I2C_SH_EVENT__TR_START:
	LDD ACCUM,Y+_DRV_I2C_SH_LED_PORT
	CPI ACCUM,0xFF
	BREQ PC+0x01+_MCALL_SIZE
	MCALL PORT_SET_HI
	STD Y+_DRV_I2C_SH_POS,C0x00
	RJMP _DRV_I2C_SH_EVENT__TR_DATA

_DRV_I2C_SH_EVENT__RC_DATA:
	LDD LOOP_CNTR,Y+_DRV_I2C_SH_POS
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LDS ACCUM,TWDR
	ST Y,ACCUM
	INC LOOP_CNTR
	STD Y+_DRV_I2C_SH_POS,LOOP_CNTR
	LDD ACCUM,Y+_DRV_I2C_SH_BUFFER_SIZE
	CP ACCUM,LOOP_CNTR
	BRNE PC+0x03
	RCALL _DRV_I2C_SH_EVENT__PROC_EXEC
	RJMP _DRV_I2C_SH_EVENT__NACK
	SBRC TEMP,0x03
	RCALL _DRV_I2C_SH_EVENT__PROC_EXEC
	RJMP _DRV_I2C_SH_EVENT__STOP

_DRV_I2C_SH_EVENT__TR_DATA:
	LDD LOOP_CNTR,Y+_DRV_I2C_SH_POS
	LDD TEMP,Y+_DRV_I2C_SH_TR_LEN
	LDI ACCUM,0xFF
	CP LOOP_CNTR,TEMP
	BREQ _DRV_I2C_SH_EVENT__TR_DATA_SKIP
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LD ACCUM,Y
	INC LOOP_CNTR
	STD Y+_DRV_I2C_SH_POS,LOOP_CNTR
_DRV_I2C_SH_EVENT__TR_DATA_SKIP:
	STS TWDR,ACCUM
	RJMP _DRV_I2C_SH_EVENT__END

_DRV_I2C_SH_EVENT__STOP:
	LDD ACCUM,Y+_DRV_I2C_SH_LED_PORT
	CPI ACCUM,0xFF
	BREQ PC+0x01+_MCALL_SIZE
	MCALL PORT_SET_LO
	RJMP _DRV_I2C_SH_EVENT__END

_DRV_I2C_SH_EVENT__ACK:
	LDS TEMP,TWCR
	ORI TEMP,(1<<TWEA)
	STS TWCR,TEMP
	RJMP _DRV_I2C_SH_EVENT__END
_DRV_I2C_SH_EVENT__NACK:
	LDS TEMP,TWCR
	ANDI TEMP,~(1<<TWEA)
	STS TWCR,TEMP
	RJMP _DRV_I2C_SH_EVENT__END

_DRV_I2C_SH_EVENT__END:
	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_Y
	RET

;--------------------------------------------------------
;Параметры обработчика:
;IN: X-буфер, Y-память задачи, LOOP-CNTR-размер данных в
;буфере
;OUT: LOOP_CNTR-размер данных ответа в буфере
;--------------------------------------------------------
_DRV_I2C_SH_EVENT__PROC_EXEC:
	PUSH_X
	PUSH_Z
	PUSH PID

	LDD LOOP_CNTR,Y+_DRV_I2C_SH_POS
	MOVW XL,YL
	ADIW XL,_DRV_I2C_SH_BUFFER
	LDD ZL,Y+_DRV_I2C_SH_PROC_ADDR+0x00
	LDD ZH,Y+_DRV_I2C_SH_PROC_ADDR+0x01
	LDD PID,Y+_DRV_I2C_SH_PROC_PID
	MCALL C5_RAM_OFFSET
	ICALL
	STD Y+_DRV_I2C_SH_TR_LEN,LOOP_CNTR

	POP PID
	POP_Z
	POP_X
	RET

.endif
