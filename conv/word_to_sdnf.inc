;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.08.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_WORD_TO_SDNF
.else
.set DEF_WORD_TO_SDNF = 1

.if REPORT_INCLUDES == 0x01
.message "included WORD_TO_SDNF"
.endif

.include	"./math/div100.inc"

;--------------------------------------------------------
WORD_TO_SDNF:
;--------------------------------------------------------
;Конвертация 2 байта сотых в SDNF
;IN: TEMP_H/TEMP_L-беззнаковое слово
;OUT: TEMP_H/TEMP_L-SDNF
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH ACCUM
	PUSH TEMP

	LDI TEMP,0x64
	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L
	MCALL DIV16X8
	MOV ACCUM,TEMP_L
	CPI TEMP_H,0x00
	BREQ PC+0x04
	LDI TEMP_H,0xff
	LDI TEMP_L,0xff
	RJMP _WORD_TO_SDNF__END
	PUSH TEMP_EL
	MCALL MUL16X8
	POP TEMP_EL
	SUB TEMP_EL,TEMP_L
	MOV TEMP_H,ACCUM
	MOV TEMP_L,TEMP_EL

_WORD_TO_SDNF__END:
	POP TEMP
	POP ACCUM
	POP TEMP_EL
	POP TEMP_EH
	RET
.endif
