;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com			Начало
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;26.01.2021  w5277c@gmail.com			Не выводим лишние нули
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_NUMX8
.else
.set DEF_C5_LOG_NUMX8 = 1
.ifdef LOGGING_PORT

.include	"./math/div16x8.inc"
.include	"./math/div10.inc"
.include	"./math/mul10.inc"
.include	"./math/mul100.inc"
.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_NUMX8:
;--------------------------------------------------------
;Логирование десятиричного числа(1байт)
;IN: TEMP - байт
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	MOV ACCUM,TEMP
	LDI TEMP_H,0x00
	MOV TEMP_L,ACCUM
	LDI TEMP,0x64
	MCALL DIV16x8
	CPI TEMP_L,0x00
	BREQ PC+0x5
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL MUL100
	SUB ACCUM,TEMP_L
	MOV TEMP,ACCUM
	MCALL DIV10
	CPI TEMP_L,0x00
	BREQ PC+0x5
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL MUL10
	SUB ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x30)										;+0x30
	MOV TEMP,ACCUM
	MCALL C5_LOG_CHAR

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	RET
.endif
.endif
