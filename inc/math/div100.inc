;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_DIV10
.else
.set DEF_CORE5277_DIV10 = 1

;--------------------------------------------------------
CORE5277_DIV10:
;--------------------------------------------------------
;Деление 16b числа на 100
;IN: TEMP_H/TEMP_L - 16b число
;OUT: TEMP результат
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH ACCUM
	PUSH TEMP
	PUSH RESULT_H
	PUSH RESULT_L

	MOV TEMP,TEMP_H
	MOV ACCUM,TEMP_L
	;Умножаем на 656
	;x*144
	LDI TEMP_EH,144
	CLR TEMP_EL
	MUL TEMP,TEMP_EH
	MOV TEMP_L,_RESULT_L
	MOV TEMP_H,_RESULT_H
	MUL ACCUM,TEMP_EH
	ADD TEMP_H,RESULT_L
	ADC TEMP_EL,_RESULT_H
	;x*512
	LDI LOOP_CNTR
	LSL TEMP
	ROL ACCUM
	ADD TEMP_H,TEMP
	ADC TEMP_EL,ACCUM
	MOV TEMP,TEMP_EL

	POP RESULT_L
	POP RESULT_H
	POP TEMP
	POP ACCUM
	POP TEMP_EL
	POP TEMP_EH
	RET
.endif
