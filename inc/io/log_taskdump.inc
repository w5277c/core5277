;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_TASKDUMP
.else
.set DEF_CORE5277_LOG_TASKDUMP = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_new_line.inc"
.include	"./inc/io/log_stackdump.inc"

_LOGSTR_TID:
	.db   "TID:",0x00,0x00
_LOGSTR_STATE:
	.db   "STA:",0x00,0x00
_LOGSTR_RAM:
	.db   "RAM:",0x00,0x00
_LOGSTR_STACK:
	.db   "STK:",0x00,0x00

;--------------------------------------------------------
CORE5277_LOG_TASKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: PID
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ZH
	PUSH ZL

	MCALL _CORE5277_PROC_HEADER_GET
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_ABSENT
	BRNE PC+0x02
	RJMP CORE5277_LOG_TASKDUMP__END

	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE

	CORE5277_LOG_ROMSTR _LOGSTR_TID
	MOV TEMP,PID
	MCALL CORE5277_LOG_BYTE
	LDI TEMP,' '
	MCALL CORE5277_LOG_CHAR

	LDD TEMP,Z+_CORE5277_PROC_STATE
	CORE5277_LOG_ROMSTR _LOGSTR_STATE
	MCALL CORE5277_LOG_BYTE

	LDI TEMP,' '
	MCALL CORE5277_LOG_CHAR

	LDD TEMP_H,Z+_CORE5277_PROC_RAM_OFFSET+0x00
	LDD TEMP_L,Z+_CORE5277_PROC_RAM_OFFSET+0x01
	LDD TEMP,Z+_CORE5277_PROC_RAM_SIZE
	CORE5277_LOG_ROMSTR _LOGSTR_RAM
	MCALL CORE5277_LOG_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL CORE5277_LOG_CHAR
	POP TEMP
	MCALL CORE5277_LOG_BYTE
	LDI TEMP,'.'
	MCALL CORE5277_LOG_CHAR

	LDD TEMP_H,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	LDD TEMP,Z+_CORE5277_TASK_STACK_SIZE
	CORE5277_LOG_ROMSTR _LOGSTR_STACK
	MCALL CORE5277_LOG_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL CORE5277_LOG_CHAR
	POP TEMP
	MCALL CORE5277_LOG_BYTE

	MCALL CORE5277_LOG_STACKDUMP

CORE5277_LOG_TASKDUMP__END:
	POP ZL
	POP ZH
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
.endif
