;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIME32_MARK
.else
.set DEF_C5_TIME32_MARK = 1

.include	"./core/uptime_copy.inc"
.include	"./math/mul32x16.inc"
.include	"./math/mul16x8.inc"

;--------------------------------------------------------
C5_TIME32_MARK:
;--------------------------------------------------------
;Записываем в выделенную память временную метку (4 байт)
;IN: PID,TEMP-смещение на выделенные 4 байт, для хранения
;временной метки (71s-1us)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	MCALL C5_RAM_OFFSET
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDS TEMP_H,_C5_UPTIME+0x03
	LDS TEMP_L,_C5_UPTIME+0x04
	LDS YH,_C5_MAIN_TIMER_CNTR
	_C5_TIMER_TCNT YL

	POP TEMP
	STS SREG,TEMP

	CPI TEMP_H,0x00
	BRNE PC+0x03
	CPI TEMP_L,0x06
	BRCS _C5_TIME32_MARK__BY_CNTR
	LDI XH,high(1000)
	LDI XL,low(1000)
	MCALL MUL32X16
	RJMP _C5_TIME32_MARK__STORE
_C5_TIME32_MARK__BY_CNTR:
	CPI YH,0x02
	BRCS _C5_TIME32_MARK__BY_TCNT
	MOV TEMP_L,YH
	LDI TEMP,0x32
	MCALL MUL16X8
	RJMP _C5_TIME32_MARK__STORE
_C5_TIME32_MARK__BY_TCNT:
	MOV TEMP_L,YL
	LSR TEMP_L
_C5_TIME32_MARK__STORE:
	STD Z+0x00,TEMP_EH
	STD Z+0x01,TEMP_EL
	STD Z+0x02,TEMP_H
	STD Z+0x03,TEMP_L

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
.endif
