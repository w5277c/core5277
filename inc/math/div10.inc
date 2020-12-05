;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_DIV10
.else
.set DEF_C5_DIV10 = 1

;--------------------------------------------------------
C5_DIV10:
;--------------------------------------------------------
;Деление 8b числа на 10
;IN: TEMP - 8b число
;OUT: TEMP_L результат
;--------------------------------------------------------
	PUSH _RESULT_H
	PUSH _RESULT_L
	PUSH ACCUM
	PUSH TEMP

	LDI ACCUM,205														;204.8
	MUL TEMP,ACCUM
	MOV TEMP_L,_RESULT_H
	LSR TEMP_L
	LSR TEMP_L
	LSR TEMP_L

	POP TEMP
	POP ACCUM
	POP _RESULT_L
	POP _RESULT_H
	RET
.endif