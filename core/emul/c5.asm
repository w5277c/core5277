;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.05.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Первые наброски эмулятора AVR(для core5277), необходим для выполнения программы из RAM
;Быстрого выполнения не требуется, так как все вызовы подпрограмм будут реальные.
;Эмулятор не использует память, программа сама выделяет для себя память.
;16 регистров хранятся в стеке, стек общий для эмулятора и программы
;Часть кодов не актуально, как например SPM, RETI, SEI, MULL, SLEEP, BREAK, WDR, IN, OUT и прочее
;-----------------------------------------------------------------------------------------------------------------------
;TODO Исполльзвание стека по минимуму, учитывать что стек программы и эмулятора общий
;TODO После реализации основной логики провести оптимизацию.

.ifdef DEF_EMUL_C5
.else
.set DEF_EMUL_C5 = 1

.if REPORT_INCLUDES == 0x01
.message "included C5 emulator v0.1"
.endif

;.include	"./mem/ram_copy16.inc"

;---CONSTANTS--------------------------------------------
C5_CODE_HANDLERS:
	.dw	_EMUL_C5_H0x,_EMUL_C5_H1x,_EMUL_C5_H2x,_EMUL_C5_H3x,_EMUL_C5_H4x,_EMUL_C5_H5x,_EMUL_C5_H6x,_EMUL_C5_H7x
	.dw	_EMUL_C5_H8x,_EMUL_C5_H9x,_EMUL_C5_Hax,_EMUL_C5_Hbx,_EMUL_C5_Hcx,_EMUL_C5_Hdx,_EMUL_C5_Hex,_EMUL_C5_Hfx


;--------------------------------------------------------
EMUL_C5_INIT:
;--------------------------------------------------------
;Инициализация эмулятора
;IN:Y-адрес на программу
;--------------------------------------------------------
	LDI TEMP,0x00
	LDI LOOP_CNTR,0x10
EMUL_C5_INIT__LOOP:
	PUSH TEMP
	DEC LOOP_CNTR
	BRNE EMUL_C5_INIT__LOOP
	LDS ZL,SPL
	LDS ZH,SPH
	ADIW ZL,0x01

;--------------------------------------------------------
;Основной код
;Z-адрес 16 регистров, Y-адрес программы, FLAGS-SREG
;--------------------------------------------------------
EMUL_C5__MAIN_LOOP:
	LD ACCUM,Y+
	MOV TEMP,ACCUM
	ANDI ACCUM,0x0f
	SWAP TEMP
	ANDI TEMP,0x0f
	;TODO необходимо проеверить адресацию и порядок записи в стек
	LDI TEMP_H,high(C5_CODE_HANDLERS*2)

	LDI TEMP_L,low(C5_CODE_HANDLERS*2)
	ADD TEMP_L,TEMP
	CLR TEMP
	ADC TEMP_H,TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	RET

;--------------------------------------------------------
_EMUL_C5_H0x:
	;0x00-NOP
	CPI ACCUM,0x00
	BRNE _EMUL_C5_H0x__NOT_NOP
	LD TEMP,Y+
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_NOP:
	;0x01-MOVW
	CPI ACCUM,0x01
	BRNE _EMUL_C5_H0x__NOT_MOVW
		LD ACCUM,Y+
		SBRS ACCUM,0x03													;Если 3й бит в октете выключен, то работа с source регистрами, т.е. сбой
		RJMP _EMUL_C5_FAULT
		SBRS ACCUM,0x07													;Если 3й бит в октете выключен, то работа с source регистрами, т.е. сбой
		RJMP _EMUL_C5_FAULT
		MOV TEMP,ACCUM
		ANDI TEMP,0x07
		LSL TEMP
		MCALL _EMUL_C5_GET_REG16
		MOV TEMP,ACCUM
		SWAP TEMP
		ANDI TEMP,0x07
		LSL TEMP
		MCALL _EMUL_C5_SET_REG16
		RET
