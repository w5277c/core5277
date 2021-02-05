;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_ADD
.else
.set DEF_PRIM_ADD = 1

.include	"./prim/common.inc"

;--------------------------------------------------------
PRIM_ADD:
;--------------------------------------------------------
;Сложение примитивов
;IN: X-адрес слагаемого и результата, Y-адрес слагаемого
;LOOP_CNTR-размер переменной
;OUT: Flags C - переполнение
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_X

	LDI TEMP_H,0x00
	MOV TEMP_L,LOOP_CNTR
	ADD YL,TEMP_L
	ADC YH,TEMP_H
	DEC TEMP_L
	ADD XL,TEMP_L
	ADC XH,TEMP_H

	CLC
	LDS TEMP,SREG
__PRIM_ADD__LOOP:
	STS SREG,TEMP
	LD TEMP_H,X
	LD TEMP_L,-Y
	LDS TEMP,SREG
	ADC TEMP_H,TEMP_L
	ST X,TEMP_H
	SUBI XL,0x01
	SBCI XH,0x00
	DEC LOOP_CNTR
	BRNE __PRIM_ADD__LOOP
	STS SREG,TEMP

	POP_X
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
