;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO перейти на использование MUL8X8
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_MUL32X16
.else
.set DEF_MUL32X16 = 1

.if REPORT_INCLUDES == 0x01
.message "included MUL32X16"
.endif

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
	PUSH TEMP

	CLR ZH
	CLR ZL
	CLR YL
	CLR YH
	CLR TEMP


	MUL TEMP_L,XL
	MOV YL,_RESULT_L
	MOV YH,_RESULT_H

	MUL TEMP_H,XL
	ADD YH,_RESULT_L
	ADC ZL,_RESULT_H
	ADC ZH,TEMP

	MUL TEMP_EL,XL
	ADD ZL,_RESULT_L
	ADC ZH,_RESULT_H

	MUL TEMP_EH,XL
	ADD YH,_RESULT_L


	MUL TEMP_L,XH
	ADD YH,_RESULT_L
	ADC ZL,_RESULT_H
	ADC ZH,TEMP

	MUL TEMP_H,XH
	ADD ZL,_RESULT_L
	ADC ZH,_RESULT_H

	MUL TEMP_EL,XH
	ADD ZH,_RESULT_L

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
