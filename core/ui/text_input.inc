;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.04.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;20.11.2022	konstantin@5277.ru		Мелкий багфиксинг(забыт adc zh,c0x00)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TEXT_INPUT
.else
.set DEF_C5_TEXT_INPUT = 1

.include "./core/io/out_char.inc"
.include "./core/io/out_strn.inc"
.include "./core/io/out_cr.inc"

;--------------------------------------------------------
C5_TEXT_INPUT:
;--------------------------------------------------------
;Считывет с UART ASCII HEX символы(без добавления 0)
;IN:Z-адрес буфера,ACCUM-размер буфера
;OUT: TEMP-длина
;--------------------------------------------------------
	PUSH LOOP_CNTR

	;Текущая длина
	LDI LOOP_CNTR,0x00

	MCALL _C5_TEXT_INPUT_DRAW

C5_TEXT_INPUT__MAIN_LOOP:
	MCALL C5_INPUT_WAIT
	CPI TEMP,0x0d
	BREQ C5_TEXT_INPUT__DONE

	CPI TEMP,0x08
	BRNE C5_TEXT_INPUT__KEY_CHECKING_NOT_BACKSPACE
	CPI LOOP_CNTR,0x00
	BREQ C5_TEXT_INPUT__MAIN_LOOP
	DEC LOOP_CNTR
	MCALL _C5_TEXT_INPUT_DRAW
	LDI TEMP,' '
	MCALL C5_OUT_CHAR
	MCALL _C5_TEXT_INPUT_DRAW
	RJMP C5_TEXT_INPUT__MAIN_LOOP
C5_TEXT_INPUT__KEY_CHECKING_NOT_BACKSPACE:
	CPI TEMP,0x20
	BRCS C5_TEXT_INPUT__MAIN_LOOP
	CPI TEMP,0x7f
	BRCC C5_TEXT_INPUT__MAIN_LOOP
	CP LOOP_CNTR,ACCUM
	BREQ C5_TEXT_INPUT__MAIN_LOOP
	PUSH_Z
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
	PUSH TEMP
	LDI TEMP,0x00
	ADC ZL,TEMP
	POP TEMP
	ST Z,TEMP
	POP_Z
	INC LOOP_CNTR
	MCALL _C5_TEXT_INPUT_DRAW
	RJMP C5_TEXT_INPUT__MAIN_LOOP
C5_TEXT_INPUT__DONE:
	MCALL C5_OUT_CR
	MOV TEMP,LOOP_CNTR

	POP LOOP_CNTR
	RET

_C5_TEXT_INPUT_DRAW:
	LDI TEMP,0x0d
	MCALL C5_OUT_CHAR
	LDI TEMP,'$'
	MCALL C5_OUT_CHAR

	MOV TEMP,LOOP_CNTR
	CPI TEMP,0x00
	BREQ PC+0x01+_MCALL_SIZE
	MCALL C5_OUT_STRN
	RET
.endif
