;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
	;Ссылаются в никуда, толкьо чтобы были метки
	LOGSTR_CORE:
	LOGSTR_NEW_LINE:
	LOGSTR_NULL:
	LOGSTR_ERROR:
	.db	"?",0x00

	.MACRO CORE5277_LOG_ROMSTR
	.ENDMACRO
	.MACRO CORE5277_LOG_MARK_S
	.ENDMACRO
	.MACRO CORE5277_LOG_MARK_R
	.ENDMACRO

	CORE5277_LOG_INIT:
	CORE5277_LOG_COREFAULT:
	CORE5277_LOG_TASKSDUMP:
	CORE5277_LOG_TASKDUMP:
	CORE5277_LOG_STACKDUMP:
	CORE5277_LOG_RAMDUMP:
	CORE5277_LOG_ROMDUMP:
	CORE5277_LOG_EEPROMDUMP:
	CORE5277_LOG_MARK:
	CORE5277_LOG_SDNF:
	CORE5277_LOG_STR:
	CORE5277_LOG_BYTES:
	CORE5277_LOG_WORD:
	CORE5277_LOG_BYTE:
	CORE5277_LOG_CHAR:
		RET
