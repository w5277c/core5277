;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CRC16_BLOCK
.else
.set DEF_CRC16_BLOCK = 1

;--------------------------------------------------------
CRC16_BLOCK:
;--------------------------------------------------------
;Подсчет блока CRC(16 бит)
;IN: X-адрес начала блока, TEMP_H/L-длина
;Y-адрес процедуры для вычисления байта
;OUT: TEMP_EH,EL-сумма
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

_CRC16_BLOCK__LOOP:
	LDI TEMP,low(_CRC16_BLOCK__CODEPOINT)
	PUSH TEMP
	LDI TEMP,high(_CRC16_BLOCK__CODEPOINT)
	PUSH TEMP
	PUSH YL
	PUSH YH
	LD TEMP,X+
	RET
_CRC16_BLOCK__CODEPOINT:
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	CPI TEMP_L,0x00
	BRNE _CRC16_BLOCK__LOOP
	CPI TEMP_H,0x00
	BRNE _CRC16_BLOCK__LOOP

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_X
	RET
.endif
