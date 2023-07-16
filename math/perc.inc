;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;31.01.2023	konstantin@5277.ru			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PERC
.else
.set DEF_PERC = 1

.if REPORT_INCLUDES == 0x01
.message "included PERC"
.endif

.include	"./math/mul8x8.inc"
.include	"./math/div16x8.inc"

;--------------------------------------------------------
PERC:
;--------------------------------------------------------
;Проценты 8b числа
;IN: TEMP_H-макс. значение, TEMP_L-текущее
;OUT: TEMP-результат(0-100%)
;--------------------------------------------------------
	PUSH_T16

	CPI TEMP_H,0x00
	BREQ _PERC_OVERFLOW

	PUSH TEMP_H
	LDI TEMP,0x64
	MCALL MUL8X8
	POP TEMP
	MCALL DIV16X8
	CPI TEMP_H,0x00
	BRNE _PERC_OVERFLOW
	MOV TEMP,TEMP_L
	CPI TEMP_L,0x65
	BRCS _PERC_END
_PERC_OVERFLOW:
	LDI TEMP,0x64
_PERC_END:

	POP_T16
	RET
.endif
