;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_MUL24X8
.else
.set DEF_MUL24X8 = 1

;--------------------------------------------------------
MUL24X8:
;--------------------------------------------------------
;Умножение 24b числа на 8б число
;IN: TEMP_EL,TEMP_H,TEMP_L - 24b число, TEMP - 8b число
;OUT: TEMP_EH, TEMP_EL, TEMP_H, TEMP_L - 32b результат
;-START--------------------------------------------------
	PUSH _RESULT_H
	PUSH _RESULT_L
	PUSH_Y
	PUSH_X

	CLR YH
	CLR YL

	MUL TEMP_L,TEMP
	MOV XL,_RESULT_L
	MOV XH,_RESULT_H

	MUL TEMP_H,TEMP
	ADD XH,_RESULT_L
	ADC YL,_RESULT_H

	MUL TEMP_EL,TEMP
	ADD YL,_RESULT_L
	ADC YH,_RESULT_H

	MOV TEMP_EH,YH
	MOV TEMP_EL,YL
	MOV TEMP_H,XH
	MOV TEMP_L,XL

	POP_X
	POP_Y
	POP _RESULT_L
	POP _RESULT_H
	RET
.endif