_EMUL_C5_H0x__NOT_MOVW:
	;0x04-CPC
	CPI ACCUM,0x07
	BRNE _EMUL_C5_H0x__NOT_CPC
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		CPC TEMP_L,TEMP_H
		LDS FLAGS,SREG
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_CPC:
	;0x0b-SBC
	CPI ACCUM,0x0b
	BRNE _EMUL_C5_H0x__NOT_SBC
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		SBC TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_SBC:
	;0x0f-ADD
	CPI ACCUM,0x0f
	BRNE _EMUL_C5_H0x__NOT_ADD
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		ADD TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_ADD:
	RJMP _EMUL_C5_FAULT

;--------------------------------------------------------
_EMUL_C5_H1x:
	;0x13-CPSE
	CPI ACCUM,0x13
	BRNE _EMUL_C5_H0x__NOT_CPSE
		MCALL _EMUL_C5_GET_RD
		CP TEMP_L,TEMP_H
		BRNE PC+0x03
		MCALL _EMUL_C5_SKIP
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_CPSE:
	;000101rd-CP
	CPI ACCUM,0x17
	BRNE _EMUL_C5_H0x__NOT_CP
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		CP TEMP_L,TEMP_H
		LDS FLAGS,SREG
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_CP:
	;000110rd-SUB
	CPI ACCUM,0x1B
	BRNE _EMUL_C5_H0x__NOT_SUB
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		SUB TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_SUB:
	;000111rd-ADC
	CPI ACCUM,0x1f
	BRNE _EMUL_C5_H0x__NOT_ADC
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		ADC TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_ADC:
	RJMP _EMUL_C5_FAULT

;--------------------------------------------------------
_EMUL_C5_H2x:
	;001000rd-AND
	CPI ACCUM,0x23
	BRNE _EMUL_C5_H0x__NOT_AND
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		AND TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_AND:
	;001001rd-EOR
	CPI ACCUM,0x27
	BRNE _EMUL_C5_H0x__NOT_EOR
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		EOR TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_EOR:
	;001010rd-EOR
	CPI ACCUM,0x2b
	BRNE _EMUL_C5_H0x__NOT_OR
		MCALL _EMUL_C5_GET_RD
		STS SREG,FLAGS
		OR TEMP_L,TEMP_H
		LDS FLAGS,SREG
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_OR:
	;001011rd-MOV
	CPI ACCUM,0x2f
	BRNE _EMUL_C5_H0x__NOT_MOV
		MCALL _EMUL_C5_GET_RD
		MOV TEMP_L,TEMP_H
		MCALL _EMUL_C5_SET_REG8
		RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H0x__NOT_MOV:
	RJMP _EMUL_C5_FAULT

;--------------------------------------------------------
_EMUL_C5_H3x:
	;0011-CPI
	MCALL _EMUL_C5_GET_KD
	STS SREG,FLAGS
	CP TEMP_L,TEMP_H
	LDS FLAGS,SREG
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_H4x:
	;0100-SBCI
	MCALL _EMUL_C5_GET_KD
	STS SREG,FLAGS
	SBC TEMP_L,TEMP_H
	LDS FLAGS,SREG
	MCALL _EMUL_C5_SET_REG8
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_H5x:
	;0101-SUBI
	MCALL _EMUL_C5_GET_KD
	STS SREG,FLAGS
	SUB TEMP_L,TEMP_H
	LDS FLAGS,SREG
	MCALL _EMUL_C5_SET_REG8
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_H6x:
	;0110-ORI
	MCALL _EMUL_C5_GET_KD
	STS SREG,FLAGS
	OR TEMP_L,TEMP_H
	LDS FLAGS,SREG
	MCALL _EMUL_C5_SET_REG8
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_H7x:
	;0111-ANDI
	MCALL _EMUL_C5_GET_KD
	STS SREG,FLAGS
	AND TEMP_L,TEMP_H
	LDS FLAGS,SREG
	MCALL _EMUL_C5_SET_REG8
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_H8x:
	LDI TEMP_EL,0x00
	RJMP PC+0x02
