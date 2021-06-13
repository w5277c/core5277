;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;09.08.2020  w5277c@gmail.com        Разбиение на файлы
;19.08.2020  w5277c@gmail.com        Отсчет с нуля
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIMER_SET
.else
.set DEF_C5_TIMER_SET = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER_SET"
.endif

.include "./core/_timer_offset.inc"

;--------------------------------------------------------
C5_TIMER_SET:
;--------------------------------------------------------
;Установка таймера
;IN: TEMP - номер таймера(0-...),
;TEMP_H,TEMP_L - адрес обработчика
;ACCUM - кол-во итераций:
;7-й бит включен - итерация по 0.002c иначе по 0.000 050с
;диапазон значений 0x01-0x7f
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP

	MCALL _C5_TIMER_OFFSET

	LDS TEMP,SREG
	CLI
	STD Z+_C5_TIMER_DEFAULT,ACCUM
	ANDI ACCUM,0x7f
	STD Z+_C5_TIMER_CNTR,ACCUM
	LDI ACCUM,0xff
	STD Z+_C5_TIMER_PROC_ID,ACCUM
	STD Z+_C5_TIMER_HANDLER+0x00,TEMP_H
	STD Z+_C5_TIMER_HANDLER+0x01,TEMP_L
	STS SREG,TEMP

	POP TEMP
	POP ACCUM
	POP_Z
	RET
.endif
