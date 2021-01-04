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
;Поиск блока в блоке памяти
;IN: X-адрес на основной блок, Y-адрес на искомый блок
;OUT: TEMP-смещение в оснвоном блоке (0xff-не найдено)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM
	PUSH_X

	SBRS YH,0x07
	RJMP PC+0x04
	LSL YL																;Умножаем на 2 адрес рабоыт с ROM
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
	POP_X
	SUB TEMP_L,XL
	MOV TEMP,TEMP_L
	RJMP __STR_FIND__END
__STR_FIND__FAIL:
	POP_X
	LDI TEMP,0xff
__STR_FIND__END:
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
