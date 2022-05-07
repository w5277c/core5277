;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало
;08.01.2022	w5277c@gmail.com			Багфикс
;-----------------------------------------------------------------------------------------------------------------------
;TODO перейти на использование MUL8X8
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_MUL32X16
.else
.set DEF_MUL32X16 = 1

.if REPORT_INCLUDES == 0x01
.message "included MUL32X16"
.endif

.include	"./math/mul8x8.inc"

;--------------------------------------------------------
MUL32X16:
;--------------------------------------------------------
;Умножение 32б числа на 16б число
;IN: TEMP_EH/EL/H/L-32б число, X-16б число
;OUT: TEMP_EH/EL/H/L-32б результат
;--------------------------------------------------------
	PUSH _RESULT_H
	PUSH _RESULT_L
	PUSH_Y
	PUSH_Z
	PUSH TEMP

	CLR ZH
	CLR ZL
	CLR YL
	CLR YH
	CLR TEMP

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_L,XL
	MOV TEMP,XL
	MCALL MUL8X8
	MOV YL,TEMP_L
	MOV YH,TEMP_H
	POP TEMP_L
	POP TEMP_H

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_H,XL
	MOV TEMP_L,TEMP_H
	MOV TEMP,XL
	MCALL MUL8X8
	ADD YH,TEMP_L
	ADC ZL,TEMP_H
	POP TEMP_L
	POP TEMP_H
	ADC ZH,C0x00

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_EL,XL
	MOV TEMP_L,TEMP_EL
	MOV TEMP,XL
	MCALL MUL8X8
	ADD ZL,TEMP_L
	ADC ZH,TEMP_H
	POP TEMP_L
	POP TEMP_H

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_EH,XL
	MOV TEMP_L,TEMP_EH
	MOV TEMP,XL
	MCALL MUL8X8
	ADD YH,TEMP_L
	POP TEMP_L
	POP TEMP_H

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_L,XH
	MOV TEMP,XH
	MCALL MUL8X8
	ADD YH,TEMP_L
	ADC ZL,TEMP_H
	ADC ZH,C0x00
	POP TEMP_L
	POP TEMP_H

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_H,XH
	MOV TEMP_L,TEMP_H
	MOV TEMP,XH
	MCALL MUL8X8
	ADD ZL,TEMP_L
	ADC ZH,TEMP_H
	POP TEMP_L
	POP TEMP_H

	PUSH TEMP_H
	PUSH TEMP_L
	;MUL TEMP_EL,XH
	MOV TEMP_L,TEMP_EL
	MOV TEMP,XH
	MCALL MUL8X8
	ADD ZH,TEMP_L
	POP TEMP_L
	POP TEMP_H

	MOV TEMP_L,YL
	MOV TEMP_H,YH
	MOV TEMP_EL,ZL
	MOV TEMP_EH,ZH

	POP TEMP
	POP_Z
	POP_Y
	POP _RESULT_L
	POP _RESULT_H
	RET
.endif
