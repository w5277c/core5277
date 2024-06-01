;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.12.2023	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_EEPROM_ICP
.else
.set DEF_EEPROM_ICP = 1

.if REPORT_INCLUDES == 0x01
.message "included EEPROM_CP"
.endif

.include	"./mem/eeprom_read_byte.inc"

;--------------------------------------------------------
EEPROM_ICP:
;--------------------------------------------------------
;Сравнение блоков RAM и EEPROM с учетом инверсии
;IN: X-RAD addr, TEMP_H/L-EEPROM addr, LOOP_CNTR-длина
;OUT: флаг С(X<Z), флаг Z(X=Z)
;--------------------------------------------------------
	PUSH_X
	PUSH_T16
	PUSH ACCUM
	PUSH TEMP
	PUSH LOOP_CNTR

_EEPROM_ICP__LOOP:
	LDS ACCUM,X+
	COM ACCUM
	MCALL EEPROM_READ
	CP ACCUM,TEMP
	BRNE _EEPROM_ICP__END
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	DEC LOOP_CNTR
	BRNE _EEPROM_ICP__LOOP

_EEPROM_ICP_END:
	POP LOOP_CNTR
	POP TEMP
	POP ACCUM
	POP_T16
	POP_X
	RET
.endif
