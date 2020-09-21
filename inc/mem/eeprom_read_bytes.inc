;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;16.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_EEPROM_READ_BYTES
.else
.set DEF_CORE5277_EEPROM_READ_BYTES = 1

.include	"./inc/mem/eeprom_read_byte.inc"

;--------------------------------------------------------
CORE5277_EEPROM_READ_BYTES:
;--------------------------------------------------------
;Считываем байт из EEPROM
;IN: X-DST адрес, TEMP_H,TEMP_L - адрес в EEPROM
;LOOP_CNTR - длина данных
;--------------------------------------------------------
	PUSH_X
	PSUH LOOP_CNTR
	PUSH TEMP

CORE5277_EEPROM_READ_BYTES__LOOP:
	MCALL CORE5277_EEPROM_READ_BYTE
	
	STS X+,TEMP
	
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	DEC LOOP_CNTR
	BRNE CORE5277_EEPROM_READ_BYTE__LOOPS

	POP TEMP
	POP LOOP_CNTR
	POP_X
	RET
.endif
