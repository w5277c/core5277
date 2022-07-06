;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.03.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.if TIMERS16>0x00
.ifdef DEF_C5_TIMER16_START
.else
.set DEF_C5_TIMER16_START = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER16_START"
.endif

.include "./core/_timer16_offset.inc"

;--------------------------------------------------------
C5_TIMER16_START:
;--------------------------------------------------------
;Запуск таймера
;IN: PID,TEMP-номер таймера(0-31)
;--------------------------------------------------------
	PUSH_Z
	MCALL _C5_TIMER16_OFFSET
	STD Z+_C5_TIMER16_PROC_ID,PID
	POP_Z
	RET
.endif
.endif