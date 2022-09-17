;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.08.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_POP_TEMP32
.else
.set DEF_POP_TEMP32 = 1

.if REPORT_INCLUDES == 0x01
.message "included POP_TEMP32"
.endif

;--------------------------------------------------------
POP_TEMP32:
;--------------------------------------------------------
;Извлекаем из стека TEMP_EH,EL,H,L
;--------------------------------------------------------
	POP _RESULT_L
	POP _RESULT_H
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	PUSH _RESULT_H
	PUSH _RESULT_L
	RET