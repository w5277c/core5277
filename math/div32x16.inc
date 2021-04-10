;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.10.2020  w5277c@gmail.com			Начало
;10.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DIV32X16
.else
.set DEF_DIV32X16 = 1

.if REPORT_INCLUDES == 0x01
.message "included DIV32X16"
.endif

;--------------------------------------------------------
DIV32X16:
;--------------------------------------------------------
;Деление 32b числа на 16б число (A/B=C)
;IN: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L - A, X - B
;OUT: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L - C
;-START--------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH_Y
	PUSH_Z

	CLR TEMP
	LDI LOOP_CNTR,33
	SUB YL,YL
	CLR YH
	CLR ZL
	CLR ZH
_DIV32X16__LOOP:
	ROL TEMP_L
	ROL TEMP_H
	ROL TEMP_EL
	ROL TEMP_EH
	DEC LOOP_CNTR
	BREQ _DIV32X16__END
	ROL YL
	ROL YH
	ROL ZL
	ROL ZH
	SUB YL,XL
	SBC YH,XH
	SBC ZL,TEMP
	SBC ZH,TEMP
	BRCC _DIV32X16__SKIP
	ADD YL,XL
	ADC YH,XH
	ADC ZL,TEMP
	ADC ZH,TEMP
	CLC
	RJMP _DIV32X16__LOOP
_DIV32X16__SKIP:
	SEC
	RJMP _DIV32X16__LOOP
_DIV32X16__END:

	POP_Z
	POP_Y
	POP TEMP
	POP LOOP_CNTR
	RET
.endif
