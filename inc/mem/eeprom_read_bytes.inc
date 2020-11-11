;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.09.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_EEPROM_READ_BYTES
.else
.set DEF_C5_EEPROM_READ_BYTES = 1

.include	"./inc/mem/eeprom_read_byte.inc"

;--------------------------------------------------------
C5_EEPROM_READ_BYTES:
;--------------------------------------------------------
;Считываем байт из EEPROM
;IN: X-DST адрес, TEMP_H,TEMP_L - адрес в EEPROM
;LOOP_CNTR - длина данных
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP

C5_EEPROM_READ_BYTES__LOOP:
	MCALL C5_EEPROM_READ_BYTE

	ST X+,TEMP

	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	DEC LOOP_CNTR
	BRNE C5_EEPROM_READ_BYTES__LOOP

	POP TEMP
	POP LOOP_CNTR
	POP_X
	RET
.endif
