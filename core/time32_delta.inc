;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало
;16.09.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIME32_DELTA
.else
.set DEF_C5_TIME32_DELTA = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIME32_DELTA"
.endif

.include	"./core/time32_mark.inc"
.include	"./math/mul32x16.inc"
.include	"./math/mul16x8.inc"

;--------------------------------------------------------
C5_TIME32_DELTA:
;--------------------------------------------------------
;Считаем дельту между текущей временной меткой и
;записанной в выделенной паямти (4 байта)
;IN: PID,TEMP-смещение на выделенные 4 байта, хранящие
;временную метку
;OUT: Дельта будет записана поверх временной метки в
;выделенную память
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	MCALL C5_RAM_OFFSET
	ADD YL,TEMP
	ADC YH,C0x00

	LDD TEMP_EH,Y+0x00
	LDD TEMP_EL,Y+0x01
	LDD TEMP_H,Y+0x02
	LDD TEMP_L,Y+0x03

	;Используем TEMP!
	MCALL C5_TIME32_MARK

	LDD TEMP,Y+0x03
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDD TEMP,Y+0x02
	SUB TEMP,TEMP_H
	MOV TEMP_H,TEMP
	LDD TEMP,Y+0x01
	SBC TEMP,TEMP_EL
	MOV TEMP_EL,TEMP
	LDD TEMP,Y+0x00
	SBC TEMP,TEMP_EH
	MOV TEMP_EH,TEMP

	CPI TEMP_EH,0x00
	BRNE PC+0x05
	CPI TEMP_EL,0x00
	BRNE PC+0x03
	CPI TEMP_H,0x06
	BRCS _C5_TIME32_DELTA__BY_CNTR
	MOV TEMP_L,TEMP_H
	MOV TEMP_H,TEMP_EL
	MOV TEMP_EL,TEMP_EH
	CLR TEMP_EH
	LSL TEMP_L
	ROL TEMP_H
	ROL TEMP_EL
	LDI XH,high(1000)
	LDI XL,low(1000)
	MCALL MUL32X16
	RJMP _C5_TIME32_DELTA__STORE
_C5_TIME32_DELTA__BY_CNTR:
	CLR TEMP_H
	LDI TEMP,0x32
	MCALL MUL16X8
	CLR TEMP_EH
_C5_TIME32_DELTA__STORE:
	STD Y+0x00,TEMP_EH
	STD Y+0x01,TEMP_EL
	STD Y+0x02,TEMP_H
	STD Y+0x03,TEMP_L

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Y
	POP_X
	RET
.endif
