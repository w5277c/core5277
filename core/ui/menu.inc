;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_MENU
.else
.set DEF_C5_MENU = 1

.include "./core/log/log_char.inc"
.include "./core/log/log_str_el.inc"
.include	"./io/input_get.inc"

;--------------------------------------------------------
C5_MENU:
;--------------------------------------------------------
;Выводит в лог элементы меню и ждет выбора
;IN: Y-адрес на элементы текста (заголовок меню, затем
;элементы), ACCUM-кол-во элементов меню(без учета
;заголовка)
;OUT: ACCUM-номер элемента(1-...)
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS
	PUSH LOOP_CNTR

	LDI TEMP,0x00
	MCALL C5_LOG_STR_EL
	MCALL C5_LOG_CR

	LDI FLAGS,0x01
	MOV LOOP_CNTR,ACCUM
_MENU_EXEC__LOOP:
	LDI TEMP,' '
	MCALL C5_LOG_CHAR
	MOV TEMP,FLAGS
	CPI TEMP,0x0a
	BRCS PC+0x02
	SUBI TEMP,(0x100-0x07)
	SUBI TEMP,(0x100-0x30)
	MCALL C5_LOG_CHAR
	LDI TEMP,')'
	MCALL C5_LOG_CHAR
	MOV TEMP,FLAGS
	MCALL C5_LOG_STR_EL
	MCALL C5_LOG_CR
	INC FLAGS
	DEC LOOP_CNTR
	BRNE _MENU_EXEC__LOOP
	LDI TEMP,'>'
	MCALL C5_LOG_CHAR

_MENU_EXEC__WAIT:
	MCALL INPUT_WAIT_CHAR
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
	MCALL C5_LOG_CHAR
	LDI TEMP,')'
	MCALL C5_LOG_CHAR
	MOV TEMP,ACCUM
	MCALL C5_LOG_STR_EL
	MCALL C5_LOG_CR

	POP LOOP_CNTR
	POP FLAGS
	POP TEMP
	RET
.endif