;--------------------------------------------------------
_EMUL_C5_Hax:
	LDI TEMP_EL,0x20
	;10q0 qqSd dddd Yqqq (Y=Y/Z,q-offset,d-direction(S=ST/LD))
	SBRS ACCUM,0x00
	RJMP _EMUL_C5_FAULT
	SBRC ACCUM,0x03
	ORI TEMP_EL,10
	SBRC ACCUM,0x02
	ORI TEMP_EL,0x08

	LD TEMP,Y+
	MOV TEMP_EH,TEMP
	ANDI TEMP_EH,0x07
	OR TEMP_EL,TEMP_EH
	MOVW XL,ZL
	ADIW XL,0x0c
	SBRS TEMP,0x03
	ADIW XL,0x02
	SWAP TEMP
	ANDI TEMP,0x0f

	LDD ACCUM,X+
	LDD XH,X
	MOV XL,ACCUM
	ADD XL,TEMP_EL
	CLR TEMP_EL
	ADC XH,TEMP_EL

	SBRC ACCUM,0x01
	RJMP _EMUL_C5_H8x__ST
	LD TEMP_L,X
	MCALL _EMUL_C5_SET_REG8
	RJMP _EMUL_C5_H8x__DONE
_EMUL_C5_H8x__ST:
	MCALL _EMUL_C5_GET_REG8
	ST X,TEMP_L
_EMUL_C5_H8x__DONE:
	RJMP EMUL_C5__MAIN_LOOP


;TODO использовать X вместо PUSH_Z/POP_Z
;--------------------------------------------------------
_EMUL_C5_H9x:
	MOV TEMP,ACCUM
	ANDI TEMP,0x0c
	CPI TEMP,0x00
	BRNE _EMUL_C5_H9x__NOT_00

	;(O:00=NO OP,01=+,10=-,Y:11=X,10=Y,00=Z,S=ST/LD)
	;1001 00Sr rrrr YYOO
	;1001 001r rrrr 1100 st x,rr
	;1001 001r rrrr 1101 st x+,rr
	;1001 001r rrrr 1110 st -x,rr
	;1001 001r rrrr 1001 st y+,rr
	;1001 001r rrrr 1010 st -y,rr
	;1001 001r rrrr 0001 st z+,rr
	;1001 001r rrrr 0010 st -z,rr
	;ld -//-
	;1001 001d dddd 1111 push
	;1001 001d dddd 0000 sts k,rr + word
	;1001 000d dddd 0000	lds rd,k
	;1001 000d dddd 1111 pop

	;NO SUPPORT
	;1001 001r rrrr 0110 lac z,rd
	;1001 001r rrrr 0101 las z,rd
	;1001 001r rrrr 0111 lat z,rd
	;1001 001r rrrr 0100 xch z,rd
	;1001 000d dddd 0110 elpm rd,z
	;1001 000d dddd 0111 elpm rd,z+
	;1001 000d dddd 0100 lpm rd,z
	;1001 000d dddd 0101 lpm rd,z+
	;1001 0101 1101 1000 elpm

	SBRS ACCUM,0x00
	RJMP _EMUL_C5_FAULT

	LD TEMP,Y+
	MOV TEMP_L,TEMP
	ANDI TEMP_L,0x0f
	BREQ _EMUL_C5_H9x__STSLDS
	CPI TEMP_L,0x0f
	BREQ _EMUL_C5_H9x__POPPUSH
	ANDI TEMP_L,0x0c
	CPI TEMP_L,0x04
	BREQ _EMUL_C5_FAULT
	CPI TEMP_L,0x08
	BREQ _EMUL_C5_FAULT
	MOV TEMP_L,TEMP
	ANDI TEMP_L,0x0f

	PUSH_Y
	MOVW YL,ZL
	ADIW YL,0x0e
	SBRS TEMP,0x03
	RJMP _EMUL_C5_H9x_Z
	SBIW YL,0x02
	SBRC TEMP,0x02
	SBIW YL,0x02
