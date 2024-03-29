;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.03.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.if TIMERS16>0x00
.ifdef DEF_C5_TIMER16_START_AT
.else
.set DEF_C5_TIMER16_START_AT = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER16_START_AT"
.endif

.include "./core/_timer16_offset.inc"

;--------------------------------------------------------
C5_TIMER16_START_AT:
;--------------------------------------------------------
;Переназначение периода таймера и его запуск
;IN: TEMP-номер таймера(0-31),
;X-кол-во итераций по 0.000 050с
;--------------------------------------------------------
	PUSH_Z
	PUSH_X
	SBIW XL,0x01
	MCALL _C5_TIMER16_OFFSET
	STD Z+_C5_TIMER16_DEFAULT+0x00,XH
	STD Z+_C5_TIMER16_DEFAULT+0x01,XL
	STD Z+_C5_TIMER16_CNTR+0x00,XH
	STD Z+_C5_TIMER16_CNTR+0x01,XL
	STD Z+_C5_TIMER16_PROC_ID,PID
	POP_X
	POP_Z
	RET
.endif
.endif
