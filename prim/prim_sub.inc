;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_SUB
.else
.set DEF_PRIM_SUB = 1

.include	"./prim/common.inc"

;--------------------------------------------------------
PRIM_SUB:
;--------------------------------------------------------
;Выичтание UPTIME'ов
;IN: X-адрес уменьшаемого и результата, Y-вычитаемое
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
__PRIM_SUB__LOOP:
	STS SREG,TEMP
	LD TEMP_H,X
	LD TEMP_L,-Y
	LDS TEMP,SREG
	SBC TEMP_H,TEMP_L
	ST X,TEMP_H
	SUBI XL,0x01
	SBCI XH,0x00
	DEC LOOP_CNTR
	BRNE __PRIM_SUB__LOOP
	STS SREG,TEMP

	POP_X
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
