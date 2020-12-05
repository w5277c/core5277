;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.09.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;21.11.2020  w5277c@gmail.com        -с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_EEPROM_WRITE_BYTE
.else
.set DEF_EEPROM_WRITE_BYTE = 1

;--------------------------------------------------------
EEPROM_WRITE_BYTE:
;--------------------------------------------------------
;Записываем байт в EEPROM
;IN: TEMP_H,TEMP_L - адрес в EEPROM, TEMP - значение
;--------------------------------------------------------
	PUSH TEMP
__EEPROM_WRITE_BYTE__LOOP:
	LDS TEMP,EECR
	ANDI TEMP,(1<<EEPE)
	BRNE __EEPROM_WRITE_BYTE__LOOP
	STS EEARH,TEMP_H
	STS EEARL,TEMP_L
	POP TEMP
	PUSH TEMP
	STS EEDR,TEMP
	MCALL C5_DISPATCHER_LOCK								;TODO заменить за блокировку ресурса
	LDS TEMP,EECR
	ORI TEMP,(1<<EEMPE)
	STS EECR,TEMP
	ORI TEMP,(1<<EEPE)
	STS EECR,TEMP

	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	RET
.endif
