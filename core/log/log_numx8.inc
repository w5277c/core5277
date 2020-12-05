;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_NUMx8
.else
.set DEF_C5_LOG_NUMx8 = 1
.ifdef LOGGING_PORT

.include	"./math/div16x8.inc"
.include	"./math/mul24x8.inc"
.include	"./math/mul10.inc"
.include	"./math/mul100.inc"
.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_NUMx8:
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
	MCALL C5_DIV16x8
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL C5_MUL100
	SUB ACCUM,TEMP_L
	MOV TEMP,ACCUM
	MCALL C5_DIV10
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL C5_MUL10
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
