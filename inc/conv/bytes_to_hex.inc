;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_BYTES_TO_HEX
.else
.set DEF_CORE5277_BYTES_TO_HEX = 1

.include	"./inc/conv/byte_to_hex.inc"

;--------------------------------------------------------
CORE5277_BYTES_TO_HEX:
;--------------------------------------------------------
;Конвертирование блока данных в HEX(по два ASCII символа)
;IN: X-src адрес, TEMP-длина данных, Z-dst-адрес
;-START--------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X
	PUSH_Z

	TST TEMP
	BREQ _CORE5277_BYTES_TO_HEX__END

	MOV LOOP_CNTR,TEMP
_CORE5277_BYTES_TO_HEX__LOOP:
	LD TEMP,X+
	MCALL CORE5277_BYTE_TO_HEX
	ST Z+,TEMP_H
	ST Z+,TEMP_L
	DEC LOOP_CNTR
	BRNE _CORE5277_BYTES_TO_HEX__LOOP
	LDI TEMP,0x00
	ST Z,TEMP

_CORE5277_BYTES_TO_HEX__END:
	POP_Z
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP LOOP_CNTR
	RET
.endif
