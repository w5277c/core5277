;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.12.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_AVREMUL
.else
.set DEF_C5_AVREMUL = 1

.include	"./conv/bitnum_to_num.inc"
;---REGISTERS--------------------------------------------;Регистры
	.EQU	C5_AVREMUL_R16							= 0x00			;
	.EQU	C5_AVREMUL_R17							= 0x01			;
	.EQU	C5_AVREMUL_R18							= 0x02			;
	.EQU	C5_AVREMUL_R19							= 0x03			;
	.EQU	C5_AVREMUL_R20							= 0x04			;
	.EQU	C5_AVREMUL_R21							= 0x05			;
	.EQU	C5_AVREMUL_R22							= 0x06			;
	.EQU	C5_AVREMUL_R23							= 0x07			;
	.EQU	C5_AVREMUL_R24							= 0x08			;
	.EQU	C5_AVREMUL_R25							= 0x09			;
	.EQU	C5_AVREMUL_RXL							= 0x0a			;
	.EQU	C5_AVREMUL_RXH							= 0x0b			;
	.EQU	C5_AVREMUL_RYL							= 0x0c			;
	.EQU	C5_AVREMUL_RYH							= 0x0d			;
	.EQU	C5_AVREMUL_RZL							= 0x0e			;
	.EQU	C5_AVREMUL_RZH							= 0x0f			;

;FLAGS - sreg
;X - CP



EVREMUL_XYZ_IN:
	PUSH TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	ANDI TEMP_L,0x03
	BRNE PC+0x02
	LDI TEMP_L,0x01
	;01-Z,02-Y,03-X
	PUSH_Y
	SUB YL,TEMP_L
	SBCI YH,0x00
	LDD ZH,Y+C5_AVREMUL_RZH+0x01
	LDD ZL,Y+C5_AVREMUL_RZH+0x00
	POP_Y
	ANDI TEMP_H,0x0f
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_H
	ADC YH,TEMP
	LDD ACCUM,Y+C5_AVREMUL_R16
	POP_Y
	POP TEMP_L
	ICALL
	RET

EVREMUL_XYZ_OUT:
	LSR TEMP_L
	LSR TEMP_L
	ANDI TEMP_L,0x03
	BRNE PC+0x02
	LDI TEMP_L,0x01
	;01-Z,02-Y,03-X
	PUSH_Y
	SUB YL,TEMP_L
	SBCI YH,0x00
	STD Y+C5_AVREMUL_RZH+0x01,ZH
	STD Y+C5_AVREMUL_RZH+0x00,ZL
	POP_Y
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_H
	ADC YH,TEMP
	STD Y+C5_AVREMUL_R16,ACCUM
	POP_Y
	RET

AVREMUL_TABLE1:
	CPC ACCUM,TEMP														;0000 01rd dddd rrrr
	RET
	SBC ACCUM,TEMP														;0000 10rd dddd rrrr
	RET
	ADD ACCUM,TEMP														;0000 11rd dddd rrrr
	RET
	RJMP AVREMUL_TABLE1_CPSE										;0001 00rd dddd rrrr
	NOP
	CP ACCUM,TEMP														;0001 01rd dddd rrrr
	RET
	SUB ACCUM,TEMP														;0001 10rd dddd rrrr
	RET
	ADC ACCUM,TEMP														;0001 11rd dddd rrrr
	RET
	AND ACCUM,TEMP														;0010 00rd dddd rrrr
	RET
	EOR ACCUM,TEMP														;0010 01rd dddd rrrr
	RET
	OR ACCUM,TEMP														;0010 10rd dddd rrrr
	RET
	MOV ACCUM,TEMP														;0010 11rd dddd rrrr
	RET
AVREMUL_TABLE1_CPSE:
	PUSH FLAGS
	CP ACCUM,TEMP
	BRNE PC+0x02
	ADIW X,0x02
	POP FLAGS
	RET

