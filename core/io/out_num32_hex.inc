;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало (не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_NUM32_HEX
.else
.set DEF_C5_OUT_NUM32_HEX = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_byte.inc"

;--------------------------------------------------------
C5_OUT_NUM32_HEX:
;--------------------------------------------------------
;Логирование HEX числа(4 байта)
;IN: TEMP_EH/EL/H/L-двойное слово
;--------------------------------------------------------
	PUSH TEMP

	LDI TEMP,'0'
	MCALL C5_OUT_CHAR
	LDI TEMP,'x'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_EH
	MCALL C5_OUT_BYTE
	MOV TEMP,TEMP_EL
	MCALL C5_OUT_BYTE
	MOV TEMP,TEMP_H
	MCALL C5_OUT_BYTE
	MOV TEMP,TEMP_L
	MCALL C5_OUT_BYTE

	POP TEMP
	RET
.endif
.endif
