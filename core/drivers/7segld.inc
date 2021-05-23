;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.05.2021	w5277c@gmail.com			Начало
;23.05.2021	w5277c@gmail.com			Готов общий код, не тестировано
;23.05.2021	w5277c@gmail.com			Исправлены основные ошибки
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер 7 сегментного цифрового индикатора типа kem-3461ar
;-----------------------------------------------------------------------------------------------------------------------
;TODO: Выключать сегменты перед включением следующей цифры.
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_7SEGLD
.else
.set DEF__C5_DRV_7SEGLD = 1
.message "Included driver 7seg ld v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./math/div16x8.inc"
.include	"./mem/ram_fill8.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set.inc"

;---CONSTANTS--------------------------------------------
	;---OFFSETS---
	.EQU	_DRV_7SEGLD_SIZE								= 0x00	;1B - Количество цифр
	.EQU	_DRV_7SEGLD_POS								= 0x01	;1B - Текущая цифра
	.EQU	_DRV_7SEGLD_FLAGS								= 0x02	;1B - Режим работы
	.EQU	_DRV_7SEGLD_SEG_PORTS						= 0x03	;16B - Порты для 8 сегментов
	.EQU	_DRV_7SEGLD_DIG_DATA							= 0x13	;SIZE*3B - SIZE*(1B значение + 2B порт цифры)
	;---MODES---
	.EQU	DRV_7SEGLD_MODE_LDIG_LSEG					= 0x00	;- на пины цифр и - для сегментов
	.EQU	DRV_7SEGLD_MODE_LDIG_HSEG					= 0x01	;- на пины цифр и + для сегментов
	.EQU	DRV_7SEGLD_MODE_HDIG_LSEG					= 0x02	;+ на пины цифр и - для сегментов
	.EQU	DRV_7SEGLD_MODE_HDIG_HSEG					= 0x03	;+ на пины цифр и + для сегментов

	;---OPERATIONS---
	.EQU	DRV_7SEGLD_OP_SET_PORT						= 0x00	;Устанавливаем порт цифры
	.EQU	DRV_7SEGLD_OP_SET_VAL						= 0x01	;Устанавливаем значение цифры

;--------------------------------------------------------
DRV_7SEGLD_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид таймера, LOOP_CNTR-кол-во цифр,
;FLAGS-режим работы DRV_7SEGLD_MODE_...
;TEMP_EH/EL/H/L,XH/L,YH/L-порты сегментов(ABCDEFGp)
;--------------------------------------------------------
	PUSH_Z
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	PUSH ACCUM
	;_DRV_7SEGLD_SIZE*0x03+_DRV_7SEGLD_DIG_DATA
	MOV ACCUM,LOOP_CNTR
	MOV TEMP,LOOP_CNTR
	LSL TEMP
	ADD ACCUM,TEMP
	SUBI ACCUM,(0x0100-_DRV_7SEGLD_DIG_DATA)
	MCALL C5_RAM_REALLOC

	MOV ACCUM,TEMP_EH
	LDI TEMP,0x00
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,TEMP_EL
	LDI TEMP,0x01
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,TEMP_H
	LDI TEMP,0x02
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,TEMP_L
	LDI TEMP,0x03
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,XH
	LDI TEMP,0x04
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,XL
	LDI TEMP,0x05
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,YH
	LDI TEMP,0x06
	MCALL _DRV_7SEGLD_SEGPORT_INIT
	MOV ACCUM,YL
	LDI TEMP,0x07
	MCALL _DRV_7SEGLD_SEGPORT_INIT

	STD Z+_DRV_7SEGLD_SIZE,LOOP_CNTR
	LDI TEMP,0x00
	STD Z+_DRV_7SEGLD_POS,TEMP
	STD Z+_DRV_7SEGLD_FLAGS,FLAGS

	PUSH LOOP_CNTR
	MOV LOOP_CNTR,ACCUM
	MOV XH,ZH
	MOV XL,ZL
	ADIW XL,0x12
	LDI TEMP,0xff
	MCALL RAM_FILL8
	POP LOOP_CNTR

	;Инициализирую таймер с периодом в 120Hz для всех цифр
	;120/_7SEGLD_SIZE
	LDI TEMP_H,0x00
	LDI TEMP_L,0x78
	MOV TEMP,LOOP_CNTR
	MCALL DIV16X8
	MOV ACCUM,TEMP_L
	LDI TEMP_H,high(DRV_7SEGLD_TIMER_EVENT)
	LDI TEMP_L,low(DRV_7SEGLD_TIMER_EVENT)
	POP TEMP
	MCALL C5_TIMER_SET
	;Запускаю таймер
	MCALL C5_TIMER_START

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	POP_X
	POP_Z
	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS - тип операции
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_7SEGLD_OP_SET_PORT
	BRNE PC+0x04
	MCALL _DRV_7SEGLD_OP_SET_PORT
	RJMP _DRV_7SEGLD_PROC_END
	CPI FLAGS,DRV_7SEGLD_OP_SET_VAL
	BRNE PC+0x04
	MCALL _DRV_7SEGLD_OP_SET_VAL
	RJMP _DRV_7SEGLD_PROC_END

_DRV_7SEGLD_PROC_END:
	POP TEMP
	POP_Z
	RET


;--------------------------------------------------------
_DRV_7SEGLD_OP_SET_PORT:
;--------------------------------------------------------
;Установка порта цифры
;IN: ACCUM-порт,TEMP_L-номер(0-...)
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	LDD FLAGS,Z+_DRV_7SEGLD_FLAGS

	MCALL _DRV_7SEGLD_GET_DIG_OFFSET

	CPI ACCUM,0xff
	BRNE PC+0x07
	LDI TEMP_H,0xff
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	RJMP _DRV_7SEGLD_OP_SET_PORT__SET

	MCALL PORT_MODE_OUT
	MOV TEMP_H,FLAGS
	LSR TEMP_H
	LSR TEMP_H
	MCALL PORT_SET
	MCALL PORT_OFFSETS
	SBRS FLAGS,0x01
	COM ACCUM

