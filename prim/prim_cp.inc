;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com			Начало
;06.02.2021  w5277c@gmail.com			Изменены аргументы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_CP
.else
.set DEF_PRIM_CP = 1

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_CP:
;--------------------------------------------------------
;Сравнение примитивов
;IN: Z-адрес на память с переменными, TEMP_H-смещение для
;арг.1, TEMP_L-смещение для арг.2, ACCUM-тип примитива
;OUT: флаги SREG после команды CP
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	MCALL _PRIM_OFFSETS_L

	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
__PRIM_CP__LOOP:
	LD TEMP_H,Z+
	LD TEMP_L,X+
	CP TEMP_H,TEMP_L
	BRNE __PRIM_CP__END
	LDS TEMP,SREG
	DEC LOOP_CNTR
	BRNE __PRIM_CP__LOOP
	STS SREG,TEMP
__PRIM_CP__END:

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
