;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;09.08.2020	w5277c@gmail.com			Разбиение на файлы
;19.08.2020	w5277c@gmail.com			Отсчет с нуля
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;04.10.2021	w5277c@gmail.com			Добавлен код таймера C
;-----------------------------------------------------------------------------------------------------------------------
.if TIMERS > 0x00
.ifdef DEF_C5_TIMER_STOP
.else
.set DEF_C5_TIMER_STOP = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER_STOP"
.endif

.include "./core/_timer_offset.inc"

;--------------------------------------------------------
C5_TIMER_STOP:
;--------------------------------------------------------
;Останов таймера
;IN TEMP-номер таймера(0-... или TID_TIMER_C)
;--------------------------------------------------------
.if TIMER_C_ENABLE == 0x01
	CPI TEMP,TID_TIMER_C
	BREQ _C5_TIMER_STOP__TIMER_C
.endif
	PUSH_Z
	MCALL _C5_TIMER_OFFSET
	STD Z+_C5_TIMER_PROC_ID,C0xff
	POP_Z
	RET
.if TIMER_C_ENABLE == 0x01
_C5_TIMER_STOP__TIMER_C:
	PUSH TEMP
	_C5_TIMERC_STOP
	POP TEMP
	RET
.endif
.endif
.endif
