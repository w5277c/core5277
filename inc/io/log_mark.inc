;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_MARK
.else
.set DEF_C5_LOG_MARK = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_new_line.inc"

;--------------------------------------------------------
C5_LOG_MARK:
;--------------------------------------------------------
;Логирование метки
;IN _PID, TEMP - символ, TEMP_H,TEMP_L - hex код
;--------------------------------------------------------
	PUSH TEMP

	PUSH TEMP
	LDI TEMP,'#'
	MCALL C5_LOG_CHAR
	MOV TEMP,_PID
	MCALL C5_LOG_BYTE
	LDI TEMP,':'
	MCALL C5_LOG_CHAR
	POP TEMP
	MCALL C5_LOG_CHAR
	LDI TEMP,'.'
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_WORD
	C5_LOG_ROMSTR LOGSTR_NEW_LINE

	POP TEMP
	RET
.endif
.endif