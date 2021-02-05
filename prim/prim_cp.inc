;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_CP
.else
.set DEF_PRIM_CP = 1

.include	"./prim/common.inc"

;--------------------------------------------------------
PRIM_CP:
;--------------------------------------------------------
;Сравнение примитивов
;IN: X-адрес арг.1, Y-адрес арг.2, LOOP_CNTR-SIZE
;OUT: Flags C - арг.2>арг.1, Flag Z - арг.1=арг.2
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	SBRS YH,0x07
	RJMP PC+0x04
	LSL YL																;Умножаем на 2 адрес работы с ROM
	ROL YH
	ORI YH,0x80


	MOV ZH,YH
	MOV ZL,YL
	ANDI ZH,0x7f
__PRIM_CP__LOOP:
	LD TEMP_H,X+
	SBRC YH,0x07														;Берем данные из ROM или RAM?
	LPM TEMP_L,Z+
	SBRS YH,0x07														;Берем данные из ROM или RAM?
	LD TEMP_L,Z+
	CP TEMP_H,TEMP_L
	BRNE __PRIM_CP__END
	DEC LOOP_CNTR
	BRNE __PRIM_CP__LOOP
	CLC
	SEZ
__PRIM_CP__END:

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	POP_Y
	POP_X
	RET
.endif
