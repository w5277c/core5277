;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_MUL10
.else
.set DEF_C5_MUL10 = 1

;--------------------------------------------------------
C5_MUL10:
;--------------------------------------------------------
;Умножение 8b числа на 10
;IN: TEMP - 8b число
;OUT: TEMP_H,TEMP_L результат
;--------------------------------------------------------
	PUSH _RESULT_H
	PUSH _RESULT_L
	PUSH ACCUM

	LDI ACCUM,10
	MUL TEMP,ACCUM
	MOV TEMP_H,_RESULT_H
	MOV TEMP_L,_RESULT_L

	POP ACCUM
	POP _RESULT_L
	POP _RESULT_H
	RET
.endif