;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
	;Ссылаются в никуда, толкьо чтобы были метки
	LOGSTR_CORE:
	LOGSTR_NEW_LINE:
	LOGSTR_NULL:
	LOGSTR_ERROR:
	.db	"?",0x00

	.MACRO C5_LOG_ROMSTR
	.ENDMACRO
	.MACRO C5_LOG_MARK_S
	.ENDMACRO
	.MACRO C5_LOG_MARK_R
	.ENDMACRO

	C5_LOG_INIT:
	C5_LOG_COREFAULT:
	C5_LOG_TASKSDUMP:
	C5_LOG_TASKDUMP:
	C5_LOG_STACKDUMP:
	C5_LOG_RAMDUMP:
	C5_LOG_ROMDUMP:
	C5_LOG_EEPROMDUMP:
	C5_LOG_MARK:
	C5_LOG_SDNF:
	C5_LOG_STR:
	C5_LOG_BYTES:
	C5_LOG_WORD:
	C5_LOG_BYTE:
	C5_LOG_CHAR:
	C5_LOG_NUMx8:
		RET
