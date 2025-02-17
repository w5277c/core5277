;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.06.2021	w5277c@gmail.com			Начало
;12.10.2024	konstantin@5277.ru		Реализация виртуальных портов
;-----------------------------------------------------------------------------------------------------------------------
;Выключаем бит по в ячейке расположенной по адресу 0-255
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_REG_BIT_LO
.else
.set DEF_REG_BIT_LO = 1

.if REPORT_INCLUDES == 0x01
.message "included REG_BIT_LO"
.endif

;--------------------------------------------------------
REG_BIT_LO:
;--------------------------------------------------------
;Выключаем бит регистра
;IN: TEMP-адрес,ACCUM-число бита
;--------------------------------------------------------
	PUSH_Z
	MOV ZH,C0x00
	MOV ZL,TEMP
	RCALL REG_BIT_LO_BY_Z
	POP_Z
	RET

REG_BIT_LO_BY_Z:
	PUSH_T16
	PUSH ACCUM
	COM ACCUM
	LDS TEMP_H,SREG
	CLI
	LD TEMP_L,Z
	AND TEMP_L,ACCUM
	ST Z,TEMP_L
	STS SREG,TEMP_H
.if VPORTS>0x00
	CPI ZH,high(VPORTS_ADDR)
	BRCS PC+0x04+_MCALL_SIZE
	BRNE PC+0x03
	CPI ZL,low(VPORTS_ADDR)
	BRCS PC+0x01+_MCALL_SIZE
	MCALL VPORT_SET
.endif
	POP ACCUM
	POP_T16
	RET
.endif
