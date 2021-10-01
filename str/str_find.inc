;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.01.2021	w5277c@gmail.com			Начало
;28.09.2021	w5277c@gmail.com			Y->Z
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_STR_FIND
.else
.set DEF_STR_FIND = 1

.if REPORT_INCLUDES == 0x01
.message "included STR_FIND"
.endif

;--------------------------------------------------------
STR_FIND:
;--------------------------------------------------------
;Поиск подстроки в строке
;IN: X-адрес на основной блок, Z-адрес на искомый блок
;OUT: TEMP_H-адрес начала совпадения в основном блоке
;(0xff-не найдено)
;TEMP_L-смещение для адреса основного блока, указывающее
;на следующий байт после совпадения
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH_X

	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL																;Умножаем на 2 адрес работы с ROM
	ROL ZH
	ORI ZH,0x80
	MOVW YL,ZL

	;Проверка на пустые строки
	SBRC ZH,0x07														;Берем данные из ROM или RAM?
	RJMP PC+0x03
	LD ACCUM,Z	
	RJMP PC+0x04
	ANDI ZH,0b01111111
	LPM ACCUM,Z
	ORI ZH,0b10000000
	CPI ACCUM,0x00
	BRNE __STR_FIND__FB_NEXT
	MOV TEMP_L,XL
	LD ACCUM,X
	CPI ACCUM,0x00
	BREQ __STR_FIND__SUCCESS

__STR_FIND__FB_NEXT:
	MOV TEMP_H,XH
	MOV TEMP_L,XL
	MOVW ZL,YL
__STR_FIND__SB_NEXT:
	SBRC ZH,0x07														;Берем данные из ROM или RAM?
	RJMP PC+0x03
	LD ACCUM,Z+
	RJMP PC+0x04
	ANDI ZH,0b01111111
	LPM ACCUM,Z+
	ORI ZH,0b10000000
	CPI ACCUM,0x00
	BREQ __STR_FIND__SUCCESS
	LD TEMP,X+
	CPI TEMP,0x00
	BREQ __STR_FIND__FAIL
	CP ACCUM,TEMP
	BREQ __STR_FIND__SB_NEXT
	MOV XH,TEMP_H
	MOV XL,TEMP_L
	ADIW XL,0x01
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
	POP_Y
	RET
.endif
