;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.09.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;21.11.2020	w5277c@gmail.com			с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_EEPROM_READ_BYTE
.else
.set DEF_EEPROM_READ_BYTE = 1

.if REPORT_INCLUDES == 0x01
.message "included EEPROM_READ_BYTE"
.endif

;--------------------------------------------------------
EEPROM_READ_BYTE:
;--------------------------------------------------------
;Считываем байт из EEPROM
;IN: TEMP_H,TEMP_L-адрес в EEPROM
;OUT: TEMP-значение
;--------------------------------------------------------
__EEPROM_READ_BYTE__LOOP:
	LDS TEMP,EECR
	ANDI TEMP,(1<<EEPE)
	BRNE __EEPROM_READ_BYTE__LOOP
	STS EEARH,TEMP_H
	STS EEARL,TEMP_L
	MCALL C5_DISPATCHER_LOCK								;TODO заменить за блокировку ресурса
	LDS TEMP,EECR
	ORI TEMP,(1<<EERE)
	STS EECR,TEMP
	LDS TEMP,EEDR
	MCALL C5_DISPATCHER_UNLOCK
	RET
.endif
