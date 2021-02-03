;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.09.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_STACKDUMP
.else
.set DEF_C5_LOG_STACKDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"
.include	"./core/log/log_byte.inc"
.include	"./core/log/log_word.inc"
.include	"./core/log/log_romstr.inc"

;--------------------------------------------------------
C5_LOG_STACKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: Z - адрес на заголовок задачи
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH ZH
	PUSH ZL

	LDD LOOP_CNTR,Z+_C5_TASK_STACK_SIZE
	LDD TEMP,Z+_C5_TASK_STACK_OFFSET+0x01
	LDD ZH,Z+_C5_TASK_STACK_OFFSET+0x00
	MOV ZL,TEMP

	CPI LOOP_CNTR,0x00
	BREQ C5_LOG_STACKDUMP__END

	LDI TEMP,'['
	MCALL C5_LOG_CHAR
	LD TEMP,Z+
_C5_LOG_STACKDUMP__LOOP:
	LD TEMP,-Z
	MCALL C5_LOG_BYTE
	DEC LOOP_CNTR
	BRNE _C5_LOG_STACKDUMP__LOOP

C5_LOG_STACKDUMP__END:
	LDI TEMP,']'
	MCALL C5_LOG_CHAR

	POP ZL
	POP ZH
	POP LOOP_CNTR
	POP TEMP
	RET
.endif
.endif
