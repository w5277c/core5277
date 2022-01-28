;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.01.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_NOTIFY
.else
.set DEF_C5_NOTIFY = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_NOTIFY"
.endif

;--------------------------------------------------------
C5_NOTIFY:
;--------------------------------------------------------
;Меняем стостояние задачи
;_C5_PROC_STATE_TIME_WAIT -> _C5_PROC_STATE_READY
;IN:_PID-ид задачи
;--------------------------------------------------------
	PUSH_Z
	PUSH_X

	MCALL C5_DISPATCHER_LOCK
	MCALL _C5_PROC_HEADER_GET
	LDD XH,Z+_C5_PROC_STATE
	MOV XL,XH
	ANDI XL,0x0f
	CPI XL,_C5_PROC_STATE_TIME_WAIT
	BRNE PC+0x04
	ANDI XH,0xf0
	ORI XH,_C5_PROC_STATE_READY
	STD Z+_C5_PROC_STATE,XH
	MCALL C5_DISPATCHER_LOCK

	POP_X
	POP_Z
	RET
.endif
