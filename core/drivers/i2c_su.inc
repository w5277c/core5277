;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.10.2024	konstantin@5277.ru			Начало(черновой, не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
;Based on AVR312: Using the USI Module as a I2C Slave
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_SU
.else
.set DEF__C5_DRV_I2C_SU = 1
.message "Included driver I2C(USI) slave v0.1"

.include	"./core/drivers/_i2c.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_I2C_SU_ST_ADDRESS						= 0x00
	.EQU	_DRV_I2C_SU_ST_RC_ACK						= 0x01
	.EQU	_DRV_I2C_SU_ST_TR_FIRST						= 0x02
	.EQU	_DRV_I2C_SU_ST_RC_NEXT						= 0x03
	.EQU	_DRV_I2C_SU_ST_TR_CHECK_ACK				= 0x04
	.EQU	_DRV_I2C_SU_ST_TR_NEXT						= 0x05
	;---VARS---
	.EQU	_DRV_I2C_SU_ADDR								= 0x00	;1B-Адрес устройства
	.EQU	_DRV_I2C_SU_POS								= 0x01	;1B-Позиция в буфере
	.EQU	_DRV_I2C_SU_TR_LEN							= 0x02	;1B-Размер данных передачи
	.EQU	_DRV_I2C_SU_BUFFER_SIZE						= 0x03	;2B-Адрес блока буфера
	.EQU	_DRV_I2C_SU_PROC_ADDR						= 0x05	;2B-Адрес процедуры обработчика запроса
	.EQU	_DRV_I2C_SU_PROC_PID							= 0x07	;1B-PID задачи процедуры
	.EQU	_DRV_I2C_SU_STATE								= 0x08	;1B-Статус
	.EQU	_DRV_I2C_SU_BUFFER							= 0x09	;xB-Буфер

;--------------------------------------------------------
DRV_I2C_SU_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-адрес ведомого,
;X-адрес обработчика запроса,TEMP_L-размер
;буфера(1<TEMP_EL<0x100-_DRV_I2C_SH_RAM_SIZE),
;FLAGS-PID задачи процедуры
;--------------------------------------------------------
	;Подаем питание на USI
	_C5_POWER_ON PRUSI

	;Выделяем память
	LDI ACCUM,_DRV_I2C_SU_BUFFER
	ADD ACCUM,TEMP_L
	MCALL C5_RAM_REALLOC

	;Записываем в I2C адрес
	LSL TEMP_H
	STD Y+_DRV_I2C_SU_ADDR,TEMP_H

	;Записываем размер буфера
	STD Y+_DRV_I2C_SU_BUFFER_SIZE,TEMP_L

	;Записываем адрес и PID процедуры обработки запроса
	STD Y+_DRV_I2C_SU_PROC_ADDR+0x00,XH
	STD Y+_DRV_I2C_SU_PROC_ADDR+0x01,XL
	STD Y+_DRV_I2C_SU_PROC_PID,FLAGS

	MCALL C5_READY

;--------------------------------------------------------
;Основной код, запуск TWI
;OUT: TEMP-DRV_RESULT_OK
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_T16

	;Настройка порта SDA
	LDI ACCUM,SDA
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	;Настройка порта SCL
	LDI ACCUM,SCL
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	LDI TEMP,_DRV_I2C_SU_ST_ADDRESS
	STD Y+_DRV_I2C_SU_STATE,TEMP

	;USIOIE:Counter Overflow Interrupt Enable
	;USIWM1,USIWM0:Two-wire mode. Uses SDA and SCL pins. SCL line is also held low into interrupt.
	;USICS1,USICS0:Shift Register-External, negative edge. 4-bit Counter-External, both edges
	LDI TEMP,(0<<USISIE)|(1<<USIOIE)|(1<<USIWM1)|(1<<USIWM0)|(1<<USICS1)|(1<<USICS1)|(0<USICLK)|(0<USITC)
	STS USICR,TEMP														;

	LDI TEMP_H,high(_DRV_I2C_SU_EVENT)
	LDI TEMP_L,low(_DRV_I2C_SU_EVENT)
	LDI ACCUM,C5_IR_USI_OVF
	MCALL C5_IR_VECTOR_SET

	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00)
	STS USISR,TEMP

	LDI TEMP,DRV_RESULT_OK

	POP_T16
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_I2C_SU_EVENT:
;--------------------------------------------------------
;Событие от шины
;--------------------------------------------------------
	PUSH_Y
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_I2C_SU_STATE

	CPI TEMP,_DRV_I2C_SU_ST_ADDRESS
	BRNE _DRV_I2C_SH_EVENT__NO_START
	LDS ACCUM,USIDR
	MOV TEMP,ACCUM
	ANDI ACCUM,0xFE
	LDD TEMP,Y+_DRV_I2C_SU_ADDR
	CP ACCUM,TEMP
	BRNE _DRV_I2C_SH_EVENT__RESET
	ANDI TEMP,0x01
	INC TEMP
	STD Y+_DRV_I2C_SU_STATE,TEMP
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x0E)
	STS USISR,TEMP
	STS USIDR,C0x00
	LDI ACCUM,SDA														;ACK
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SH_EVENT__NO_START:

	;---RECEIVE---
	CPI TEMP,_DRV_I2C_SU_ST_RC_ACK
	BRNE _DRV_I2C_SH_EVENT__NO_RC_ACK
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00)
	STS USISR,TEMP
	LDI ACCUM,SDA
	MCALL PORT_MODE_IN
	STD Y+_DRV_I2C_SU_TR_LEN,C0x00
	LDI TEMP,_DRV_I2C_SU_ST_RC_NEXT
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SH_EVENT__NO_RC_ACK:
	CPI TEMP,_DRV_I2C_SU_ST_RC_NEXT
	BRNE _DRV_I2C_SU_ST_NO_RC_NEXT
	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	LDD TEMP,Y+_DRV_I2C_SU_BUFFER_SIZE
	CP LOOP_CNTR,TEMP
	BREQ _DRV_I2C_SH_EVENT__RESET
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LDS TEMP,USIDR
	STD Y+_DRV_I2C_SU_BUFFER,TEMP
	INC LOOP_CNTR
	STD Y+_DRV_I2C_SU_POS,LOOP_CNTR
	LDI ACCUM,SDA
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	LDI TEMP,_DRV_I2C_SU_ST_RC_ACK
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SU_ST_NO_RC_NEXT:

