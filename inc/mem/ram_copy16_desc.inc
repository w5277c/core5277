;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_COPY16_DESC
.else
.set DEF_C5_RAM_COPY16_DESC = 1

;--------------------------------------------------------
C5_RAM_COPY16_DESC:
;--------------------------------------------------------
;Копирование блока памяти(с декременом смещений)
;IN: X-SRC адрес, Z-DST адрес,
;TEMP_H,TEMP_L-длина(1-65535,0=65536)
;-START--------------------------------------------------
	PUSH XH
	PUSH XL
	PUSH ZH
	PUSH ZL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
_C5_RAM_COPY16_DESC__LOOP:
	LD TEMP,X
	ST Z,TEMP
	SUBI XL,0x01
	SBCI XH,0x00
	SUBI ZL,0x01
	SBCI ZH,0x00
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	TST TEMP_H
	BRNE _C5_RAM_COPY16_DESC__LOOP
	TST TEMP_L
	BRNE _C5_RAM_COPY16_DESC__LOOP
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP ZL
	POP ZH
	POP XL
	POP XH
	RET
.endif