AVREMUL_TABLE2:
	CP ACCUM,TEMP														;0011 KKKK dddd KKKK (CPI)
	RET
	SBC ACCUM,TEMP														;0100 KKKK dddd KKKK (SBCI)
	RET
	SUB ACCUM,TEMP														;0101 KKKK dddd KKKK (SUBI)
	RET
	OR ACCUM,TEMP														;0110 KKKK dddd KKKK (ORI)
	RET
	AND ACCUM,TEMP														;0111 KKKK dddd KKKK (ANDI)
	RET

AVREMUL_TABLE3:
	PUSH_Z																;1001 000d dddd 0000 (LDS)
	LD ZH,X+
	LD ZL,X+
	LD TEMP_H,Z
	POP_Z
	RET
	LD ACCUM,Z+															;1001 000d dddd 0001 (LD Z+)
	RET
	LD ACCUM,-Z															;1001 000d dddd 0010 (LD -Z)
	RET
	RET																	;1001 000d dddd 0011
	NOP
	LD ACCUM,Z															;1001 000d dddd 0100 (LPM Z)
	RET
	LD ACCUM,Z+															;1001 000d dddd 0101 (LPM Z+)
	LD TEMP,Z+															;for increment
	RET


	RET																	;1001 000d dddd 0110 (ELPM Z)
	RET																	;1001 000d dddd 0111 (ELPM Z+)
	RET																	;1001 000d dddd 1000
	PUSH Z																;1001 000d dddd 1001 (LD Y+)
	LDD ZH,Y+C5_AVREMUL_RYH
	LDD ZL,Y+C5_AVREMUL_RYL
	LD TEMP_H,Z+
	STD Y+C5_AVREMUL_RYH,ZH
	STD Y+C5_AVREMUL_RYL,ZL
	POP_Z
	RET
	PUSH Z																;1001 000d dddd 1010 (LD -Y)
	LDD ZH,Y+C5_AVREMUL_RYH
	LDD ZL,Y+C5_AVREMUL_RYL
	LD TEMP_H,-Z
	STD Y+C5_AVREMUL_RYH,ZH
	STD Y+C5_AVREMUL_RYL,ZL
	POP_Z
	RET
	RET																	;1001 000d dddd 1011
	RET																	;1001 000d dddd 1000

AVREMUL_TABLE4:
	COM ACCUM															;1001 010d dddd 0000
	RET
	NEG ACCUM															;1001 010d dddd 0001
	RET
	SWAP ACCUM															;1001 010d dddd 0010
	RET
	INC ACCUM															;1001 010d dddd 0011
	RET
	NOP																	;1001 010d dddd 0100	(reserved)
	RET
	ASR ACCUM															;1001 010d dddd 0101
	RET
	LSR ACCUM															;1001 010d dddd 0110
	RET
	ROR ACCUM															;1001 010d dddd 0111
	RET
	MOV TEMP,ACCUM														;1001 0100 Bbbb 1000 (set/clear sreg bit)
	ANDI TEMP,0x07
	MCALL BITNUM_TO_NUM
	PUSH TEMP
	COM TEMP
	ANDI FLAGS,TEMP
	POP TEMP
	SBRS ACCUM,0x03
	ORI FLAGS,TEMP
	RET


