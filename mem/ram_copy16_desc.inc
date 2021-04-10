;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;05.12.2020  w5277c@gmail.com        -с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_COPY16_DESC
.else
.set DEF_RAM_COPY16_DESC = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_COPY16_DESC"
.endif

;--------------------------------------------------------
RAM_COPY16_DESC:
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

	ADIW XL,0x01
	ADIW ZL,0x01
_RAM_COPY16_DESC__LOOP:
	LD TEMP,-X
	ST -Z,TEMP
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	CPI TEMP_H,0x00
	BRNE _RAM_COPY16_DESC__LOOP
	CPI TEMP_L,0x00
	BRNE _RAM_COPY16_DESC__LOOP

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP ZL
	POP ZH
	POP XL
	POP XH
	RET
.endif
