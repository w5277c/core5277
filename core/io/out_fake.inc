;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
	;Ссылаются в никуда, толкьо чтобы были метки
	OUTSTR_CORE:
	OUTSTR_NULL:
	OUTSTR_ERROR:
	.db	"?",0x00

	.MACRO C5_OUT_ROMSTR
	.ENDMACRO
	.MACRO C5_OUT_MARK_S
	.ENDMACRO
	.MACRO C5_OUT_MARK_R
	.ENDMACRO

	C5_OUT_INIT:
	C5_OUT_COREFAULT:
	C5_OUT_TASKSDUMP:
	C5_OUT_TASKDUMP:
	C5_OUT_STACKDUMP:
	C5_OUT_RAMDUMP:
	C5_OUT_ROMDUMP:
	C5_OUT_EEPROMDUMP:
	C5_OUT_MARK:
	C5_OUT_SDNF:
	C5_OUT_STR:
	C5_OUT_STR_EL:
	C5_OUT_BYTES:
	C5_OUT_WORD:
	C5_OUT_BYTE:
	C5_OUT_CHAR:
	C5_OUT_NUM8:
	C5_OUT_NUM16:
	C5_OUT_NUM32:
	C5_OUT_NUM16_HEX:
	C5_OUT_TIME32:
	C5_OUT_PRIM_HEX:
	C5_OUT_CR:
		RET
