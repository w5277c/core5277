;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_INPUT_GET_CHAR
.else
.set DEF_INPUT_GET_CHAR = 1
;--------------------------------------------------------
INPUT_GET_CHAR:
;Считываем с символ в виде UART RX
;OUT: TEMP-символ
;--------------------------------------------------------
.ifdef INPUT_PORT
	PUSH ZH
	PUSH ZL
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDS ACCUM,SREG
	PUSH ACCUM
	CLI

	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,((INPUT_PORT>>4)*0x03)+0x02
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH

_INPUT_GET_CHAR__WAIT_START:
	LD ACCUM,Z
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
	BRNE _INPUT_GET_CHAR__WAIT_START
	LDI ACCUM,0x08
	DEC ACCUM
	BRNE PC-0x01

	;DATA BITS
	CLR TEMP
	LDI LOOP_CNTR,0x08
_INPUT_GET_CHAR__BITES_LOOP:
	LD ACCUM,Z
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
	BRNE PC+0x02
	ORI TEMP,0x01
	LSL TEMP
	LDI ACCUM,0x08
	DEC ACCUM
	BRNE PC-0x01
	DEC LOOP_CNTR
	BRNE _INPUT_GET_CHAR__BITES_LOOP

;	LD ACCUM,Z
;	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
;	BREQ _INPUT_GET_CHAR__WAIT_START

	POP ACCUM
	STS SREG,ACCUM

	POP LOOP_CNTR
	POP ACCUM
	POP ZL
	POP ZH
	RET
.else
	LDI TEMP,0x00
	RET
.endif
.endif
