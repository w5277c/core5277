;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.02.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_MENU
.else
.set DEF_C5_MENU = 1

.include "./core/io/out_char.inc"
.include "./core/io/out_str_el.inc"

;--------------------------------------------------------
C5_MENU:
;--------------------------------------------------------
;Выводит в лог элементы меню и ждет выбора
;IN: Z-адрес на элементы текста (заголовок меню, затем
;элементы), ACCUM-кол-во элементов меню(без учета
;заголовка)
;OUT: ACCUM-номер элемента(1-...)
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS
	PUSH LOOP_CNTR

	LDI TEMP,0x00
	MCALL C5_OUT_STR_EL
	MCALL C5_OUT_CR

	LDI FLAGS,0x01
	MOV LOOP_CNTR,ACCUM
_MENU_EXEC__LOOP:
	LDI TEMP,' '
	MCALL C5_OUT_CHAR
	MOV TEMP,FLAGS
	CPI TEMP,0x0a
	BRCS PC+0x02
	SUBI TEMP,(0x100-0x07)
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	LDI TEMP,')'
	MCALL C5_OUT_CHAR
	MOV TEMP,FLAGS
	MCALL C5_OUT_STR_EL
	MCALL C5_OUT_CR
	INC FLAGS
	DEC LOOP_CNTR
	BRNE _MENU_EXEC__LOOP
	LDI TEMP,'>'
	MCALL C5_OUT_CHAR

_MENU_EXEC__WAIT:
	MCALL C5_INPUT_WAIT
	MOV FLAGS,TEMP
	CPI TEMP,0x31
	BRCS _MENU_EXEC__WAIT
	CPI TEMP,0x7f
	BRCC _MENU_EXEC__WAIT
	CPI TEMP,0x3a
	BRCS PC+0x02
	SUBI TEMP,0x07
	SUBI TEMP,0x30
	MOV LOOP_CNTR,ACCUM
	INC LOOP_CNTR
	CP TEMP,LOOP_CNTR
	BRCC _MENU_EXEC__WAIT
	MOV ACCUM,TEMP
	MOV TEMP,FLAGS
	MCALL C5_OUT_CHAR
	LDI TEMP,')'
	MCALL C5_OUT_CHAR
	MOV TEMP,ACCUM
	MCALL C5_OUT_STR_EL
	MCALL C5_OUT_CR

	POP LOOP_CNTR
	POP FLAGS
	POP TEMP
	RET
.endif
