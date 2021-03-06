;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com			Начало
;02.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_ROMSTR
.else
.set DEF_C5_LOG_ROMSTR = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_str.inc"

.MACRO C5_LOG_ROMSTR
	PUSH YH
	PUSH YL
	LDI YH,high(@0)|0x80
	LDI YL,low(@0)
	MCALL C5_LOG_STR
	POP YL
	POP YH
.ENDMACRO
.endif
.endif
