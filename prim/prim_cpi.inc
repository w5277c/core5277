;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021	w5277c@gmail.com			Начало
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_CPI
.else
.set DEF_PRIM_CPI = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM_CPI"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_CPI:
;--------------------------------------------------------
;Сравнение примитива с константой
;IN: Y-адрес на память с переменными, TEMP-смещение для
;арг.1, Z-адрес на константу из ROM, ACCUM-тип примитива
;OUT: флаги SREG после команды CP
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_Y
	PUSH_Z

	ADD YL,TEMP
	ADC YH,C0x00
	LSL ZL
	ROL ZH

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
