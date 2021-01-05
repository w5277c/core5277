;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_UPTIME_SUB
.else
.set DEF_UPTIME_SUB = 1

.include	"./prim/common.inc"

;--------------------------------------------------------
UPTIME_SUB:
;--------------------------------------------------------
;Выичтание UPTIME'ов
;IN: X-адрес уменьшаемого, Y-вычитаемое
;OUT: X-результат, Flags C - переполнение
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	LDI TEMP_H,0x00
	LDI TEMP_L,UPTIME_SIZE
	ADD XL,TEMP_L
	ADC XH,TEMP_H
	ADD YL,TEMP_L
	ADC YH,TEMP_H

	LDI LOOP_CNTR,UPTIME_SIZE
	CLC
	LDS TEMP,SREG
__UPTIME_SUB__LOOP:
	STS SREG,TEMP
	LD TEMP_H,-X
	LD TEMP_L,-Y
	SBC TEMP_H,TEMP_L
	LDS TEMP,SREG
	DEC LOOP_CNTR
	BRNE __UPTIME_SUB__LOOP
	STS SREG,TEMP

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
.endif
