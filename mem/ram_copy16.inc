;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;15.08.2020	w5277c@gmail.com			Оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_COPY16
.else
.set DEF_RAM_COPY16 = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_COPY16"
.endif

;--------------------------------------------------------
RAM_COPY16:
;--------------------------------------------------------
;Копирование блока памяти
;IN: X-SRC адрес, Z-DST адрес,
;TEMP_H,TEMP_L-длина(1-65535,0=65536)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP

	MOV YH,TEMP_H
	MOV YL,TEMP_L
_RAM_COPY16__LOOP:
	LD TEMP,X+
	ST Z+,TEMP
	SBIW YL,0x01
	BRNE _RAM_COPY16__LOOP

	POP TEMP
	POP_Z
	POP_Y
	POP_X
	RET
.endif
