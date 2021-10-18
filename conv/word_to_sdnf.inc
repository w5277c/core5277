;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.08.2021	w5277c@gmail.com			Начало
;18.10.2021	w5277c@gmail.com			Оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_WORD_TO_SDNF
.else
.set DEF_WORD_TO_SDNF = 1

.if REPORT_INCLUDES == 0x01
.message "included WORD_TO_SDNF"
.endif

.include	"./math/div16x8.inc"

;--------------------------------------------------------
WORD_TO_SDNF:
;--------------------------------------------------------
;Конвертация 2 байта сотых в SDNF
;IN: TEMP_H/L-беззнаковое слово
;OUT: TEMP_H/L-SDNF
;--------------------------------------------------------
	PUSH TEMP

	LDI TEMP,0x64
	MCALL DIV16X8
	CPI TEMP_H,0x00
	BREQ PC+0x04
	LDI TEMP_H,0xff
	LDI TEMP_L,0xff
	RJMP _WORD_TO_SDNF__END
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,TEMP

_WORD_TO_SDNF__END:
	POP TEMP
	RET
.endif
