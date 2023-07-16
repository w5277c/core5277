;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;09.08.2020	w5277c@gmail.com			Разбиение на файлы
;19.08.2020	w5277c@gmail.com			Отсчет с нуля
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;22.09.2021	w5277c@gmail.com			Добавлен адрес на выделенную память
;04.10.2021	w5277c@gmail.com			Добавлен код таймера C
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
;IN: TEMP-номер таймера(0-...),
;Y-адрес на выделенную память
;TEMP_H,TEMP_L-адрес обработчика
;ACCUM-кол-во итераций:
;7-й бит включен - итерация по 0.002c иначе по 0.000 050с
;диапазон значений 0x01-0x7f
;--------------------------------------------------------
.if TIMER_C_ENABLE == 0x01
	CPI TEMP,TID_TIMER_C
	BREQ C5_TIMER_C_SET
.endif
.if TIMERS>0x00
	PUSH_Z
	PUSH ACCUM
	MCALL _C5_TIMER_OFFSET
	STD Z+_C5_TIMER_DEFAULT,ACCUM
	ANDI ACCUM,0x7f
	STD Z+_C5_TIMER_CNTR,ACCUM
	STD Z+_C5_TIMER_PROC_ID,C0xFF
	STD Z+_C5_TIMER_HANDLER+0x00,TEMP_H
	STD Z+_C5_TIMER_HANDLER+0x01,TEMP_L
	STD Z+_C5_TIMER_RAM_ADDR+0x00,YH
	STD Z+_C5_TIMER_RAM_ADDR+0x01,YL
	POP ACCUM
	POP_Z
.endif
	RET
.if TIMER_C_ENABLE == 0x01
C5_TIMER_C_SET:
	STS _C5_TIMER_C_PID,PID
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x00,TEMP_H
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x01,TEMP_L
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x00,YH
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x01,YL
	PUSH TEMP
	MOV TEMP,ACCUM
	_C5_TIMERC_SET_PERIOD
	POP TEMP
	RET
.endif
.endif
