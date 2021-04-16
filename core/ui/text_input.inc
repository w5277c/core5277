;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.04.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TEXT_INPUT
.else
.set DEF_C5_TEXT_INPUT = 1

.include "./core/log/log_char.inc"
.include "./core/log/log_strn.inc"
.include "./core/log/log_cr.inc"

;--------------------------------------------------------
C5_TEXT_INPUT:
;--------------------------------------------------------
;Считывет с UART ASCII HEX символы(без добавления 0)
;IN:Y-адрес буфера,ACCUM-размер буфера
;OUT: TEMP-длина
;--------------------------------------------------------
	PUSH LOOP_CNTR

	;Текущая длина
	LDI LOOP_CNTR,0x00

	MCALL _C5_TEXT_INPUT_DRAW

C5_TEXT_INPUT__MAIN_LOOP:
	MCALL C5_IN_WAIT
	CPI TEMP,0x0d
	BREQ C5_TEXT_INPUT__DONE

	CPI TEMP,0x08
	BRNE C5_TEXT_INPUT__KEY_CHECKING_NOT_BACKSPACE
	CPI LOOP_CNTR,0x00
	BREQ C5_TEXT_INPUT__MAIN_LOOP
	DEC LOOP_CNTR
	MCALL _C5_TEXT_INPUT_DRAW
	LDI TEMP,' '
	MCALL C5_LOG_CHAR
	MCALL _C5_TEXT_INPUT_DRAW
	RJMP C5_TEXT_INPUT__MAIN_LOOP
C5_TEXT_INPUT__KEY_CHECKING_NOT_BACKSPACE:
	CPI TEMP,0x20
	BRCS C5_TEXT_INPUT__MAIN_LOOP
	CPI TEMP,0x7f
	BRCC C5_TEXT_INPUT__MAIN_LOOP
	CP LOOP_CNTR,ACCUM
	BREQ C5_TEXT_INPUT__MAIN_LOOP
	PUSH_Y
	ADD YL,LOOP_CNTR
	PUSH TEMP
	LDI TEMP,0x00
	ADC YL,TEMP
	POP TEMP
	ST Y,TEMP
	POP_Y
	INC LOOP_CNTR
	MCALL _C5_TEXT_INPUT_DRAW
	RJMP C5_TEXT_INPUT__MAIN_LOOP
C5_TEXT_INPUT__DONE:
	MCALL C5_LOG_CR
	MOV TEMP,LOOP_CNTR

	POP LOOP_CNTR
	RET

_C5_TEXT_INPUT_DRAW:
	LDI TEMP,0x0d
	MCALL C5_LOG_CHAR
	LDI TEMP,'$'
	MCALL C5_LOG_CHAR

	MOV TEMP,LOOP_CNTR
	CPI TEMP,0x00
	BREQ PC+0x03
	MCALL C5_LOG_STRN
	RET
.endif
