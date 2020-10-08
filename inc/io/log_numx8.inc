;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_NUMx8
.else
.set DEF_CORE5277_LOG_NUMx8 = 1
.ifdef LOGGING_PORT

.include	"./inc/math/div16x8.inc"
.include	"./inc/math/mul24x8.inc"
.include	"./inc/math/mul10.inc"
.include	"./inc/math/mul100.inc"
.include	"./inc/io/log_char.inc"

;--------------------------------------------------------
CORE5277_LOG_NUMx8:
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
	MCALL CORE5277_DIV16x8
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL CORE5277_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL CORE5277_MUL100
	SUB ACCUM,TEMP_L
	MOV TEMP,ACCUM
	MCALL CORE5277_DIV10
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL CORE5277_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL CORE5277_MUL10
	SUB ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x30)										;+0x30
	MOV TEMP,ACCUM
	MCALL CORE5277_LOG_CHAR

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	RET
.endif
.endif
