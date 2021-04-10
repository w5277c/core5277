;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CRC16_MODBUS
.else
.set DEF_CRC16_MODBUS = 1

.if REPORT_INCLUDES == 0x01
.message "included CRC16_MODBUS"
.endif

;--------------------------------------------------------
CRC16_MODBUS:
;--------------------------------------------------------
;Подсчет CRC16 (x^16 + x^15 + x^2 + x^0)
;Изначально в TEMP_EH/EL должно быть 0xffff
;IN: TEMP-байт, TEMP_EH/EL-сумма
;OUT: TEMP_EH/EL-сумма
;--------------------------------------------------------
	PUSH LOOP_CNTR
	EOR TEMP_EL,TEMP
	LDI LOOP_CNTR,0x08
_CRC16_MODBUS_LOOP:
	LSR TEMP_EH
	ROR TEMP_EL
	BRCC _CRC16_MODBUS__NO_XOR
	LDI TEMP,0xA0
	EOR TEMP_EH,TEMP
	LDI TEMP,0x01
	EOR TEMP_EL,TEMP
_CRC16_MODBUS__NO_XOR:
	DEC LOOP_CNTR
	BRNE _CRC16_MODBUS_LOOP
	POP LOOP_CNTR
	RET
