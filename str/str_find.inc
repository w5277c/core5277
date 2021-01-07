;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.01.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_STR_FIND
.else
.set DEF_STR_FIND = 1

;--------------------------------------------------------
STR_FIND:
;--------------------------------------------------------
;Поиск подстроки в строке
;IN: X-адрес на основной блок, Y-адрес на искомый блок
;OUT: TEMP_H-адрес начала совпадения в основном блоке
;(0xff-не найдено)
;TEMP_L-адрес следующего байта после совпадения в
;основном блоке
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH_X

	SBRS YH,0x07
	RJMP PC+0x04
	LSL YL																;Умножаем на 2 адрес работы с ROM
	ROL YH
	ORI YH,0x80

	;Проверка на пустые строки
	MOV ZH,YH
	MOV ZL,YL
	ANDI ZH,0x7f
	SBRC YH,0x07														;Берем данные из ROM или RAM?
	LPM ACCUM,Z
	SBRS YH,0x07														;Берем данные из ROM или RAM?
	LD ACCUM,Z
	CPI ACCUM,0x00
	BRNE __STR_FIND__FB_NEXT
	MOV TEMP_L,XL
	LD ACCUM,X
	CPI ACCUM,0x00
	BREQ __STR_FIND__SUCCESS

__STR_FIND__FB_NEXT:
	MOV TEMP_H,XH
	MOV TEMP_L,XL
	MOV ZH,YH
	MOV ZL,YL
__STR_FIND__SB_NEXT:
	ANDI ZH,0x7f
	SBRC YH,0x07														;Берем данные из ROM или RAM?
	LPM ACCUM,Z+
	SBRS YH,0x07														;Берем данные из ROM или RAM?
	LD ACCUM,Z+
	CPI ACCUM,0x00
	BREQ __STR_FIND__SUCCESS
	LD TEMP,X+
	CPI TEMP,0x00
	BREQ __STR_FIND__FAIL
	CP ACCUM,TEMP
	BREQ __STR_FIND__SB_NEXT
	LDI XL,0x01
	LDI XH,0x00
	ADD XL,TEMP_L
	ADC XH,TEMP_H
	RJMP __STR_FIND__FB_NEXT
__STR_FIND__SUCCESS:
	MOV ACCUM,XL
	POP_X
	SUB ACCUM,XL
	SUB TEMP_L,XL
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,ACCUM
	RJMP __STR_FIND__END
__STR_FIND__FAIL:
	POP_X
	LDI TEMP_H,0xff
	LDI TEMP_L,0x00
__STR_FIND__END:
	POP ACCUM
	POP TEMP
	POP_Z
	RET
.endif
