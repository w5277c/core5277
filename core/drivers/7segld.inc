;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.05.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер 7 сегментного цифрового индикатора типа kem-3461ar
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

;---CONSTANTS--------------------------------------------
	;---OFFSETS---
	.EQU	_DRV_7SEGLD_SIZE								= 0x00	;1B - Количество цифр
	.EQU	_DRV_7SEGLD_POS								= 0x01	;1B - Текущая цифра
	.EQU	_DRV_7SEGLD_SEG_PORTS						= 0x02	;16B - Порты для 8 сегментов
	.EQU	_DRV_7SEGLD_DIG_DATA							= 0x12	;SIZE*3B - Size*(1B значение + 2B порт цифры)
	;---OPERATIONS---
	.EQU	DRV_7SEGLD_OP_SET_PORT						= 0x00	;Устанавливаем порт для цифры
	.EQU	DRV_7SEGLD_OP_SET								= 0x01	;Устанавливаем значение цифры

;--------------------------------------------------------
DRV_7SEGLD_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид таймера, LOOP_CNTR-кол-во цифр
;TEMP_EH,EL,H,L,XH,XL,YH,YL-порты сегментов(pABCDEFG)
;--------------------------------------------------------
	PUSH_Z
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	PUSH ACCUM
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

	;_DRV_7SEGLD_SIZE*0x03+0x12
	MOV ACCUM,LOOP_CNTR
	MOV TEMP,LOOP_CNTR
	LSL TEMP
	ADD ACCUM,TEMP
	SUBI ACCUM,(0x0100-0x12)
	MCALL C5_RAM_REALLOC

	MOV LOOP_CNTR,ACCUM
	MOV XH,ZH
	MOV XL,ZL
	ADIW XL,0x12
	LDI TEMP,0xff
	MCALL RAM_FILL8

	STD Z+_DRV_7SEGLD_SIZE,LOOP_CNTR
	LDI TEMP,0x00
	STD Z+_DRV_7SEGLD_POS,TEMP

	;Инициализирую таймер с периодом в 100Hz для всех цифр
	;200/_DRV_7SEGLD_SIZE
	LDI TEMP_H,0x00
	LDI TEMP_L,0xc8
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
	CPI FLAGS,DRV_7SEGLD_OP_SET
	BRNE PC+0x04
	MCALL _DRV_7SEGLD_OP_SET
	RJMP _DRV_7SEGLD_PROC_END

_DRV_7SEGLD_PROC_END:
	POP TEMP
	POP_Z
	RET


;TODO

;--------------------------------------------------------
DRV_7SEGLD_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру(переключение цифр)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_Z

	;TODO

	POP_Z
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_7SEGLD_SEGPORT_INIT:
;--------------------------------------------------------
;Инициализация порта сегмента
;IN: ACCUM-потр, TEMP-порядковый номер, Z-адрес на
;выделенную память
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	CPI ACCUM,0xff
	BRNE _DRV_7SEGLD_SEGPORT_INIT__HAVE_PORT
	LDI TEMP_H,0xff
	LDI TEMP_L,0xff
	RJMP _DRV_7SEGLD_SEGPORT_INIT__STORE

_DRV_7SEGLD_SEGPORT_INIT__HAVE_PORT:
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	MCALL PORT_OFFSETS
	SUBI TEMP,(0x100-_DRV_7SEGLD_SEG_PORTS)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

_DRV_7SEGLD_SEGPORT_INIT__STORE:
	ST Z+,TEMP_H
	ST Z,TEMP_L

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
