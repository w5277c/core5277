;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.02.2021  w5277c@gmail.com			Начало (не работает)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CRC16_XMODEM
.else
.set DEF_CRC16_XMODEM = 1

.if REPORT_INCLUDES == 0x01
.message "included CRC16_XMODEM"
.endif

;--------------------------------------------------------
CRC16_XMODEM:
;--------------------------------------------------------
;Подсчет CRC16 (x^16 + x^12 + x^5 + x^0)
;Изначально в TEMP_EH/EL должно быть 0x0000
;IN: TEMP-байт, TEMP_EH/EL-сумма
;OUT: TEMP_EH/EL-сумма
;--------------------------------------------------------
	PUSH LOOP_CNTR
	EOR TEMP_EH,TEMP
	LDI LOOP_CNTR,0x08
_CRC16_XMODEM_LOOP:
	LSL TEMP_EL
	ROL TEMP_EH
	BRCC _CRC16_XMODEM__NO_XOR
	LDI TEMP,0x10
	EOR TEMP_EH,TEMP
	LDI TEMP,0x21
	EOR TEMP_EL,TEMP
_CRC16_XMODEM__NO_XOR:
	DEC LOOP_CNTR
	BRNE _CRC16_XMODEM_LOOP
	POP LOOP_CNTR
	RET
.endif
