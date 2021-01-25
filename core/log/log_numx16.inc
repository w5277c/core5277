;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_NUMX16
.else
.set DEF_C5_LOG_NUMX16 = 1
.ifdef LOGGING_PORT

.include	"./math/div32x16.inc"
.include	"./math/mul24x8.inc"
.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_NUMX16:
;--------------------------------------------------------
;Логирование десятиричного числа(1байт)
;IN: TEMP_H/TEMP_L - слово
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH_X
	PUSH_Y

	MOV ACCUM,TEMP
	LDI TEMP_H,0x00
	MOV TEMP_L,ACCUM

	MOV YH,TEMP_H
	MOV YL,TEMP_L
	LDI TEMP_EH,0
	LDI TEMP_EL,0
	LDI_X 10000
	MCALL DIV32X16
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	LDI TEMP,100
	MCALL MUL24X8
	LDI TEMP,100
	MCALL MUL24X8
	SUB YL,TEMP_L
	SBC YH,TEMP_H
	LDI TEMP_EH,0
	LDI TEMP_EL,0
	LDI_X 1000
	MCALL DIV32X16
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	LDI TEMP,100
	MCALL MUL24X8
	LDI TEMP,10
	MCALL MUL24X8
	SUB YL,TEMP_L
	SBC YH,TEMP_H
	LDI TEMP_EH,0
	LDI TEMP_EL,0
	LDI_X 100
	MCALL DIV32X16
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	LDI TEMP,100
	MCALL MUL24X8
	SUB YL,TEMP_L
	SBC YH,TEMP_H
	LDI TEMP_EH,0
	LDI TEMP_EL,0
	LDI_X 10
	MCALL DIV32X16
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_LOG_CHAR

	LDI TEMP,100
	MCALL MUL24X8
	SUB YL,TEMP_L
	SBC YH,TEMP_H
	SUBI YL,(0x100-0x30)												;+0x30
	MOV TEMP,YL
	MCALL C5_LOG_CHAR

	POP_Y
	POP_X
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	RET
.endif
.endif
