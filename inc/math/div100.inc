;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2020  w5277c@gmail.com        Начало
;08.10.2020  w5277c@gmail.com        Все плохо, храмает точность, например для 59942
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_DIV100
.else
.set DEF_C5_DIV100 = 1

;--------------------------------------------------------
C5_DIV100:
;--------------------------------------------------------
;Деление 16b числа на 100 (неточное!)
;IN: TEMP_H/TEMP_L - 16b число
;OUT: TEMP_H/TEMP_L результат
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH ACCUM
	PUSH TEMP
	PUSH _RESULT_H
	PUSH _RESULT_L

	MOV TEMP,TEMP_H
	MOV ACCUM,TEMP_L
	;Умножаем на 656
	;x*144
	LDI TEMP_EH,144
	CLR TEMP_EL
	MUL ACCUM,TEMP_EH
	MOV TEMP_H,_RESULT_H
	MOV TEMP_L,_RESULT_L
	MUL TEMP,TEMP_EH
	ADD TEMP_H,_RESULT_L
	ADC TEMP_EL,_RESULT_H
	;x*512
	CLR TEMP_EH
	CLR LOOP_CNTR
	LSL ACCUM
	ROL TEMP
	ROL LOOP_CNTR

	ADD TEMP_H,ACCUM
	ADC TEMP_EL,TEMP
	ADC TEMP_EH,LOOP_CNTR
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL

	POP _RESULT_L
	POP _RESULT_H
	POP TEMP
	POP ACCUM
	POP TEMP_EL
	POP TEMP_EH
	RET
.endif
