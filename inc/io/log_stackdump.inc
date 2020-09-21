;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;05.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_STACKDUMP
.else
.set DEF_CORE5277_LOG_STACKDUMP = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_new_line.inc"

;--------------------------------------------------------
CORE5277_LOG_STACKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: Z - адрес на заголовок задачи
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH ZH
	PUSH ZL

	LDD LOOP_CNTR,Z+_CORE5277_TASK_STACK_SIZE
	LDD TEMP,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	LDD ZH,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	MOV ZL,TEMP

	CPI LOOP_CNTR,0x00
	BREQ CORE5277_LOG_STACKDUMP__END

	LDI TEMP,'['
	MCALL CORE5277_LOG_CHAR
	LD TEMP,Z+
_CORE5277_LOG_STACKDUMP__LOOP:
	LD TEMP,-Z
	MCALL CORE5277_LOG_BYTE
	DEC LOOP_CNTR
	BRNE _CORE5277_LOG_STACKDUMP__LOOP

CORE5277_LOG_STACKDUMP__END:
	LDI TEMP,']'
	MCALL CORE5277_LOG_CHAR

	POP ZL
	POP ZH
	POP LOOP_CNTR
	POP TEMP
	RET
.endif
.endif
