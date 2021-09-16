;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.07.2021	w5277c@gmail.com			Начало
;10.07.2021	w5277c@gmail.com			Не тестирован
;-----------------------------------------------------------------------------------------------------------------------
;Счетчик изменений порта, максиммальная частота - 50Гц
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_COUNTER
.else
.set DEF__C5_DRV_COUNTER = 1
.message "Included driver counter v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_COUNTER_TICKS								= 0x09;Необходимое количество чтений порта
	.EQU	_DRV_COUNTER_RAM_SIZE							= 0x08
	;---DATA---
	.EQU	_DRV_COUNTER_PORT									= 0x00;1B - порт
	.EQU	_DRV_COUNTER_CNTR									= 0x01;1B - счетчик чтений порта
	.EQU	_DRV_COUNTER_SUMM									= 0x02;1B - количество низких состояний порта
	.EQU	_DRV_COUNTER_LAST_STATE							= 0x03;1B - предидущее состояние порта
	.EQU	_DRV_COUNTER_VALUE								= 0x04;4B - значение
	;---OPERATIONS---
	.EQU	DRV_COUNTER_OP_GET								= 0x00
	.EQU	DRV_COUNTER_OP_RESET								= 0x01

;--------------------------------------------------------
DRV_COUNTER_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт(7-бит-подтяжка к +), TEMP_L-ид таймера
;--------------------------------------------------------
	LDI ACCUM,_DRV_COUNTER_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_COUNTER_PORT,TEMP_H
	STD Y+_DRV_COUNTER_CNTR,C0x00
	STD Y+_DRV_COUNTER_SUMM,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x00,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x01,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x02,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x03,C0x00

	;Сбрасываем флаг подтяжки
	MOV ACCUM,TEMP_H
	ANDI ACCUM,0x7f
	STD Y+_DRV_COUNTER_PORT,ACCUM
	;Порт на вход
	MCALL PORT_MODE_IN
	;Включаем подтяжку, если необходимо
	SBRS TEMP_H,0x07
	RJMP PC+0x03
	MCALL PORT_SET_HI

	LDI TEMP,0xff
	STD Y+_DRV_COUNTER_LAST_STATE,TEMP

	MOV TEMP,TEMP_L
	LDI TEMP_H,high(_DRV_COUNTER_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_COUNTER_TIMER_EVENT)
	LDI ACCUM,0x81
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	MCALL C5_READY
;--------------------------------------------------------
;Основной код.
;--------------------------------------------------------
;IN: FLAGS-код операции
;OUT: TEMP_EH/EL/H/L-значение счетчика
;--------------------------------------------------------
	PUSH_Y
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_COUNTER_OP_GET
	BRNE _DRV_COUNTER__NO_OP_GET
	LDS ACCUM,SREG
	CLI
	LDD TEMP_EH,Y+_DRV_COUNTER_VALUE+0x00
	LDD TEMP_EL,Y+_DRV_COUNTER_VALUE+0x01
	LDD TEMP_H,Y+_DRV_COUNTER_VALUE+0x02
	LDD TEMP_L,Y+_DRV_COUNTER_VALUE+0x03
	STS SREG,ACCUM
	RJMP _DRV_COUNTER__END
_DRV_COUNTER__NO_OP_GET:
	CPI FLAGS,DRV_COUNTER_OP_RESET
	BRNE _DRV_COUNTER__END
	LDS ACCUM,SREG
	CLI
	STD Y+_DRV_COUNTER_VALUE+0x00,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x01,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x02,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x03,C0x00
	STS SREG,ACCUM

_DRV_COUNTER__END:
	POP ACCUM
	POP_Y
	RET

;--------------------------------------------------------
_DRV_COUNTER_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик по таймеру
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	LDD TEMP_H,Y+_DRV_COUNTER_SUMM
	LDD TEMP_L,Y+_DRV_COUNTER_CNTR
	CPI TEMP_L,_DRV_COUNTER_TICKS
	BRNE _DRV_COUNTER_TIMER_EVENT__PORT_READ

	LDD TEMP_L,Y+_DRV_COUNTER_LAST_STATE
	CPI TEMP_H,_DRV_COUNTER_TICKS/2
	LDI TEMP_H,0x00
	ROL TEMP_H
	CPI TEMP_L,0xff
	BRNE PC+0x03
	STD Y+_DRV_COUNTER_LAST_STATE,TEMP_H
	RJMP _DRV_COUNTER_TIMER_EVENT__INC_SKIP
	EOR TEMP_L,TEMP_H
	BREQ _DRV_COUNTER_TIMER_EVENT__INC_SKIP
	STD Y+_DRV_COUNTER_LAST_STATE,TEMP_H
	SBRS TEMP_H,0x00
	RJMP _DRV_COUNTER_TIMER_EVENT__INC_SKIP

	LDI TEMP_H,0x01
	LDD ACCUM,Y+_DRV_COUNTER_VALUE+0x03
	ADD ACCUM,TEMP_H
	STD Y+_DRV_COUNTER_VALUE+0x03,ACCUM
	LDD ACCUM,Y+_DRV_COUNTER_VALUE+0x02
	ADC ACCUM,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x02,ACCUM
	LDD ACCUM,Y+_DRV_COUNTER_VALUE+0x01
	ADC ACCUM,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x01,ACCUM
	LDD ACCUM,Y+_DRV_COUNTER_VALUE+0x00
	ADC ACCUM,C0x00
	STD Y+_DRV_COUNTER_VALUE+0x00,ACCUM

_DRV_COUNTER_TIMER_EVENT__INC_SKIP:
	STD Y+_DRV_COUNTER_CNTR,C0x00
	STD Y+_DRV_COUNTER_SUMM,C0x00
	RJMP _DRV_COUNTER_TIMER_EVENT__END

_DRV_COUNTER_TIMER_EVENT__PORT_READ:
	LDD ACCUM,Y+_DRV_COUNTER_PORT
	MCALL PORT_GET
	BRCS _DRV_COUNTER_TIMER_EVENT__PORT_HI
	INC TEMP_H
	STD Y+_DRV_COUNTER_SUMM,TEMP_H
_DRV_COUNTER_TIMER_EVENT__PORT_HI:
	INC TEMP_L
	STD Y+_DRV_COUNTER_CNTR,TEMP_L

_DRV_COUNTER_TIMER_EVENT__END:
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET
.endif
