;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.08.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_POP_ALL
.else
.set DEF_POP_ALL = 1

.if REPORT_INCLUDES == 0x01
.message "included POP_ALL"
.endif

;--------------------------------------------------------
POP_ALL:
;--------------------------------------------------------
;Извлекаемиз стека все старшие 16 регистров
;--------------------------------------------------------
	POP _RESULT_L
	POP _RESULT_H
	POP r31
	POP r30
	POP r29
	POP r28
	POP r27
	POP r26
	POP r25
	POP r24
	POP r23
	POP r22
	POP r21
	POP r20
	POP r19
	POP r18
	POP r17
	POP r16
	PUSH _RESULT_H
	PUSH _RESULT_L
	RET