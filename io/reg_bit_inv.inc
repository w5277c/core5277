;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.06.2021	w5277c@gmail.com			Начало
;12.10.2024	konstantin@5277.ru		Реализация виртуальных портов
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_REG_BIT_INV
.else
.set DEF_REG_BIT_INV = 1

.if REPORT_INCLUDES == 0x01
.message "included REG_BIT_INV"
.endif

;--------------------------------------------------------
REG_BIT_INV:
;--------------------------------------------------------
;Инвертируем бит регистра и возвращаем его новое значение
;IN: TEMP-адрес,ACCUM-число бита
;OUT: flag C-состояние бита(true = HI)
;--------------------------------------------------------
	PUSH_Z
	PUSH_T16

	MOV ZH,C0x00
	MOV ZL,TEMP
	LDS TEMP_H,SREG
	CLI
	LD TEMP_L,Z
	EOR TEMP_L,ACCUM
	ST Z,TEMP_L
	AND TEMP_L,ACCUM
	STS SREG,TEMP_H
.if VPORTS>0x00
	CPI TEMP,VPORTS_ADDR
	BRCS PC+0x01+_MCALL_SIZE
	MCALL VPORT_SET
.endif

	ADD TEMP_L,C0xff

	POP_T16
	POP_Z
	RET
.endif
