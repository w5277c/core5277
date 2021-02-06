;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_INC
.else
.set DEF_PRIM_INC = 1

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_INC:
;--------------------------------------------------------
;Сложение примитивов
;IN: Z-адрес на память с переменными, TEMP-смещение на
;переменную, ACCUM-тип примитива
;OUT: флаги SREG после команды INC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_Z

	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	ADD ZL,LOOP_CNTR
	ADC ZH,TEMP

	SEC
	LDS TEMP,SREG
	LDI TEMP_L,0x00
__PRIM_INC__LOOP:
	LD TEMP_H,-Z
	STS SREG,TEMP
	ADC TEMP_H,TEMP_L
	LDS TEMP,SREG
	ST Z,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM_INC__LOOP
	STS SREG,TEMP

	POP_Z
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
