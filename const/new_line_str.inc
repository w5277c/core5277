;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CONST_NEW_LINE_STR
.else
.set DEF_CONST_NEW_LINE_STR = 1

NEW_LINE_STR:
	.db	0x0d,0x0a,0x00,0x00
.endif
