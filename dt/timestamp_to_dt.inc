;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.12.2021	w5277c@gmail.com			Начало
;01.01.2022	w5277c@gmail.com			Черновой
;-----------------------------------------------------------------------------------------------------------------------
;На основе http://git.musl-libc.org/cgit/musl/tree/src/time/__secs_to_tm.c?h=v0.9.15
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_TIMESTAMP_TO_DT
.else
.set DEF_TIMESTAMP_TO_DT = 1

.if REPORT_INCLUDES == 0x01
.message "included TIMESTAMP_TO_DT"
.endif

.include	"./math/div32x16.inc"

	.EQU	TIMESTAMP_TO_DT_01032000			=	951868800	;01 March 2000
	.EQU	TIMESTAMP_TO_DT_4Y					=	365*4 + 1	;4 years
	.EQU	TIMESTAMP_TO_DT_1Y					=	365			;1 year

_TIMESTAMP_TO_DT__DAYS_IN_MONTH:
	.db	31,30,31,30,31,31,30,31,30,31,31,29,00
;--------------------------------------------------------
TIMESTAMP_TO_DT:
;--------------------------------------------------------
;Конвертация unix timestamp в дату и время
;IN: TEMP_EH/EL/H,L-unix timestamp
;OUT: TEMP_EH-год(0=2000), TEMP_EL-месяц(1-12),
;TEMP-день месяца(1-31, 0x80-ошика), TEMP_H-часы(0-23),
;TEMP_L-минуты(0-59), XH-секунды(0-59),
;XL-день недели(1-7)
;--------------------------------------------------------
	PUSH_Z

	SUBI TEMP_L,(TIMESTAMP_TO_DT_01032000>>0x00 & 0xff)
	SBCI TEMP_H,(TIMESTAMP_TO_DT_01032000>>0x08 & 0xff)
	SBCI TEMP_EL,(TIMESTAMP_TO_DT_01032000>>0x10 & 0xff)
	SBCI TEMP_EH,(TIMESTAMP_TO_DT_01032000>>0x18 & 0xff)
	BRCS _TIMESTAMP_TO_DT__ERROR

	LDI_X 60
	MCALL DIV32X16
	PUSH XL																;sec
	LDI_X 60
	MCALL DIV32X16
	PUSH XL																;min
	LDI_X 24
	MCALL DIV32X16
	PUSH XL																;hour

	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP,0x03
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	LDI TEMP,7
	MCALL DIV16X8
	POP TEMP_L
	POP TEMP_H
	PUSH TEMP															;day of week

	PUSH TEMP_H
	PUSH TEMP_L
	CLR TEMP_EH
	CLR TEMP_EL
	LDI_X TIMESTAMP_TO_DT_4Y
	MCALL DIV32X16
	MOV ZH,TEMP_L
	CPI ZH,25
	BRNE PC+0x02
	DEC ZH
	MOV TEMP,ZH
	LDI TEMP_H,high(TIMESTAMP_TO_DT_4Y)
	LDI TEMP_L,low(TIMESTAMP_TO_DT_4Y)
	MCALL MUL16X8
	MOV XL,TEMP_L
	MOV XH,TEMP_H
	POP TEMP_L
	POP TEMP_H
	SUBI TEMP_L,XL
	SBCI TEMP_H,XH

	PUSH TEMP_H
	PUSH TEMP_L
	LDI_X TIMESTAMP_TO_DT_1Y
	MCALL DIV32X16
	MOV ZL,TEMP_L
	CPI ZL,4
	BRNE PC+0x02
	DEC ZL
	MOV TEMP,ZL
	LDI TEMP_H,high(TIMESTAMP_TO_DT_1Y)
	LDI TEMP_L,low(TIMESTAMP_TO_DT_1Y)
	MCALL MUL16X8
	MOV XLTEMP_L
	MOV XH,TEMP_H
	POP TEMP_L
	POP TEMP_H
	SUBI TEMP_L,XL
	SBCI TEMP_H,XH

	LSL ZH
	LSL ZH
	ADD ZH,ZL
	MOV TEMP_EH,ZH														;year

	LDI TEMP_EL,0x01
	LDI_Z _TIMESTAMP_TO_DT__DAYS_IN_MONTH
_TIMESTAMP_TO_DT__LOOP:
	LPM TEMP,Z+
	CPI TEMP_H,0x00
	BRNE PC+0x03
	CP TEMP_L,TEMP
	BRCS _TIMESTAMP_TO_DT__LOOP_END
	SUBI TEMP_L,TEMP
	SBCI TEMP_H,C0x00
	INC TEMP_EL															;month
	RJMP _TIMESTAMP_TO_DT__LOOP
_TIMESTAMP_TO_DT__LOOP_END:

	MOV TEMP,TEMP_L													;day of month
	POP XL																;day of week
	POP TEMP_H															;hour
	POP TEMP_L															;min
	POP XH																;SEC
	RJMP _TIMESTAMP_TO_DT__END

_TIMESTAMP_TO_DT__ERROR:
	LDI TEMP,0x80
_TIMESTAMP_TO_DT__END:
	POP_Z
	RET
.endif
