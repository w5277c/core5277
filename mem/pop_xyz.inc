;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.08.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_POP_XYZ
.else
.set DEF_POP_XYZ = 1

.if REPORT_INCLUDES == 0x01
.message "included POP_XYZ"
.endif

;--------------------------------------------------------
POP_XYZ:
;--------------------------------------------------------
;Извлекаем из стека XYZ
;--------------------------------------------------------
	POP _RESULT_L
	POP _RESULT_H
	POP ZL
	POP ZH
	POP YL
	POP YH
	POP XL
	POP XH
	PUSH _RESULT_H
	PUSH _RESULT_L
	RET