_EMUL_C5_H9x_Z:

	SWAP TEMP
	ANDI TEMP,0x0f

	PUSH_Y
	LDD ACCUM,Y+0x01
	LDD YL,Y+0x00
	MOV YH,ACCUM
	ADD YL,ZL
	ADC YH,ZH
	ADIW YL,_EMUL_C5_PROG_RAM

	SBRC ACCUM,0x01
	RJMP _EMUL_C5_H9x__ST
	SBRC TEMP_L,0x03
	SBIW YL,0x01
	LD TEMP_L,Y
	SBRC TEMP_L,0x02
	ADIW YL,0x01
	MCALL _EMUL_C5_SET_REG8
	RJMP _EMUL_C5_H8x__DONE
_EMUL_C5_H9x__ST:
	RCALL _EMUL_C5_DET_REG8
	ST Y,TEMP_L
_EMUL_C5_H9x__DONE:
	SUB YL,ZL
	SBC YH,ZH
	SBIW YL,_EMUL_C5_PROG_RAM
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	POP_Y
	ST Y+0x01,TEMP_H
	ST Y+0x00,TEMP_L
	POP_Y
	RJMP EMUL_C5__MAIN_LOOP

_EMUL_C5_H9x__POPPUSH:
	SWAP TEMP
	ANDI TEMP,0x0f

	MOVW XL,ZL
	ADD XL,TEMP
	CLR TEMP
	ADC XH,TEMP

	SBRS ACCUM,0x01
	RJMP _EMUL_C5_H9x__POP
_EMUL_C5_H9x__PUSH:
	LD TEMP_L,X
	PUSH TEMP_L
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_H9x__POP:
	POP TEMP_L
	ST X,TEMP_L
_EMUL_C5_H9x__POPPUSH_DONE:
	RJMP EMUL_C5__MAIN_LOOP

_EMUL_C5_H9x__STSLDS:
	SWAP TEMP
	ANDI TEMP,0x0f
	SBRS ACCUM,0x01
	RJMP _EMUL_C5_H9x__LDS
_EMUL_C5_H9x__STS:
	PUSH_Z
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LD TEMP_L,Z
	POP_Z
	PUSH_Y
	LD ACCUM,Y+
	LD YH,Y+
	MOV YL,ACCUM
	ADD YL,ZL
	ADC YH,ZH
	ADIW YL,_EMUL_C5_PROG_RAM
	ST Y,TEMP_L
	RJMP _EMUL_C5_H9x__STSLDS_DONE
_EMUL_C5_H9x__LDS:
	PUSH_Y
	LD ACCUM,Y+
	LD YH,Y+
	MOV YL,ACCUM
	ADD YL,ZL
	ADC YH,ZH
	ADIW YL,_EMUL_C5_PROG_RAM
	LD TEMP_L,Y
	POP_Y
	PUSH_Z
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	ST Z,TEMP_L
_EMUL_C5_H9x__STSLDS_DONE:
	RJMP EMUL_C5__MAIN_LOOP

_EMUL_C5_H9x__NOT_00:
	CPI TEMP,0x04
	BRNE _EMUL_C5_H9x__NOT_04


_EMUL_C5_H9x__NOT_04:
	CPI TEMP,0x08
	BRNE _EMUL_C5_H9x__NOT_08


_EMUL_C5_H9x__NOT_08:
	RJMP _EMUL_C5_FAULT

;--------------------------------------------------------
_EMUL_C5_Hbx:
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_Hcx:
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_Hdx:
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_Hex:
	RJMP EMUL_C5__MAIN_LOOP
_EMUL_C5_Hfx:
	RJMP EMUL_C5__MAIN_LOOP

