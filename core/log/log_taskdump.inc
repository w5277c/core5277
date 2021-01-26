;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_TASKDUMP
.else
.set DEF_C5_LOG_TASKDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"
.include	"./core/log/log_byte.inc"
.include	"./core/log/log_word.inc"
.include	"./core/log/log_romstr.inc"
.include	"./core/log/log_cr.inc"
.include	"./core/log/log_stackdump.inc"

_LOGSTR_TID:
	.db   "TID:",0x00,0x00
_LOGSTR_STATE:
	.db   "STA:",0x00,0x00
_LOGSTR_RAM:
	.db   "RAM:",0x00,0x00
_LOGSTR_TIME:
	.db   "TIM:",0x00,0x00
_LOGSTR_STACK:
	.db   "STK:",0x00,0x00

;--------------------------------------------------------
C5_LOG_TASKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: PID
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ZH
	PUSH ZL

	MCALL _C5_PROC_HEADER_GET
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_ABSENT
	BRNE PC+0x02
	RJMP C5_LOG_TASKDUMP__END

	MCALL C5_LOG_CR

	C5_LOG_ROMSTR _LOGSTR_TID
	MOV TEMP,PID
	MCALL C5_LOG_BYTE
	LDI TEMP,' '
	MCALL C5_LOG_CHAR

	LDD TEMP,Z+_C5_PROC_STATE
	C5_LOG_ROMSTR _LOGSTR_STATE
	MCALL C5_LOG_BYTE

	LDI TEMP,' '
	MCALL C5_LOG_CHAR

	LDD TEMP_H,Z+_C5_PROC_RAM_OFFSET+0x00
	LDD TEMP_L,Z+_C5_PROC_RAM_OFFSET+0x01
	LDD TEMP,Z+_C5_PROC_RAM_SIZE
	C5_LOG_ROMSTR _LOGSTR_RAM
	MCALL C5_LOG_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL C5_LOG_CHAR
	POP TEMP
	MCALL C5_LOG_BYTE
	LDI TEMP,' '
	MCALL C5_LOG_CHAR

	LDD TEMP_H,Z+_C5_TASK_EXECTIME+0x00
	LDD TEMP_L,Z+_C5_TASK_EXECTIME+0x01
	LDD TEMP,Z+_C5_TASK_EXECTIME+0x02
	C5_LOG_ROMSTR _LOGSTR_TIME
	MCALL C5_LOG_WORD
	MCALL C5_LOG_BYTE
	LDI TEMP,' '
	MCALL C5_LOG_CHAR

	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	LDD TEMP,Z+_C5_TASK_STACK_SIZE
	C5_LOG_ROMSTR _LOGSTR_STACK
	MCALL C5_LOG_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL C5_LOG_CHAR
	POP TEMP
	MCALL C5_LOG_BYTE

	MCALL C5_LOG_STACKDUMP

C5_LOG_TASKDUMP__END:
	POP ZL
	POP ZH
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
.endif
