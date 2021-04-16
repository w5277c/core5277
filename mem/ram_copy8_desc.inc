;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;05.12.2020  w5277c@gmail.com        -с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_COPY8_DESC
.else
.set DEF_RAM_COPY8_DESC = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_COPY8_DESC"
.endif

;--------------------------------------------------------
RAM_COPY8_DESC:
;--------------------------------------------------------
;Копирование блока памяти(с декрементом смещений)
;IN: X-SRC адрес, Z-DST адрес, LOOP_CNTR-длина
;-START--------------------------------------------------
	PUSH XH
	PUSH XL
	PUSH ZH
	PUSH ZL
	PUSH LOOP_CNTR
	PUSH TEMP

	ADIW XL,0x01
	ADIW ZL,0x01
_RAM_COPY8_DESC__LOOP:
	LD TEMP,-X
	ST -Z,TEMP
	DEC LOOP_CNTR
	BRNE _RAM_COPY8_DESC__LOOP

	POP TEMP
	POP LOOP_CNTR
	POP ZL
	POP ZH
	POP XL
	POP XH
	RET
.endif