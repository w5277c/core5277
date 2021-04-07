;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.04.2021  w5277c@gmail.com			Начало
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
;IN: Z-адрес на память с переменными, TEMP-смещение на
;переменную, ACCUM-тип примитива
;OUT: флаги SREG после команды DEC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
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
__PRIM_DEC__LOOP:
	LD TEMP_H,-Z
	STS SREG,TEMP
	SBCI TEMP_H,0x00
	LDS TEMP,SREG
	ST Z,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM_DEC__LOOP
	STS SREG,TEMP

	POP_Z
	POP LOOP_CNTR
	POP TEMP_H
	POP TEMP
	RET
.endif
