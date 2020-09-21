;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_MARK
.else
.set DEF_CORE5277_LOG_MARK = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_new_line.inc"

;--------------------------------------------------------
CORE5277_LOG_MARK:
;--------------------------------------------------------
;Логирование метки
;IN _PID, TEMP - символ, TEMP_H,TEMP_L - hex код
;--------------------------------------------------------
	PUSH TEMP

	PUSH TEMP
	LDI TEMP,'#'
	MCALL CORE5277_LOG_CHAR
	MOV TEMP,_PID
	MCALL CORE5277_LOG_BYTE
	LDI TEMP,':'
	MCALL CORE5277_LOG_CHAR
	POP TEMP
	MCALL CORE5277_LOG_CHAR
	LDI TEMP,'.'
	MCALL CORE5277_LOG_CHAR
	MCALL CORE5277_LOG_WORD
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE

	POP TEMP
	RET
.endif
.endif
