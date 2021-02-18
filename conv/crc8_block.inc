;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CRC8_BLOCK
.else
.set DEF_CRC8_BLOCK = 1

.if REPORT_INCLUDES == 0x01
.message "included CRC8_BLOCK"
.endif

;--------------------------------------------------------
CRC8_BLOCK:
;--------------------------------------------------------
;Подсчет блока CRC(8 бит)
;IN: X-адрес начала блока, LOOP_CNTR - длина
;Y-адрес процедуры для вычисления байта
;OUT: ACCUM - сумма
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP
	CLR ACCUM

_CRC8_BLOCK__LOOP:
	LDI TEMP,low(_CRC8_BLOCK__CODEPOINT)
	PUSH TEMP
	LDI TEMP,high(_CRC8_BLOCK__CODEPOINT)
	PUSH TEMP
	PUSH YL
	PUSH YH
	LD TEMP,X+
	RET
_CRC8_BLOCK__CODEPOINT:
	DEC LOOP_CNTR
	BRNE _CRC8_BLOCK__LOOP

	POP TEMP
	POP LOOP_CNTR
	POP_X
	RET
.endif