;--------------------------------------------------------
_EMUL_C5_GET_REG16:
;--------------------------------------------------------
;Считываем слово из регистров
;IN:Z-адрес переменных, TEMP-номер регисра 0-15=15-31
;OUT: TEMP_H/L-значение
;--------------------------------------------------------
	PUSH_Z
	ADD ZL,TEMP
	CLR TEMP_H
	ADC ZH,TEMP_H
	LD TEMP_L,Z+
	LD TEMP_H,Z
	POP_Z
	RET
;--------------------------------------------------------
_EMUL_C5_SET_REG16:
;--------------------------------------------------------
;Записываем слово в регистры
;IN:Z-адрес переменных, TEMP-номер регисра 0-15=15-31
;TEMP_H/L-значение
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	ST Z+,TEMP_L
	ST Z,TEMP_H
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_EMUL_C5_GET_REG8:
;--------------------------------------------------------
;Считываем байт из регистра
;IN:Z-адрес переменных, TEMP-номер регисра 0-15=15-31
;OUT: TEMP_L-значение
;--------------------------------------------------------
	PUSH_Z
	ADD ZL,TEMP
	CLR TEMP_L
	ADC ZH,TEMP_L
	LD TEMP_L,Z
	POP_Z
	RET
;--------------------------------------------------------
_EMUL_C5_SET_REG8:
;--------------------------------------------------------
;Записываем байт в регистр
;IN:Z-адрес переменных, TEMP-номер регисра 0-15=15-31
;TEMP_L-значение
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	ST Z,TEMP_L
	POP TEMP
	POP_Z
	RET



;--------------------------------------------------------
_EMUL_C5_POP8:
;--------------------------------------------------------
;Считываем байт из стека
;IN:Z-адрес переменных,TRY_CNTR-смещение стека
;OUT:TEMP_L-значение
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	ADD ZL,TRY_CNTR
	CLR TEMP
	ADC ZH,TEMP
	LD TEMP_L,Z
	INC TRY_CNTR
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_EMUL_C5_PUSH8:
;--------------------------------------------------------
;Записываем байт в стек
;IN:Z-адрес переменных,TRY_CNTR-смещение стека
;TEMP_L-значение
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	ADD ZL,TRY_CNTR
	CLR TEMP
	ADC ZH,TEMP
	ST Z,TEMP_L
	DEC TRY_CNTR
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_EMUL_C5_GET_KD:
;--------------------------------------------------------
;Считываем 8b константу и байт из регистра
;IN:Z-адрес переменных,ACCUM-старший нибл константы в
;младшем нибле ACCUM
;OUT: TEMP_H-константа,TEMP_L-знач.D,TEMP-D
;--------------------------------------------------------
	SWAP ACCUM
	ANDI ACCUM,0xf0
	LD TEMP_H,Y+
	MOV TEMP,TEMP_H
	ANDI TEMP_H,0x0f
	OR TEMP_H,ACCUM
	SWAP TEMP
	ANDI TEMP,0x0f
	MCALL _EMUL_C5_GET_REG8
	RET

;--------------------------------------------------------
_EMUL_C5_GET_RD:
;--------------------------------------------------------
;Считываем слово из регистров
;IN:Z-адрес переменных
;OUT: TEMP_H-знач.R,TEMP_L-знач.D,TEMP-D
;--------------------------------------------------------
	LD TEMP_H,Y+
	MOV TEMP,ACCUM
	ANDI TEMP,0x07
	MCALL _EMUL_C5_GET_REG8
	MOV TEMP_H,TEMP_L
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	MCALL _EMUL_C5_SET_REG8
	RET

;--------------------------------------------------------
_EMUL_C5_SKIP:
;--------------------------------------------------------
;Считываем байт из регистра
;IN:Z-адрес переменных,Y-адрес программы
;OUT:Y-адрес программы
;--------------------------------------------------------
	LD TEMP_H,Y+
	LD TEMP_L,Y+
	ANDI TEMP_H,0xfc
	CPI TEMP_H,0x90
	BREQ PC+0x02
	RET
	ANDI TEMP_L,0x0f
	BRNE PC+0x02
	ADIW YL,0x02
	RET

.endif
