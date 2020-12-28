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
	.EQU	C5_AVREMUL_R26							= 0x0a			;
	.EQU	C5_AVREMUL_R27							= 0x0b			;
	.EQU	C5_AVREMUL_R28							= 0x0c			;
	.EQU	C5_AVREMUL_R29							= 0x0d			;
	.EQU	C5_AVREMUL_R30							= 0x0e			;
	.EQU	C5_AVREMUL_R31							= 0x0f			;
	.EQU	C5_AVREMUL_SREG						= 0x10			;
	.EQU	C5_AVREMUL_CP							= 0x11			;

	.EQU	_C5_PROC_STATE_GHOST					= 0x01			;Инициализируется или деактивация
	.EQU	_C5_PROC_STATE_BUSY					= 0x02			;Выполняется


;FLAGS - sreg
;X - CP


EVREMUL_RR_IN:
	MOV TEMP_EH,TEMP_L
	SWAP TEMP_EH
	ANDI TEMP_EH,0x0f
	MOV TEMP_EL,TEMP_L
	ANDI TEMP_EL,0x0f
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_EH
	ADC YH,TEMP
	LDD TEMP_H,Y+C5_AVREMUL_R16
	POP_Y
	PUSH_Y
	ADD YL,TEMP_EL
	ADC YH,TEMP
	LDD TEMP_L,Y+C5_AVREMUL_R16
	POP_Y
	STS SREG,FLAGS
	RET

EVREMUL_RC_IN:
	MOV TEMP_EH,TEMP_L
	SWAP TEMP_EH
	ANDI TEMP_EH,0x0f
	ANDI TEMP_L,0x0f
	ANDI TEMP_H,0xf0
	ORI TEMP_L,TEMP_H
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_EH
	ADC YH,TEMP
	LDD TEMP_H,Y+C5_AVREMUL_R16
	POP_Y
	STS SREG,FLAGS
	RET

EVREMUL_R_IN:
	SWAP TEMP_L
	ANDI TEMP_L,0x0f
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_L
	ADC YH,TEMP
	LDD TEMP_L,Y+C5_AVREMUL_R16
	POP_Y
	STS SREG,FLAGS
	RET

EVREMUL_R_OUT:
	LDS FLAGS,SREG
	MOV TEMP_L,TEMP_H
	POP TEMP_H
	SWAP TEMP_H
	ANDI TEMP_H,0x0f
	CLR TEMP
	PUSH_Y
	ADD YL,TEMP_H
	ADC ZH,TEMP
	STD Y+C5_AVREMUL_R16,TEMP_L
	POP_Y
	RET

EVREMUL_CPSE:
	PUSH FLAGS
	CP TEMP_H,TEMP_L
	BRNE PC+0x02
	ADIW X,0x02
	POP FLAGS
	RET

EVREMUL_TABLE1:
	CPC TEMP_H,TEMP_L													;0000 01rd dddd rrrr
	RET
	SBC TEMP_H,TEMP_L													;0000 10rd dddd rrrr
	RET
	ADD TEMP_H,TEMP_L													;0000 11rd dddd rrrr
	RET
	RJMP EVREMUL_CPSE														;0001 00rd dddd rrrr
	NOP
	CP TEMP_H,TEMP_L													;0001 01rd dddd rrrr
	RET
	SUB TEMP_H,TEMP_L													;0001 10rd dddd rrrr
	RET
	ADC TEMP_H,TEMP_L													;0001 11rd dddd rrrr
	RET
	AND TEMP_H,TEMP_L													;0010 00rd dddd rrrr
	RET
	EOR TEMP_H,TEMP_L													;0010 01rd dddd rrrr
	RET
	OR TEMP_H,TEMP_L													;0010 10rd dddd rrrr
	RET
	MOV TEMP_H,TEMP_L													;0010 11rd dddd rrrr
	RET
	CP TEMP_H,TEMP_L													;0011 KKKK dddd KKKK (CPI)
	RET
	SBC TEMP_H,TEMP_L													;0100 KKKK dddd KKKK (SBCI)
	RET
	SUB TEMP_H,TEMP_L													;0101 KKKK dddd KKKK (SUBI)
	RET
	OR TEMP_H,TEMP_L													;0110 KKKK dddd KKKK (ORI)
	RET
	AND TEMP_H,TEMP_L													;0111 KKKK dddd KKKK (ANDI)
	RET
EVREMUL_TABLE2:
	COM TEMP_L															;1001 010d dddd 0000
	RET
	NEG TEMP_L															;1001 010d dddd 0001
	RET
	SWAP TEMP_L															;1001 010d dddd 0010
	RET
	INC TEMP_L															;1001 010d dddd 0011
	RET
	NOP																	;1001 010d dddd 0100	(reserved)
	RET
	ASR TEMP_L															;1001 010d dddd 0101
	RET
	LSR TEMP_L															;1001 010d dddd 0110
	RET
	ROR TEMP_L															;1001 010d dddd 0111
	RET

AVREMUL_EXECUTOR1:													;0000 01xx - 0111 xxxx
	LDI ZH,high(EVREMUL_TABLE1)
	LDI ZL,low(EVREMUL_TABLE1)
	ANDI TEMP_H,0x7c
	ROL TEMP_H
	CLR TEMP
	ADD ZL,TEMP_H
	ADC ZH,TEMP
	SBRC TEMP_H,0x04
	RJMP AVREMUL_EXECUTOR1_CONSTANT
	MCALL EVREMUL_RR_IN
	RJMP AVREMUL_EXECUTOR1_CONSTANT_NEXT
AVREMUL_EXECUTOR1_CONSTANT:
	MCALL EVREMUL_RC_IN
AVREMUL_EXECUTOR1_CONSTANT_NEXT:
	ICALL
	MJMP EVREMUL_R_OUT


AVREMUL_EXECUTOR2:													;1001 010x xxxx 0xxx
	LDI ZH,high(EVREMUL_TABLE2)
	LDI ZL,low(EVREMUL_TABLE2)
	ANDI TEMP_L,0x07
	ROL TEMP_H
	CLR TEMP
	ADD ZL,TEMP_H
	ADC ZH,TEMP
	MCALL EVREMUL_R_IN
	ICALL
	MJMP EVREMUL_R_OUT


AVREMUL_EXECUTOR3:													;1001 0100 xxxx 1000
	MCALL EVREMUL_R_IN
	MOV TEMP,TEMP_L													;1001 0100 Bbbb 1000 (set/clear sreg bit)
	ANDI TEMP,0x07
	MCALL BITNUM_TO_NUM
	PUSH TEMP
	COM TEMP
	ANDI FLAGS,TEMP
	POP TEMP
	SBRS TEMP_L,0x07
	ORI FLAGS,TEMP
	MJMP EVREMUL_R_OUT
