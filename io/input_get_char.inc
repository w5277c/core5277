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
	PUSH ACCUM

_INPUT_GET_CHAR__REPEATE:
.if		INPUT_PORT == INT0_PORT
	LDI ACCUM,C5_IR_INT0
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,high(_INPUT_GET_CHAR__GOT_START)
	LDI TEMP_L,low(_INPUT_GET_CHAR__GOT_START)
	MCALL C5_SET_VECTOR
	POP TEMP_L
	POP TEMP_H
	LDS ACCUM,SREG
	CLI
	LDS TEMP,EICRA
	ANDI TEMP,0xfc
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP,EIMSK
	ORI TEMP,0x01
	STS EIMSK,TEMP
	CLR _C5_INPUT_CHAR
	STS SREG,ACCUM

	MCALL C5_WAIT
	TST _C5_INPUT_CHAR
	BREQ PC-0x04
	MOV TEMP,_C5_INPUT_CHAR
	POP ACCUM
	RET
.elseif	INPUT_PORT == INT1_PORT
.elseif	INPUT_PORT == INT2_PORT
.elseif	INPUT_PORT == INT3_PORT
.elseif	INPUT_PORT == INT4_PORT
.elseif	INPUT_PORT == INT5_PORT
.elseif	INPUT_PORT == INT6_PORT
.elseif	INPUT_PORT == INT7_PORT
.else
	PUSH_Z
	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,((INPUT_PORT>>4)*0x03)+0x02
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH

	LDS ACCUM,SREG
	CLI
_INPUT_GET_CHAR__WAIT_START:
	LD TEMP,Z
	ANDI TEMP,EXP2 (INPUT_PORT & 0x0f)
	BRNE _INPUT_GET_CHAR__WAIT_START
	POP_Z
	POP ACCUM
.endif
_INPUT_GET_CHAR__GOT_START:										;Одновременно точка входа в обработчик прерывания
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP
	CLI

	PUSH ACCUM
	PUSH_Z
	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,((INPUT_PORT>>4)*0x03)+0x02
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH

	;DATA BITS
	PUSH LOOP_CNTR
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
	POP LOOP_CNTR

	LD ACCUM,Z
	POP_Z
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
.if	INPUT_PORT == INT0_PORT
	BREQ _INPUT_GET_CHAR__KEEP_ENABLED
	MOV _C5_INPUT_CHAR,TEMP
	LDS TEMP,EIMSK
	ANDI TEMP,~(0x01)
	STS EIMSK,TEMP
_INPUT_GET_CHAR__KEEP_ENABLED:
	LDS TEMP,EIFR
	ORI TEMP,0x01
	STS EIFR,TEMP
	POP ACCUM
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RET
.elseif	INPUT_PORT == INT1_PORT
.elseif	INPUT_PORT == INT2_PORT
.elseif	INPUT_PORT == INT3_PORT
.elseif	INPUT_PORT == INT4_PORT
.elseif	INPUT_PORT == INT5_PORT
.elseif	INPUT_PORT == INT6_PORT
.elseif	INPUT_PORT == INT7_PORT
.else
	BREQ _INPUT_GET_CHAR__REPEATE
.endif
	POP ACCUM
	RET


.else
	LDI TEMP,0x00
	RET
.endif
.endif
