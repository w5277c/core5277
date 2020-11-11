;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.09.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_EEPROM_WRITE_BYTES
.else
.set DEF_C5_EEPROM_WRITE_BYTES = 1

.include	"./inc/mem/eeprom_write_byte.inc"

;--------------------------------------------------------
C5_EEPROM_WRITE_BYTES:
;--------------------------------------------------------
;Записываем байт в EEPROM
;IN: X-SRC адрес, TEMP_H,TEMP_L - адрес в EEPROM,
;LOOP_CNTR - длина данных
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP

C5_EEPROM_WRITE_BYTES__LOOP:
	LD TEMP,X+

	MCALL C5_EEPROM_WRITE_BYTE

	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	DEC LOOP_CNTR
	BRNE C5_EEPROM_WRITE_BYTES__LOOP

	POP TEMP
	POP LOOP_CNTR
	POP_X
	RET
.endif
