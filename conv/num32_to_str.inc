;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало (не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_NUMX32_TO_STR
.else
.set DEF_NUMX32_TO_STR = 1

.include	"./math/div32x16.inc"
.include	"./math/mul32x16.inc"

;--------------------------------------------------------
NUMX32_TO_STR:
;--------------------------------------------------------
;Запись 4Б числа в виде строки
;IN: TEMP_EH/EL/H/L-двойное слово, Z-dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Y
	PUSH FLAGS
	PUSH TRY_CNTR

	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV FLAGS,TEMP_H
	MOV TRY_CNTR,TEMP_L

	LDI_X 0x000a
	LDI LOOP_CNTR,0x0a
_NUMX32_TO_STR__LOOP1:
	MOV TEMP_EH,YH
	MOV TEMP_EL,YL
	MOV TEMP_H,FLAGS
	MOV TEMP_L,TRY_CNTR
	PUSH TEMP_L
	MCALL DIV32X16
	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV FLAGS,TEMP_H
	MOV TRY_CNTR,TEMP_L
	MCALL MUL32X16
	POP TEMP_H
	SUB TEMP_H,TEMP_L
	PUSH TEMP_H
	DEC LOOP_CNTR
	BRNE _NUMX32_TO_STR__LOOP1

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x0a
_NUMX32_TO_STR__LOOP2:
	POP TEMP
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	DEC LOOP_CNTR
	BRNE _NUMX32_TO_STR__LOOP2

	POP TRY_CNTR
	POP FLAGS
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP_FT
	RET
.endif
.endif