;--------------------------------------------------------
AVREMUL_EXEC:
;--------------------------------------------------------
;X - PC, FLAGS - sreg
;--------------------------------------------------------
	LD TEMP_H,X+
	LD TEMP_L,X+
	CPI TEMP_H,00000000b
	BREQ AVREMUL_EXEC
	;00cc ccrd dddd rrrr		2-operand instructions
	;1111 1100 0000 0000
	MOV TEMP,TEMP_H
	ANDI TEMP,11111100b
	CPI TEMP,0x04
	BRCS AVREMUL_EXEC_UNSUPPORTED
	CPI TEMP,0x2c+0x01
	BRCC AVREMUL_EXEC_L1
	;CPC/CP,SBC/SUB,ADD/ADC,CPSE,AND,EOR,OR,MOV
		;Получаю адрес процедуры
		MOV TEMP,TEMP_H
		LDI ZH,high(AVREMUL_TABLE1)
		LDI ZL,low(AVREMUL_TABLE1)
		ANDI TEMP,0x3c
		ROL TEMP														;?
		ADD ZL,TEMP_H
		CLR TEMP
		ADC ZH,TEMP
		;Получаю номера регистров
		MOV TEMP_H,TEMP_L
		ANDI TEMP_L,0x0f
		SWAP TEMP_H
		ANDI TEMP_H,0x0f
		;получаю адреса регистров
		PUSH_Y
		ADD YL,TEMP_H
		CLR TEMP
		ADC YH,TEMP
		LDD ACCUM,Y+C5_AVREMUL_R16
		POP_Y
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP
		ADC YH,TEMP
		LDD TEMP,Y+C5_AVREMUL_R16
		POP_Y
		;выполняю процедуру с учетом SREG
		STS SREG,FLAGS
		ICALL
		LDS FLAGS,SREG
		;записываю результат
		PUSH_Y
		ADD YL,TEMP_H
		CLR TEMP
		ADC YH,TEMP
		STD Y+C5_AVREMUL_R16,ACCUM
		POP_Y
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L1:
	;1111 0000b,0x30-0x70
	MOV TEMP,TEMP_H
	ANDI TEMP,11110000b
	CPI TEMP,0x30
	BRCS AVREMUL_EXEC_UNSUPPORTED
	CPI TEMP,0x70+0x01
	BRCC AVREMUL_EXEC_L2
		;SBCI/SUBI,ORI,ANDI
		;Получаю адрес процедуры
		LDI ZH,high(AVREMUL_TABLE2)
		LDI ZL,low(AVREMUL_TABLE2)
		MOV TEMP,TEMP_H
		SWAP TEMP
		SUBI TEMP,0x03
		ROL TEMP															;?
		ADD ZL,TEMP
		CLR TEMP
		ADC ZH,TEMP
		;Получаю номера регистра и константу
		MOV TEMP,TEMP_L
		SWAP TEMP_L
		ANDI TEMP_L,0x0f
		ANDI TEMP,0x0f
		ANDI TEMP_H,0xf0
		SWAP TEMP_H
		OR TEMP,TEMP_H
		;получаю адрес регистра
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP_H
		ADC YH,TEMP_H
		LDD ACCUM,Y+C5_AVREMUL_R16
		POP_Y
		;выполняю процедуру с учетом SREG
		STS SREG,FLAGS
		ICALL
		LDS FLAGS,SREG
		;записываю результат
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP
		ADC YH,TEMP
		STD Y+C5_AVREMUL_R16,ACCUM
		POP_Y
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L2:
	;1101 0000b,0xd0
	;10k0 kksd dddd ykkk
	MOV TEMP,TEMP_H
	ANDI TEMP,11010000b
	CPI TEMP,0xd0
	BRCS AVREMUL_EXEC_UNSUPPORTED
	BRNE AVREMUL_EXEC_L3
		;LDD/STD (Z+k,Y+k)
		;Получаю номер регистра и константу
		MOV TEMP,TEMP_L
		SWAP TEMP_L
		ANDI TEMP_L,0x0f
		ANDI TEMP,0x07
		PUSH TEMP
		ANDI TEMP_H,0x0c
		LSL TEMP_H
		LSL TEMP_H
		OR TEMP,TEMP_H
		POP TEMP_H
		BLD TEMP_H,0x01
		ANDI TEMP_H,0x20
		LSL TEMP_H
		OR TEMP,TEMP_H
		;получаю адрес регистра
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP_H
		ADC YH,TEMP_H
		LDD ACCUM,Y+C5_AVREMUL_R16
		POP_Y
		LDD ZH,Y+C5_AVREMUL_RZH+0x01
		LDD ZL,Y+C5_AVREMUL_RZH+0x00
		ADD ZL,TEMP
		CLR TEMP
		ADC ZH,TEMP
		;Читаю или записываю согласно биту s
		BRTS PC+0x02
		LD ACCUM,Z
		BRTC PC+0x02
		ST Z,ACCUM
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP
		ADC YH,TEMP
		STD Y+C5_AVREMUL_R16,ACCUM
		POP_Y
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L3:
;	;1111 1100b,0x90-0x94
;	MOV TEMP,TEMP_H
;	ANDI TEMP,11111100b
;	CPI TEMP,0x90
;	BRCS AVREMUL_EXEC_UNSUPPORTED
;	BRNE AVREMUL_EXEC_L4
;		LDI ZH,high(AVREMUL_TABLE3)
;		LDI ZL,low(AVREMUL_TABLE3)
;		ANDI TEMP_L,0x0f
;		SWAP TEMP_H
;		LSR TEMP_H														;?
;		LSR TEMP_H														;?
;		ANDI TEMP_H,0x10
;		OR TEMP_L,TEMP_H
;		CLR TEMP
;		ADD ZL,TEMP_L
;		ADC ZH,TEMP
;		MCALL EVREMUL_R_IN
;		MCALL EVREMUL_R_OUT
;		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L4:
	;1001 010d dddd cccc
	MOV TEMP,TEMP_H
	ANDI TEMP,11111110b
	CPI TEMP,0x94
	BRCS AVREMUL_EXEC_UNSUPPORTED
	BRNE AVREMUL_EXEC_L5
		;COM,NEG,SWAP,INC,ASR,LSR,ROR,SEx/CLx
		;Получаю адрес процедуры
		LDI ZH,high(AVREMUL_TABLE4)
		LDI ZL,low(AVREMUL_TABLE4)
		MOV TEMP,TEMP_L
		ANDI TEMP,0x07
		ROL TEMP															;?
		ADD ZL,TEMP
		CLR TEMP
		ADC ZH,TEMP
		;Получаю номер регистра
		SWAP TEMP_L
		ANDI TEMP_L,0x0f
		;получаю адрес регистра
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP_H
		ADC YH,TEMP_H
		LDD ACCUM,Y+C5_AVREMUL_R16
		POP_Y
		;выполняю процедуру с учетом SREG
		STS SREG,FLAGS
		ICALL
		LDS FLAGS,SREG
		;записываю результат
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP
		ADC YH,TEMP
		STD Y+C5_AVREMUL_R16,ACCUM
		POP_Y
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L5:
	;TODO ...

