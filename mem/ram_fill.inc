;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_FILL
.else
.set DEF_RAM_FILL = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_FILL"
.endif

;--------------------------------------------------------
RAM_FILL:
;--------------------------------------------------------
;Заполнение блока памяти значением
;IN: X-адрес, TEMP-значение, LOOP_CNTR-длина
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR

_RAM_FILL__LOOP:
	ST X+,TEMP
	DEC LOOP_CNTR
	BRNE _RAM_FILL__LOOP

	POP LOOP_CNTR
	POP_X
	RET
.endif
