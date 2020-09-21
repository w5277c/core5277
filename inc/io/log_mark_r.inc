;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_MARK_R
.else
.set DEF_CORE5277_LOG_MARK_R = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_mark.inc"

.MACRO CORE5277_LOG_MARK_R
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
	MCALL CORE5277_LOG_MARK
	POP TEMP_L
	POP TEMP_H

	POP TEMP
	POP _RESULT_L
.ENDMACRO
.endif
.endif