_DRV_7SEGLD_OP_SET_PORT__SET:
	LDI TEMP,0x00
	ST Y+,TEMP
	ST Y+,TEMP_H
	ST Y,ACCUM

	POP FLAGS
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET

;--------------------------------------------------------
_DRV_7SEGLD_OP_SET_VAL:
;--------------------------------------------------------
;Установка значения цифры
;IN: ACCUM-значение,TEMP_L-номер(0-...)
;--------------------------------------------------------
	PUSH_Y
	PUSH FLAGS
	PUSH ACCUM

	LDD FLAGS,Z+_DRV_7SEGLD_FLAGS
	MCALL _DRV_7SEGLD_GET_DIG_OFFSET
	SBRS FLAGS,0x01
	COM ACCUM
	ST Y,ACCUM

	POP ACCUM
	POP FLAGS
	POP_Y
	RET

;--------------------------------------------------------
DRV_7SEGLD_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру(переключение цифр)
;--------------------------------------------------------
	PUSH FLAGS
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET

	LDD FLAGS,Z+_DRV_7SEGLD_FLAGS
	LDD TEMP_H,Z+_DRV_7SEGLD_SIZE
	LDD TEMP_L,Z+_DRV_7SEGLD_POS

	;Выключаем текущую цифру
	MCALL _DRV_7SEGLD_GET_DIG_OFFSET
	LDI XH,0x00
	LDD XL,Y+0x01														;PORT addr
	LDD ACCUM,Y+0x02													;PIN num
	COM ACCUM
	CPI XL,0xff
	BREQ DRV_7SEGLD_TIMER_EVENT__SKIP_TURN_OFF_DIG
	LD TEMP,X
	SBRC FLAGS,0x01
	AND TEMP,ACCUM
	SBRS FLAGS,0x01
	OR TEMP,ACCUM
	ST X,TEMP
DRV_7SEGLD_TIMER_EVENT__SKIP_TURN_OFF_DIG:

	;Следующая цифра
	INC TEMP_L
	CP TEMP_L,TEMP_H
	BRNE PC+0x02
	CLR TEMP_L
	STD Z+_DRV_7SEGLD_POS,TEMP_L

	;Включаем цифру
	MCALL _DRV_7SEGLD_GET_DIG_OFFSET
	LDD XL,Y+0x01														;PORT addr
	LDD ACCUM,Y+0x02													;PIN num
	CPI XL,0xff
	BREQ DRV_7SEGLD_TIMER_EVENT__SKIP_TURN_ON_DIG
	LD TEMP,X
	SBRS FLAGS,0x01
	AND TEMP,ACCUM
	SBRC FLAGS,0x01
	OR TEMP,ACCUM
	ST X,TEMP
DRV_7SEGLD_TIMER_EVENT__SKIP_TURN_ON_DIG:

	LDD ACCUM,Y+0x00													;Value
	;Устанавливаем новое значение
	ADIW ZL,_DRV_7SEGLD_SEG_PORTS
	LDI XH,0x00
	LDI LOOP_CNTR,0x08
_DRV_7SEGLD_TIMER_EVENT__LOOP:
	LD XL,Z+
	LD TEMP_H,Z+
	LD FLAGS,X
	MOV TEMP_L,TEMP_H
	COM TEMP_L
	SBRS ACCUM,0x07
	AND FLAGS,TEMP_L
	SBRC ACCUM,0x07
	OR FLAGS,TEMP_H
	ST X,FLAGS
	LSL ACCUM
	DEC LOOP_CNTR
	BRNE _DRV_7SEGLD_TIMER_EVENT__LOOP

_DRV_7SEGLD_TIMER_EVENT__END:
	POP_Z
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_7SEGLD_SEGPORT_INIT:
;--------------------------------------------------------
;Инициализация порта сегмента
;IN: ACCUM-потр, TEMP-порядковый номер, Z-адрес на
;выделенную память, FLAGS-режим работы
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	CPI ACCUM,0xff
	BRNE PC+0x03
	LDI TEMP_H,0xff
	RJMP _DRV_7SEGLD_SEGPORT_INIT__SET

	MCALL PORT_MODE_OUT
	MOV TEMP_H,FLAGS
	LSR TEMP_H
	MCALL PORT_SET

	PUSH TEMP
	MCALL PORT_OFFSETS
	POP TEMP
	SBRS FLAGS,0x00
	COM ACCUM

_DRV_7SEGLD_SEGPORT_INIT__SET:
	LSL TEMP
	SUBI TEMP,(0x100-_DRV_7SEGLD_SEG_PORTS)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	ST Z+,TEMP_H
	ST Z,ACCUM

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET

;--------------------------------------------------------
_DRV_7SEGLD_GET_DIG_OFFSET:
;--------------------------------------------------------
;Вычисление смещения на данные цифры
;IN: TEMP_L-порядковый номер,
;Z-адрес на выделенную память
;OUT: Y-адрес на данные цифры
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	;_DRV_7SEGLD_DIG_DATA+TEMP*0x03
	MOV TEMP_H,TEMP_L
	LSL TEMP_L
	ADD TEMP_L,TEMP_H
	MOV YH,ZH
	MOV YL,ZL
	ADD YL,TEMP_L
	CLR TEMP_L
	ADC YH,TEMP_L
	ADIW YL,_DRV_7SEGLD_DIG_DATA

	POP TEMP_L
	POP TEMP_H
	RET
