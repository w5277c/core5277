;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.03.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.if TIMERS16>0x00
.ifdef DEF_C5_TIMER16_SET
.else
.set DEF_C5_TIMER16_SET = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER16_SET"
.endif

.include "./core/_timer16_offset.inc"

;--------------------------------------------------------
C5_TIMER16_SET:
;--------------------------------------------------------
;Установка таймера
;IN: TEMP-номер таймера(0-31),
;Y-адрес на выделенную память
;TEMP_H,TEMP_L-адрес обработчика
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
	STD Z+_C5_TIMER16_PROC_ID,C0xFF
	STD Z+_C5_TIMER16_HANDLER+0x00,TEMP_H
	STD Z+_C5_TIMER16_HANDLER+0x01,TEMP_L
	STD Z+_C5_TIMER16_RAM_ADDR+0x00,YH
	STD Z+_C5_TIMER16_RAM_ADDR+0x01,YL
	POP_X
	POP_Z
	RET
.endif
.endif