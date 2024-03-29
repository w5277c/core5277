;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.10.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;13.06.2021	w5277c@gmail.com			Обновлена информация об авторских правах
;04.10.2021	w5277c@gmail.com			Добавлен код таймера C
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIMER_START_AT
.else
.set DEF_C5_TIMER_START_AT = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER_START_AT"
.endif

.include "./core/_timer_offset.inc"

;--------------------------------------------------------
C5_TIMER_START_AT:
;--------------------------------------------------------
;Переназначение периода таймера и его запуск
;IN: TEMP-номер таймера(0-...),
;ACCUM-кол-во итераций:
;7-й бит включен - итерация по 0.002c иначе по 0.000 050с
;диапазон значений 0x01-0x7f
;--------------------------------------------------------
.if TIMER_C_ENABLE == 0x01
	CPI TEMP,TID_TIMER_C
	BREQ C5_TIMER_C_START_AT
.endif
.if TIMERS>0x00
	PUSH_Z
	PUSH ACCUM
	MCALL _C5_TIMER_OFFSET
	STD Z+_C5_TIMER_DEFAULT,ACCUM
	ANDI ACCUM,0x7f
	STD Z+_C5_TIMER_CNTR,ACCUM
	STD Z+_C5_TIMER_PROC_ID,PID
	POP ACCUM
	POP_Z
.endif
	RET
.if TIMER_C_ENABLE == 0x01
C5_TIMER_C_START_AT:
	PUSH TEMP
	MOV TEMP,ACCUM
	_C5_TIMERC_SET_PERIOD
	_C5_TIMERC_START
	POP TEMP
	RET
.endif
.endif
