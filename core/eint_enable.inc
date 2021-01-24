;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_EINT_ENABLE
.else
.set DEF_C5_EINT_ENABLE = 1

;--------------------------------------------------------
C5_EINT_ENBALE:
;--------------------------------------------------------
;Включение внешних прерываний
;IN: PID-ид задачи/драйвера,
;TEMP_H-прерывание (C5_IR_INTn)
;TEMP_L-тип реакции(C5_ISC_...)
;Y-адрес обработчика прерывания
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	SBRS TEMP_H,0x02
.ifdef EICRA
	LDS TEMP,EICRA
.endif
.ifdef MCUCR
	LDS TEMP,MCUCR
.endif
	SBRC TEMP_H,0x02
.ifdef EICRB
	LDS TEMP,EICRB
.endif

	ANDI TEMP_L,0x03

	PUSH TEMP_H
	SBRS TEMP_H,0x02
	RJMP PC+0x03
	LSR TEMP_H
	LSR TEMP_H
	BREQ _C5_EINT_ENBALE__SKIP

	MOV LOOP_CNTR,TEMP_H
	LDI TEMP_H,0xfc
_C5_EINT_ENBALE__LOOP:
	LSL TEMP_H
	LSL TEMP_H
	LSL TEMP_L
	LSL TEMP_L
	DEC LOOP_CNTR
	BRNE _C5_EINT_ENBALE__LOOP

	AND TEMP,TEMP_H
	OR TEMP,TEMP_L

	POP ACCUM
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	MCALL C5_IR_VECTOR_SET

_C5_EINT_ENBALE__SKIP:
	SBRS TEMP_H,0x02
.ifdef EICRA
	STS EICRA,TEMP
.endif
.ifdef MCUCR
	STS MCUCR,TEMP
.endif
	SBRC TEMP_H,0x02
.ifdef EICRB
	STS EICRB,TEMP
.endif

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
