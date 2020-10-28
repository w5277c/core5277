;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;09.08.2020  w5277c@gmail.com        Разбиение на файлы
;19.08.2020  w5277c@gmail.com        Отсчет с нуля
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifndef DEF_C5_TIMER_SET
.set DEF_C5_TIMER_SET = 1

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
	PUSH TEMP
	PUSH ACCUM

	MOV ZL,TEMP
	LSL TEMP
	LSL TEMP
	ADD TEMP,ZL

	LDI ZH,high(_C5_TIMERS)
	LDI ZL,low(_C5_TIMERS)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	STD Z+_C5_TIMER_DEFAULT,ACCUM
	ANDI ACCUM,0x7f
	STD Z+_C5_TIMER_CNTR,ACCUM
	LDI TEMP,0xff
	STD Z+_C5_TIMER_PROC_ID,TEMP
	STD Z+_C5_TIMER_HANDLER+0x00,TEMP_H
	STD Z+_C5_TIMER_HANDLER+0x01,TEMP_L

	POP ACCUM
	POP TEMP
	POP_Z
	RET
.endif
