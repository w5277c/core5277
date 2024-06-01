;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
;22.04.2024	konstantin@5277.ru		Не корректно добавлять 0x00 в конце, так как это не строковая процедура + PUSH TEMP
;-----------------------------------------------------------------------------------------------------------------------

;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_BYTES_TO_HEX
.else
.set DEF_BYTES_TO_HEX = 1

.if REPORT_INCLUDES == 0x01
.message "included BYTES_TO_HEX"
.endif

.include	"./conv/byte_to_hex.inc"

;--------------------------------------------------------
BYTES_TO_HEX:
;--------------------------------------------------------
;Конвертирование блока данных в HEX(по два ASCII символа)
;IN: X-src адрес, TEMP-длина данных, Z-dst-адрес
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH_X
	PUSH_Z

	TST TEMP
	BREQ _BYTES_TO_HEX__END

	MOV LOOP_CNTR,TEMP
_BYTES_TO_HEX__LOOP:
	LD TEMP,X+
	MCALL BYTE_TO_HEX
	ST Z+,TEMP_H
	ST Z+,TEMP_L
	DEC LOOP_CNTR
	BRNE _BYTES_TO_HEX__LOOP

_BYTES_TO_HEX__END:
	POP_Z
	POP_X
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP LOOP_CNTR
	RET
.endif
