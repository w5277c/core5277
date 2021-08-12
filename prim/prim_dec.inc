;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.04.2021	w5277c@gmail.com			Начало
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_DEC
.else
.set DEF_PRIM_DEC = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM_INC"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_DEC:
;--------------------------------------------------------
;Уменьшение примитива на 1
;IN: Y-адрес на память с переменными, TEMP-смещение на
;переменную, ACCUM-тип примитива
;OUT: флаги SREG после команды DEC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH LOOP_CNTR
	PUSH_Y

	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
	ADD YL,TEMP
	ADC YH,C0x00
	ADD YL,LOOP_CNTR
	ADC YH,TEMP

	SEC
	LDS TEMP,SREG
__PRIM_DEC__LOOP:
	LD TEMP_H,-Y
	STS SREG,TEMP
	SBCI TEMP_H,0x00
	LDS TEMP,SREG
	ST Y,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM_DEC__LOOP
	STS SREG,TEMP

	POP_Y
	POP LOOP_CNTR
	POP TEMP_H
	POP TEMP
	RET
.endif
