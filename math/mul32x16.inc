;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_MUL32X16
.else
.set DEF_MUL32X16 = 1

;--------------------------------------------------------
MUL32X16:
;--------------------------------------------------------
;Умножение 32б числа на 16б число
;IN: TEMP_EH/EL/H/L-32б число, X-16б число
;OUT: TEMP_EH/EL/H/L-32б результат
;-START--------------------------------------------------
	PUSH _RESULT_H
	PUSH _RESULT_L
	PUSH_Y
	PUSH_Z


	CLR YL
	CLR YH
	CLR TEMP

	MUL TEMP_L,XL
	MOV ZL,_RESULT_L
	MOV ZH,_RESULT_H
	MUL TEMP_H,XL
	ADD ZH,_RESULT_L
	ADC YL,_RESULT_H
	ADC YH,TEMP
	MUL TEMP_EL,XL
	ADD YL,_RESULT_L
	ADC YH,_RESULT_H
	MUL TEMP_EH,XL
	ADD YH,_RESULT_L

	MUL TEMP_L,XH
	ADD ZH,_RESULT_H
	ADC YL,_RESULT_L
	ADC YH,TEMP
	MUL TEMP_H,XH
	ADD YL,_RESULT_L
	ADC YH,_RESULT_H
	MUL TEMP_EL,XH
	ADD YH,_RESULT_L

	MOV TEMP_L,ZL
	MOV TEMP_H,ZH
	MOV TEMP_EL,YL
	MOV TEMP_EH,YH

	POP_Z
	POP_Y
	POP _RESULT_L
	POP _RESULT_H
	RET
.endif
