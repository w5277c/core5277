;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_INPUT_GET_CHAR
.else
.set DEF_INPUT_GET_CHAR = 1

.include	"./core/wait.inc"

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
	MCALL C5_IR_VECTOR_SET
	POP TEMP_L
	POP TEMP_H
	LDS ACCUM,SREG
	CLI
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC00)|(1<<ISC01))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP,EIFR
	ORI TEMP,(1<<INTF0)
	STS EIFR,TEMP
	LDS TEMP,EIMSK
	ORI TEMP,(1<<INT0)
	STS EIMSK,TEMP
	CLR TEMP
	STS _C5_INPUT_CHAR,TEMP
	STS SREG,ACCUM

	MCALL C5_WAIT
	LDS TEMP,_C5_INPUT_CHAR
	TST TEMP
	BREQ PC-0x05
	POP ACCUM
	RET
.elseif	INPUT_PORT == INT1_PORT
;TODO после теста добавить аналогичный код, и так далее
.elseif	INPUT_PORT == INT2_PORT
.elseif	INPUT_PORT == INT3_PORT
.elseif	INPUT_PORT == INT4_PORT
.elseif	INPUT_PORT == INT5_PORT
.elseif	INPUT_PORT == INT6_PORT
.elseif	INPUT_PORT == INT7_PORT
.else
	PUSH_Z
	LDI ZH,high(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LDI ZL,low(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LPM ZL,Z
	CLR ZH

	CLI
_INPUT_GET_CHAR__WAIT_START:
	LD TEMP,Z
	ANDI TEMP,EXP2 (INPUT_PORT & 0x0f)
	BRNE _INPUT_GET_CHAR__WAIT_START
	POP_Z

	LDI ACCUM,0x13;0x08
	DEC ACCUM
	BRNE PC-0x01
	POP ACCUM
.endif
_INPUT_GET_CHAR__GOT_START:										;Одновременно точка входа в обработчик прерывания
	PUSH ACCUM
	PUSH TEMP
	PUSH_Z

	LDI ZH,high(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LDI ZL,low(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LPM ZL,Z
	CLR ZH

	;DATA BITS
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,0x08
_INPUT_GET_CHAR__BITES_LOOP:
	LSR TEMP
	LD ACCUM,Z
	NOP
	NOP
	NOP
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
	BREQ PC+0x02
	ORI TEMP,0x80
	LDI ACCUM,0x14;0x08
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
	STS _C5_INPUT_CHAR,TEMP
	LDS TEMP,EIMSK
	ANDI TEMP,~(1<<INT0)
	STS EIMSK,TEMP
_INPUT_GET_CHAR__KEEP_ENABLED:
	LDS TEMP,EIFR
	ORI TEMP,(1<<INTF0)
	STS EIFR,TEMP
	POP TEMP
	POP ACCUM
	RET
.elseif	INPUT_PORT == INT1_PORT
;TODO после теста добавить аналогичный код, и так далее
.elseif	INPUT_PORT == INT2_PORT
.elseif	INPUT_PORT == INT3_PORT
.elseif	INPUT_PORT == INT4_PORT
.elseif	INPUT_PORT == INT5_PORT
.elseif	INPUT_PORT == INT6_PORT
.elseif	INPUT_PORT == INT7_PORT
.else
	MOV ACCUM,TEMP
	POP TEMP
	BREQ _INPUT_GET_CHAR__REPEATE
	STS _C5_INPUT_CHAR,ACCUM

	SEI

	POP ACCUM
	LDS TEMP,_C5_INPUT_CHAR
	RET
.endif


.else
	LDI TEMP,0x00
	RET
.endif
.endif
