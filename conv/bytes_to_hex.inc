;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_BYTES_TO_HEX
.else
.set DEF_C5_BYTES_TO_HEX = 1

.include	"./inc/conv/byte_to_hex.inc"

;--------------------------------------------------------
C5_BYTES_TO_HEX:
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
	BREQ _C5_BYTES_TO_HEX__END

	MOV LOOP_CNTR,TEMP
_C5_BYTES_TO_HEX__LOOP:
	LD TEMP,X+
	MCALL C5_BYTE_TO_HEX
	ST Z+,TEMP_H
	ST Z+,TEMP_L
	DEC LOOP_CNTR
	BRNE _C5_BYTES_TO_HEX__LOOP
	LDI TEMP,0x00
	ST Z,TEMP

_C5_BYTES_TO_HEX__END:
	POP_Z
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP LOOP_CNTR
	RET
.endif
