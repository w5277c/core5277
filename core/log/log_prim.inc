;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_PRIM
.else
.set DEF_C5_LOG_PRIM = 1
.ifdef LOGGING_PORT

.include	"./math/div32x16.inc"
.include	"./math/mul32x16.inc"
.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_PRIM:
;--------------------------------------------------------
;Логирование примитива
;IN: Z-адрес на память с переменными, TEMP-смещение для
;переменной, ACCUM-тип примитива
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Y
	PUSH_Z

	ADD ZL,TEMP
	ADC ZH,C0x00
	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00

	CLR TEMP_EH
	CLR TEMP_EL
	CLR TEMP_H
	CLR TEMP_L

	LD TEMP_L,-Z
	DEC LOOP_CNTR
	BREQ C5_LOG_PRIM__LOAD_DONE
	LD TEMP_H,-Z
	DEC LOOP_CNTR
	BREQ C5_LOG_PRIM__LOAD_DONE
	LD TEMP_EL,-Z
	DEC LOOP_CNTR
	BREQ C5_LOG_PRIM__LOAD_DONE
	LD TEMP_EH,-Z
	;TODO не поддерживается LONG


C5_LOG_PRIM__LOAD_DONE:
	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L

	LDI XH,0x00
	LDI XL,0x0a
	LDI LOOP_CNTR,0x0a
_C5_LOG_PRIM__LOOP1:
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
	BRNE _C5_LOG_PRIM__LOOP1

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x0a
_C5_LOG_PRIM__LOOP2:
	POP TEMP
	CPI LOOP_CNTR,0x01
	BREQ PC+0x03
	OR TEMP_L,TEMP
	BREQ PC+0x04
	SUBI TEMP,(0x100-0x30)
	MCALL C5_LOG_CHAR
	DEC LOOP_CNTR
	BRNE _C5_LOG_PRIM__LOOP2

	POP_Z
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP_FT
	RET
.endif
.endif
