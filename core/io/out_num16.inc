;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.01.2021	w5277c@gmail.com			Начало (не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_NUM16
.else
.set DEF_C5_OUT_NUM16 = 1
.ifdef LOGGING_PORT

.include	"./math/div16x8.inc"
.include	"./math/mul16x8.inc"
.include	"./core/io/out_char.inc"

;--------------------------------------------------------
C5_OUT_NUM16:
;--------------------------------------------------------
;Логирование десятиричного числа(2 байта)
;IN: TEMP_H/TEMP_L-слово
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Y

	MOV YH,TEMP_H
	MOV YL,TEMP_L
	LDI TEMP,0x0a
	LDI LOOP_CNTR,0x05
_C5_OUT_NUM16__LOOP1:
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	PUSH TEMP_L
	MCALL DIV16X8
	MOV YH,TEMP_H
	MOV YL,TEMP_L
	MCALL MUL16X8
	POP TEMP_H
	SUB TEMP_H,TEMP_L
	PUSH TEMP_H
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM16__LOOP1

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x05
_C5_OUT_NUM16__LOOP2:
	POP TEMP
	CPI LOOP_CNTR,0x01
	BREQ PC+0x03
	OR TEMP_L,TEMP
	BREQ PC+0x04
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM16__LOOP2

	POP_Y
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP ACCUM
	RET
.endif
.endif
