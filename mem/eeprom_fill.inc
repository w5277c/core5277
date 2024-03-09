;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;09.03.2024	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_EEPROM_FILL
.else
.set DEF_EEPROM_FILL = 1

.if REPORT_INCLUDES == 0x01
.message "included EEPROM_FILL"
.endif

.include	"./mem/eeprom_write_byte.inc"

;--------------------------------------------------------
EEPROM_FILL:
;--------------------------------------------------------
;Заполняем блок в EEPROM
;IN: TEMP-байт для записи, TEMP_H,TEMP_L-адрес в EEPROM,
;LOOP_CNTR - длина данных
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI ACCUM,0x01
__EEPROM_FILL__LOOP:
	MCALL EEPROM_WRITE_BYTE

	ADD TEMP_L,ACCUM
	ADC TEMP_H,C0x00
	DEC LOOP_CNTR
	BRNE __EEPROM_FILL__LOOP

	POP LOOP_CNTR
	POP ACCUM
	RET
.endif