_DRV_I2C_SH_EVENT__RESET:
	LDI ACCUM,SDA														;NACK(SDA в начальное положение)
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	LDS TEMP,USICR
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00)
	STS USISR,TEMP
	ANDI TEMP,low(~(1<<USISIF))
	STS USICR,TEMP

	LDI TEMP,_DRV_I2C_SU_ST_ADDRESS
	STD Y+_DRV_I2C_SU_STATE,TEMP
_DRV_I2C_SU_EVENT__END:
	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_Y
	RET

	;---TRANSFER---
	CPI TEMP,_DRV_I2C_SU_ST_TR_FIRST
	BRNE _DRV_I2C_SU_ST_NO_TR_FIRST
	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	PUSH_X
	MOVW XL,YL
	ADIW XL,_DRV_I2C_SU_BUFFER
	PUSH_Z
	LDD ZL,Y+_DRV_I2C_SU_PROC_ADDR+0x00
	LDD ZH,Y+_DRV_I2C_SU_PROC_ADDR+0x01
	LDD PID,Y+_DRV_I2C_SU_PROC_PID
	PUSH_Y
	MCALL C5_RAM_OFFSET
	ICALL																	;--------------------------------------------------------
	POP_Y																	;Параметры обработчика:
	POP_Z																	;IN: X-буфер, Y-память задачи, LOOP-CNTR-размер данных в
	POP_X																	;буфере
	CPI LOOP_CNTR,0x00												;OUT: LOOP_CNTR-размер данных ответа в буфере
	BREQ _DRV_I2C_SH_EVENT__RESET									;--------------------------------------------------------
	STD Y+_DRV_I2C_SU_TR_LEN,LOOP_CNTR
	STD Y+_DRV_I2C_SU_POS,C0x00
	LDD TEMP,Y+_DRV_I2C_SU_BUFFER
	STS USIDR,TEMP
	LDI TEMP,0x01
	STD Y+_DRV_I2C_SU_POS,TEMP
	LDI ACCUM,SDA
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00)
	STS USISR,TEMP
	LDI TEMP,_DRV_I2C_SU_ST_TR_CHECK_ACK
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SU_ST_NO_TR_FIRST:
	CPI TEMP,_DRV_I2C_SU_ST_TR_CHECK_ACK
	BRNE _DRV_I2C_SU_ST_NO_TR_CHECK_ACK
	LDI ACCUM,SDA
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x0E)
	STS USISR,TEMP
	STS USIDR,C0xFF
	LDI TEMP,_DRV_I2C_SU_ST_TR_NEXT
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SU_ST_NO_TR_CHECK_ACK:
	CPI TEMP,_DRV_I2C_SU_ST_TR_NEXT
	BREQ PC+0x02
	RJMP _DRV_I2C_SH_EVENT__RESET
	LDS ACCUM,USIDR
	SBRC ACCUM,0x00
	RJMP _DRV_I2C_SH_EVENT__RESET
	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	LDD TEMP,Y+_DRV_I2C_SU_TR_LEN
	CP LOOP_CNTR,TEMP
	BRNE PC+0x02
	RJMP _DRV_I2C_SH_EVENT__RESET
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LDD TEMP,Y+_DRV_I2C_SU_BUFFER
	STS USIDR,TEMP
	INC LOOP_CNTR
	STD Y+_DRV_I2C_SU_POS,LOOP_CNTR
	LDI ACCUM,SDA
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	LDI TEMP,(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00)
	STS USISR,TEMP
	LDI TEMP,_DRV_I2C_SU_ST_TR_CHECK_ACK
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
.endif
