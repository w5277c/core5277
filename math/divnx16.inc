;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.10.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DIVNX16
.else
.set DEF_DIVNX16 = 1

.if REPORT_INCLUDES == 0x01
.message "included DIVNx16"
.endif


;--------------------------------------------------------
DIVNX16:
;--------------------------------------------------------
;Деление n-байт делимого на 16бит делитель
;IN: Y-адрес на память делимого и результата,
;X-делитель, ACCUM-размер делимого в байтах(0<x<32)
;--------------------------------------------------------
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH FLAGS
	
	;Умножаем на 8(бит) + 1
	MOV LOOP_CNTR,ACCUM
	LSL LOOP_CNTR
	LSL LOOP_CNTR
	LSL LOOP_CNTR
	INC LOOP_CNTR

	SUB ZH,ZH
	CLR ZL
DIVNX16__LOOP:
	;DIVIDEND_ROR
	LDS FLAGS,SREG
	PUSH ACCUM
	ADD YL,ACCUM
	ADC YH,C0x00
	STS SREG,FLAGS
DIVNX16__DIVIDEND_ROR_LOOP:
	LD TEMP,-Y
	ROR TEMP
	ST Y,TEMP
	DEC ACCUM
	BRNE DIVNX16__DIVIDEND_ROR_LOOP
	POP ACCUM
	DEC LOOP_CNTR
	BREQ DIVNX16__END
	;TEMP ROR
	ROR TEMP_L
	ROR TEMP_H
	ROR TEMP_EL
	SUB TEMP_L,XL
	SBC TEMP_H,XH
	SBC TEMP_EL,C0x00
	BRCS PC+0x03
	SEC
	RJMP DIVNX16__LOOP
	;ZERO BIT
	ADD TEMP_L,XL
	ADC TEMP_H,XH
	ADC TEMP_EL,C0x00
	CLC
	RJMP DIVNX16__LOOP
DIVNX16__END:

	POP FLAGS
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	RET
.endif
