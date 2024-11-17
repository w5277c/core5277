;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.10.2024	konstantin@5277.ru			Начало(черновой, не тестировано)
;02.11.2024	konstantin@5277.ru			Первые успешные тесты
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

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_I2C_SU_ST_CHECK_ADDRESS				= 0x00	;TODO вынести в _i2c.inc
	.EQU	_DRV_I2C_SU_ST_RC_DATA_WAIT				= 0x01
	.EQU	_DRV_I2C_SU_ST_TR_DATA						= 0x02
	.EQU	_DRV_I2C_SU_ST_RC_ACK_SEND					= 0x03
	.EQU	_DRV_I2C_SU_ST_TR_CHECK_ACK				= 0x04
	.EQU	_DRV_I2C_SU_ST_TR_NEXT						= 0x05
	;---VARS---
	.EQU	_DRV_I2C_SU_ADDR								= 0x00	;1B-Адрес устройства
	.EQU	_DRV_I2C_SU_POS								= 0x01	;1B-Позиция в буфере
	.EQU	_DRV_I2C_SU_TR_LEN							= 0x02	;1B-Размер данных передачи
	.EQU	_DRV_I2C_SU_BUFFER_SIZE						= 0x03	;1B-Размер буфера
	.EQU	_DRV_I2C_SU_PROC_ADDR						= 0x04	;2B-Адрес процедуры обработчика запроса
	.EQU	_DRV_I2C_SU_PROC_PID							= 0x06	;1B-PID задачи процедуры
	.EQU	_DRV_I2C_SU_STATE								= 0x07	;1B-Статус
	.EQU	_DRV_I2C_SU_BUFFER							= 0x08	;xB-Буфер

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
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_T16
	PUSH_Y

	MCALL C5_RAM_OFFSET

	LDI TEMP_H,high(_DRV_I2C_SU_START_EVENT)
	LDI TEMP_L,low(_DRV_I2C_SU_START_EVENT)
	LDI ACCUM,C5_IR_USI_START
	MCALL C5_IR_VECTOR_SET

	LDI TEMP_H,high(_DRV_I2C_SU_OVF_EVENT)
	LDI TEMP_L,low(_DRV_I2C_SU_OVF_EVENT)
	LDI ACCUM,C5_IR_USI_OVF
	MCALL C5_IR_VECTOR_SET

	RCALL _DRV_I2C_SU__RESET

	POP_Y
	POP_T16
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_I2C_SU__RESET:
;--------------------------------------------------------
	CBI DDRB,(SDA & 0x0f)											;SDA in
	CBI PORTB,(SDA & 0x0f)											;SDA without pull up
	CBI DDRB,(SCL & 0x0f)											;SCL in
	CBI PORTB,(SCL & 0x0f)											;SCL without pull up

	LDI ACCUM,(1<<USISIF)|(1<<USIOIF)|(0<<USIPIF)|(1<<USIDC)|(0x00<<USICNT0)
	STS USISR,ACCUM
	LDI ACCUM,(1<<USISIE)|(0<<USIOIE)|(1<<USIWM1)|(0<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(0<<USICLK)|(0<<USITC)
	STS USICR,ACCUM

	STS USIDR,C0x00

	STD Y+_DRV_I2C_SU_POS,C0x00
	RET

;--------------------------------------------------------
_DRV_I2C_SU_START_EVENT:
;--------------------------------------------------------
;Событие START от USI
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Y

	MCALL C5_RAM_OFFSET

_DRV_I2C_SU_START_EVENT__LOOP:
	LDI TEMP,0x00
	SBIC PINB,(SDA & 0x0f)
	ORI TEMP,0x02
	SBIC PINB,(SCL & 0x0f)
	ORI TEMP,0x01
	CPI TEMP,0x01
	BREQ _DRV_I2C_SU_START_EVENT__LOOP

	SBRS TEMP,0x01
	RJMP PC+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_START_EVENT__END

	STS USIDR,C0xff
	LDI TEMP,(1<<USISIF)|(1<<USIOIF)|(0<<USIPIF)|(1<<USIDC)|(0x00<<USICNT0)
	STS USISR,TEMP
	LDI TEMP,(1<<USISIE)|(1<<USIOIE)|(1<<USIWM1)|(1<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(0<<USICLK)|(0<<USITC)
	STS USICR,TEMP

	LDI TEMP,_DRV_I2C_SU_ST_CHECK_ADDRESS
	STD Y+_DRV_I2C_SU_STATE,TEMP

_DRV_I2C_SU_START_EVENT__END:
	POP_Y
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_I2C_SU_OVF_EVENT:
;--------------------------------------------------------
;Событие OVF от USI
;--------------------------------------------------------
	SBI DDRB,(SCL & 0x0f)											;SCL out(set lo by USI)
	PUSH_Y
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_I2C_SU_STATE
	CPI TEMP,_DRV_I2C_SU_ST_CHECK_ADDRESS
	BRNE _DRV_I2C_SH_EVENT__NO_CHECK_ADDRESS
	;---CHECK ADDRESS---
	LDS TEMP,USIDR
	MOV LOOP_CNTR,TEMP
	LSR TEMP
	LDD ACCUM,Y+_DRV_I2C_SU_ADDR
	CP ACCUM,TEMP
	BREQ PC+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END

	STD Y+_DRV_I2C_SU_TR_LEN,C0x00
	LDI TEMP,_DRV_I2C_SU_ST_RC_DATA_WAIT
	ANDI LOOP_CNTR,0x01
	BREQ __DRV_I2C_SU_OVF_EVENT__HANDLER_SKIP
	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	PUSH_X
	MOVW XL,YL
	ADIW XL,_DRV_I2C_SU_BUFFER
	PUSH_Z
	LDD ZH,Y+_DRV_I2C_SU_PROC_ADDR+0x00
	LDD ZL,Y+_DRV_I2C_SU_PROC_ADDR+0x01
	PUSH PID
	LDD PID,Y+_DRV_I2C_SU_PROC_PID
	ICALL																	;--------------------------------------------------------
	STD Y+_DRV_I2C_SU_TR_LEN,LOOP_CNTR							;Параметры обработчика:
	POP PID																;IN: X-буфер, PID-ид задачи, LOOP-CNTR-размер данных в
	POP_Z																	;буфере
	POP_X																	;OUT: LOOP_CNTR-размер данных ответа в буфере
	LDI TEMP,_DRV_I2C_SU_ST_TR_DATA								;--------------------------------------------------------

__DRV_I2C_SU_OVF_EVENT__HANDLER_SKIP:
	SBI DDRB,(SDA & 0x0f)											;SDA out
	STD Y+_DRV_I2C_SU_STATE,TEMP
	STD Y+_DRV_I2C_SU_POS,C0x00
	STS USIDR,C0x00
	LDI ACCUM,(1<<USISIF)|(1<<USIOIF)|(1<USIPIF)|(1<<USIDC)|(0x0E<<USICNT0)
	STS USISR,ACCUM
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SH_EVENT__NO_CHECK_ADDRESS:

	CPI TEMP,_DRV_I2C_SU_ST_RC_DATA_WAIT
	BRNE _DRV_I2C_SH_EVENT__NO_RC_DATA_WAIT
	;---PREPARING FOR DATA RECEIVE---
	LDI ACCUM,_DRV_I2C_SU_ST_RC_ACK_SEND
	STD Y+_DRV_I2C_SU_STATE,ACCUM
	CBI DDRB,(SDA & 0x0f)											;SDA in
	LDI TEMP,(1<<USISIF)|(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00<<USICNT0)
	STS USISR,TEMP
	RJMP _DRV_I2C_SU_EVENT__END

_DRV_I2C_SH_EVENT__NO_RC_DATA_WAIT:
	CPI TEMP,_DRV_I2C_SU_ST_RC_ACK_SEND
	BRNE _DRV_I2C_SU_ST_NO_RC_ACK_SEND
	;---RECEIVED DATA---
	LDI ACCUM,_DRV_I2C_SU_ST_RC_DATA_WAIT
	STD Y+_DRV_I2C_SU_STATE,ACCUM
	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	LDD TEMP,Y+_DRV_I2C_SU_BUFFER_SIZE
	CP LOOP_CNTR,TEMP
	BRCS PC+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END

	PUSH_Y
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LDS ACCUM,USIDR
	STD Y+_DRV_I2C_SU_BUFFER,ACCUM
	INC LOOP_CNTR
	POP_Y
	STD Y+_DRV_I2C_SU_POS,LOOP_CNTR

	STS USIDR,C0x00
	SBI DDRB,(SDA & 0x0f)											;SDA out
	LDI TEMP,(1<<USISIF)|(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x0e<<USICNT0)
	STS USISR,TEMP
	RJMP _DRV_I2C_SU_EVENT__END

_DRV_I2C_SU_ST_NO_RC_ACK_SEND:
	;---TRANSFER---
	CPI TEMP,_DRV_I2C_SU_ST_TR_DATA
	BRNE _DRV_I2C_SU_ST_NO_TR_DATA
	LDD TEMP,Y+_DRV_I2C_SU_TR_LEN
	CPI TEMP,0x00
	BRNE PC+0x03														;--------------------------------------------------------
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END
	LDI LOOP_CNTR,0x00
	RJMP _DRV_I2C_SU_EVENT__PUT_BYTE
_DRV_I2C_SU_ST_NO_TR_DATA:
	CPI TEMP,_DRV_I2C_SU_ST_TR_CHECK_ACK
	BRNE _DRV_I2C_SU_ST_NO_TR_CHECK_ACK
	CBI DDRB,(SDA & 0x0f)											;SDA in
	CBI PORTB,(SDA & 0x0f)											;SDA without pull up
	LDI TEMP,(1<<USISIF)|(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x0E<<USICNT0)
	STS USISR,TEMP
	STS USIDR,C0xFF
	LDI TEMP,_DRV_I2C_SU_ST_TR_NEXT
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END
_DRV_I2C_SU_ST_NO_TR_CHECK_ACK:
	CPI TEMP,_DRV_I2C_SU_ST_TR_NEXT
	BREQ PC+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END
	LDS ACCUM,USIDR
	SBRS ACCUM,0x00
	RJMP Pc+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END

	LDD LOOP_CNTR,Y+_DRV_I2C_SU_POS
	LDD TEMP,Y+_DRV_I2C_SU_TR_LEN
	CP LOOP_CNTR,TEMP
	BRNE PC+0x03
	RCALL _DRV_I2C_SU__RESET
	RJMP _DRV_I2C_SU_EVENT__END

_DRV_I2C_SU_EVENT__PUT_BYTE:
	PUSH_Y
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	LDD TEMP,Y+_DRV_I2C_SU_BUFFER
	POP_Y

	SBI DDRB,(SDA & 0x0f)											;SDA out
	SBI PORTB,(SDA & 0x0f)											;SDA hi

	STS USIDR,TEMP
	INC LOOP_CNTR
	STD Y+_DRV_I2C_SU_POS,LOOP_CNTR

	LDI TEMP,(1<<USISIF)|(1<<USIOIF)|(1<<USIPIF)|(1<<USIDC)|(0x00<<USICNT0)
	STS USISR,TEMP
	LDI TEMP,_DRV_I2C_SU_ST_TR_CHECK_ACK
	STD Y+_DRV_I2C_SU_STATE,TEMP
	RJMP _DRV_I2C_SU_EVENT__END

_DRV_I2C_SU_EVENT__END:
	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_Y
	CBI DDRB,(SCL & 0x0f)
	RET
.endif
