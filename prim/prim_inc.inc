;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021	w5277c@gmail.com			Начало
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_INC
.else
.set DEF_PRIM_INC = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM_INC"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_INC:
;--------------------------------------------------------
;Увеличение примитива на 1
;IN: Y-адрес на память с переменными, TEMP-смещение на
;переменную, ACCUM-тип примитива
;OUT: флаги SREG после команды INC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
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
	LDI TEMP_L,0x00
__PRIM_INC__LOOP:
	LD TEMP_H,-Y
	STS SREG,TEMP
	ADC TEMP_H,TEMP_L
	LDS TEMP,SREG
	ST Y,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM_INC__LOOP
	STS SREG,TEMP

	POP_Y
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
