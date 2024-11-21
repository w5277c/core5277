;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;21.09.2021	w5277c@gmail.com			Начало
;12.10.2024	konstantin@5277.ru		Реализация виртуальных портов
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_REG_BIT_GET
.else
.set DEF_REG_BIT_GET = 1

.if REPORT_INCLUDES == 0x01
.message "included REG_BIT_GET"
.endif

;--------------------------------------------------------
REG_BIT_GET:
;--------------------------------------------------------
;Считываем бит регистра
;IN: TEMP-адрес,ACCUM-число бита
;OUT: flag C-состояние бита(true = HI)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MOV ZH,C0x00
	MOV ZL,TEMP
	RCALL REG_BIT_GET_BY_Z

	POP TEMP
	POP_Z
	RET

REG_BIT_GET_BY_Z:
.if VPORTS>0x00
	CPI ZH,high(VPORTS_ADDR)
	BRCS PC+0x04+_MCALL_SIZE
	BRNE PC+0x03
	CPI ZL,low(VPORTS_ADDR)
	BRCS PC+0x01+_MCALL_SIZE
	MCALL VPORT_GET
.endif
	LD TEMP,Z
	AND TEMP,ACCUM
	ADD TEMP,C0xff
	RET
.endif
