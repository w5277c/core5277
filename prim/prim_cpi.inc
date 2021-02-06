;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_CPI
.else
.set DEF_PRIM_CPI = 1

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_CPI:
;--------------------------------------------------------
;Сравнение примитива с константой
;IN: Z-адрес на память с переменными, TEMP-смещение для
;арг.1, Y-адрес на константу из ROM, ACCUM-тип примитива
;OUT: флаги SREG после команды CP
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_Y
	PUSH_Z

	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	PUSH ZH
	PUSH ZL
	LSL YL
	ROL YH
	MOV ZH,YH
	MOV ZL,YL
	POP YL
	POP YH

	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
__PRIM_CPI__LOOP:
	LD TEMP_H,Y+
	LPM TEMP_L,Z+
	CP TEMP_H,TEMP_L
	BRNE __PRIM_CPI__END
	LDS TEMP,SREG
	DEC LOOP_CNTR
	BRNE __PRIM_CPI__LOOP
	STS SREG,TEMP
__PRIM_CPI__END:

	POP_Z
	POP_Y
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
