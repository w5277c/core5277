;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021	w5277c@gmail.com			Начало (не проверено)
;03.03.2021	w5277c@gmail.com			PUSH/POP LOOP_CNTR
;16.09.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
;TODO избавится от флага T
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_C5_OUT_TIME32
.else
.set DEF_C5_OUT_TIME32 = 1
.ifdef LOGGING_PORT

.include	"./math/mul16x8.inc"
.include	"./math/mul32x16.inc"
.include	"./core/io/out_num16.inc"
.include	"./core/io/out_num32.inc"

;--------------------------------------------------------
C5_OUT_TIME32:
;--------------------------------------------------------
;Логирование временной метки
;IN: PID,TEMP-смещение на выделенные 4 байт, хранящие
;временную метку
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	LDS TEMP_L,SREG
	PUSH TEMP_L

	MCALL C5_RAM_OFFSET
	ADD YL,TEMP
	ADC YH,C0x00

	LDD TEMP_EH,Y+0x00
	LDD TEMP_EL,Y+0x01
	LDD TEMP_H,Y+0x02
	LDD TEMP_L,Y+0x03

	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L

	LDI XH,0x00
	LDI XL,0x0a
	LDI LOOP_CNTR,0x0a
C5_OUT_TIME32__LOOP1:
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
	BRNE C5_OUT_TIME32__LOOP1

	CLT
	LDI LOOP_CNTR,0x04
C5_OUT_TIME32__LOOP_S:
	POP TEMP
	BRTS PC+0x04
	CPI TEMP,0x00
	BREQ PC+0x03+_MCALL_SIZE
	SET
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE C5_OUT_TIME32__LOOP_S

	LDI LOOP_CNTR,0x03
C5_OUT_TIME32__LOOP_MS:
	POP TEMP
	BRTS PC+0x04
	CPI TEMP,0x00
	BREQ PC+0x03+_MCALL_SIZE
	SET
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE C5_OUT_TIME32__LOOP_MS

	LDI LOOP_CNTR,0x03
C5_OUT_TIME32__LOOP_US:
	POP TEMP
	BRTS PC+0x04
	CPI TEMP,0x00
	BREQ PC+0x03+_MCALL_SIZE
	SET
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE C5_OUT_TIME32__LOOP_US

	BRTS PC+0x02+_MCALL_SIZE
	LDI TEMP,'0'
	MCALL C5_OUT_CHAR

	POP TEMP
	STS SREG,TEMP

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
.endif
.endif
