;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.10.2020	w5277c@gmail.com			Начало
;10.12.2020	w5277c@gmail.com			-с5
;18.10.2021	w5277c@gmail.com			Возврат остатка
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
;Деление 32b числа на 16б число
;IN: TEMP_EH/EL/H/L-32b делимое, X-16b делитель
;OUT: TEMP_EH/EL/H/L-32b результат, X-16b остаток
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH YH
	PUSH YL
	PUSH ZL

	LDI LOOP_CNTR,0x21
	SUB YL,YL
	CLR YH
	CLR ZL
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
	SUB YL,XL
	SBC YH,XH
	SBC ZL,C0x00
	BRCC _DIV32X16__SKIP
	ADD YL,XL
	ADC YH,XH
	ADC ZL,C0x00
	CLC
	RJMP _DIV32X16__LOOP
_DIV32X16__SKIP:
	SEC
	RJMP _DIV32X16__LOOP
_DIV32X16__END:
	MOVW XL,YL

	POP ZL
	POP YL
	POP YH
	POP TEMP
	POP LOOP_CNTR
	RET
.endif
