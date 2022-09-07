;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.10.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIMER_SET_Y
.else
.set DEF_C5_TIMER_SET_Y = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIMER_SET_Y"
.endif

.include "./core/_timer_offset.inc"

;--------------------------------------------------------
C5_TIMER_SET_Y:
;--------------------------------------------------------
;Установка нового адреса выделенной памяти
;IN: TEMP-номер таймера(0-...),
;Y-адрес на выделенную память
;--------------------------------------------------------
.if TIMER_C_ENABLE == 0x01
	CPI TEMP,TID_TIMER_C
	BREQ C5_TIMER_C_SET_Y
.endif
	PUSH_Z
	MCALL _C5_TIMER_OFFSET
	STD Z+_C5_TIMER_RAM_ADDR+0x00,YH
	STD Z+_C5_TIMER_RAM_ADDR+0x01,YL
	POP_Z
	RET
.if TIMER_C_ENABLE == 0x01
C5_TIMER_C_SET_Y:
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x00,YH
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x01,YL
	RET
.endif
.endif
