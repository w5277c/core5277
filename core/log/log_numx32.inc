;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало (не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_NUMX32
.else
.set DEF_C5_LOG_NUMX32 = 1
.ifdef LOGGING_PORT

.include	"./math/div32x16.inc"
.include	"./math/mul32x16.inc"
.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_NUMX32:
;--------------------------------------------------------
;Логирование десятиричного числа(4 байта)
;IN: TEMP_EH/EL/H/L-двойное слово
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Y
	PUSH_Z

	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L

	LDI_X 0x000a
	LDI LOOP_CNTR,0x0a
_C5_LOG_NUMX32__LOOP1:
	MOV TEMP_EH,YH
	MOV TEMP_EL,YL
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	PUSH TEMP_L
	MCALL DIV32X16
	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	MCALL MUL32X16
	POP TEMP_H
	SUB TEMP_H,TEMP_L
	PUSH TEMP_H
	DEC LOOP_CNTR
	BRNE _C5_LOG_NUMX32__LOOP1

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x0a
_C5_LOG_NUMX32__LOOP2:
	POP TEMP
	CPI LOOP_CNTR,0x01
	BREQ PC+0x03
	OR TEMP_L,TEMP
	BREQ PC+0x04
	SUBI TEMP,(0x100-0x30)
	MCALL C5_LOG_CHAR
	DEC LOOP_CNTR
	BRNE _C5_LOG_NUMX32__LOOP2

	POP_Z
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP_FT
	RET
.endif
.endif
