;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_MARK_R
.else
.set DEF_C5_LOG_MARK_R = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_mark.inc"

.MACRO C5_LOG_MARK_R
	PUSH _RESULT_L
	PUSH TEMP

	PUSH TEMP
	LDI TEMP,@0
	MOV _RESULT_L,TEMP
	POP TEMP

	PUSH TEMP_H
	PUSH TEMP_L
	PUSH @2
	PUSH @1
	MOV TEMP,_RESULT_L
	POP TEMP_H
	POP TEMP_L
	MCALL C5_LOG_MARK
	POP TEMP_L
	POP TEMP_H

	POP TEMP
	POP _RESULT_L
.ENDMACRO
.endif
.endif
