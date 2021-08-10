;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO отказаться от T
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_NUM16_TO_STR
.else
.set DEF_NUM16_TO_STR = 1

.if REPORT_INCLUDES == 0x01
.message "included NUM16_TO_STR"
.endif

.include	"./math/div32x16.inc"
.include	"./math/div16x8.inc"
.include	"./math/mul16x8.inc"

;--------------------------------------------------------
NUM16_TO_STR:
;--------------------------------------------------------
;Конвертирование числа 2Б в текст
;Конец строки не указывается
;IN: TEMP_H,TEMP_L-число (0-65535),
;Z - dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL

	CLT
	LDI LOOP_CNTR,0x05
	LDI XH,high(10000)
	LDI XL,low(10000)
__NUM16_TO_STR__LOOP:
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	PUSH TEMP_H
	PUSH TEMP_L
	MCALL DIV32X16
	BRTS PC+0x04
	CPI TEMP_L,0x00
	BREQ __NUM16_TO_STR__SKIP_CHAR
	SET
	LDI TEMP,0x30
	ADD TEMP,TEMP_L
	ST Z+,TEMP
__NUM16_TO_STR__SKIP_CHAR:
	MOV TEMP,TEMP_L
	MOV TEMP_H,XH
	MOV TEMP_L,XL
	MCALL MUL16X8
	POP TEMP_EL
	POP TEMP_EH
	SUB TEMP_EL,TEMP_L
	SBC TEMP_EH,TEMP_H
	MOV TEMP_H,XH
	MOV TEMP_L,XL
	LDI TEMP,10
	MCALL DIV16X8
	MOV XH,TEMP_H
	MOV XL,TEMP_L
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	DEC LOOP_CNTR
	BRNE __NUM16_TO_STR__LOOP

	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	POP_X
	RET
.endif
