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
;IN: X-адрес уменьшаемого, Y-вычитаемое
;OUT: X-результат, Flags C - переполнение
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP_H,0x00
	MOV TEMP_L,LOOP_CNTR
	ADD XL,TEMP_L
	ADC XH,TEMP_H
	ADD YL,TEMP_L
	ADC YH,TEMP_H

	CLC
	LDS TEMP,SREG
__PRIM_SUB__LOOP:
	STS SREG,TEMP
	LD TEMP_H,-X
	LD TEMP_L,-Y
	SBC TEMP_H,TEMP_L
	LDS TEMP,SREG
	DEC LOOP_CNTR
	BRNE __PRIM_SUB__LOOP
	STS SREG,TEMP

	POP TEMP_L
	POP TEMP_H
	POP TEMP
.endif