AVREMUL_EXEC_L6:
	;1110 kkkk dddd kkkk
	MOV TEMP,TEMP_H
	ANDI TEMP,11110000b
	CPI TEMP,0xe0
	BRCS AVREMUL_EXEC_UNSUPPORTED
	BRNE AVREMUL_EXEC_L7
		;LDI
		;Получаю номер регистра и константу
		SWAP TEMP_L
		MOV TEMP,TEMP_L
		ANDI TEMP_L,0x0f
		ANDI TEMP_H,0x0f
		ANDI TEMP,0xf0
		OR TEMP,TEMP_H
		SWAP TEMP
		;получаю адрес регистра и записываю
		PUSH_Y
		ADD YL,TEMP_L
		CLR TEMP_H
		ADC YH,TEMP_H
		ST Y+C5_AVREMUL_R16,TEMP
		POP_Y
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L7:
	;1111 0Boo oooo obbb
	MOV TEMP,TEMP_H
	ANDI TEMP,11111000b
	CPI TEMP,0xf0
	BRCS AVREMUL_EXEC_UNSUPPORTED
	BRNE AVREMUL_EXEC_L7
		;BREQ,BRNE,BRCS,BRCC...
		;Получаю номер регистра
		MOV TEMP,TEMP_L
		ANDI TEMP,0x07
		MCALL BITNUM_TO_NUM
		BLD TEMP_H,0x02
		LSR TEMP_H
		ROR TEMP_L
		LSR TEMP_H
		ROR TEMP_L
		ANDI TEMP_L,0xfe
		CLR TEMP_H
		MOV ACCUM,FLAGS
		AND ACCUM,TEMP
		BREQ AVREMUL_EXEC_L7_LO
		BRTS PC+0x02
		ADD XL,TEMP
		ADC XH,TEMP_H
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L7_LO:
		BRTC PC+0x02
		ADD XL,TEMP
		ADC XH,TEMP_H
		RJMP AVREMUL_EXEC
AVREMUL_EXEC_L8:
