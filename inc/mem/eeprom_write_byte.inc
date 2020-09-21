;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;14.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_EEPROM_WRITE_BYTE
.else
.set DEF_CORE5277_EEPROM_WRITE_BYTE = 1

;--------------------------------------------------------
CORE5277_EEPROM_WRITE_BYTE:
;--------------------------------------------------------
;Записываем байт в EEPROM
;IN: TEMP_H,TEMP_L - адрес в EEPROM, TEMP - значение
;--------------------------------------------------------
	PUSH TEMP
CORE5277_EEPROM_WRITE_BYTE__LOOP:
	LDS TEMP,EECR
	ANDI TEMP,(1<<EEPE)
	BRNE CORE5277_EEPROM_WRITE_BYTE__LOOP
	STS EEARH,TEMP_H
	STS EEARL,TEMP_L
	POP TEMP
	PUSH TEMP
	STS EEDR,TEMP
	MCALL CORE5277_DISPATCHER_LOCK								;TODO заменить за блокировку ресурса
	LDS TEMP,EECR
	ORI TEMP,(1<<EEMPE)
	STS EECR,TEMP
	ORI TEMP,(1<<EEPE)
	STS EECR,TEMP

	MCALL CORE5277_DISPATCHER_UNLOCK
	POP TEMP
	RET
.endif
