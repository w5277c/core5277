;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.01.2021	w5277c@gmail.com			Начало, не проверено
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_STR_TRIM
.else
.set DEF_STR_TRIM = 1

.if REPORT_INCLUDES == 0x01
.message "included STR_TRIM"
.endif

;--------------------------------------------------------
STR_TRIM:
;--------------------------------------------------------
;Удаление пробелов вначале и конеце строки
;(пререписывает конец строки)
;IN: Y-адрес на строку
;OUT: Y-адрес на новое начало строки
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP

_STR_TRIM__LEFT_DIR:
	MOV XH,YH
	MOV XL,YL
	LD TEMP,Y+
	CPI TEMP,0x00
	BREQ _STR_TRIM__RIGHT_DIR
	CPI TEMP,0x09
	BREQ _STR_TRIM__LEFT_DIR
	CPI TEMP,0x0a
	BREQ _STR_TRIM__LEFT_DIR
	CPI TEMP,0x0d
	BREQ _STR_TRIM__LEFT_DIR
	CPI TEMP,0x20
	BREQ _STR_TRIM__LEFT_DIR

_STR_TRIM__BODY_SKIP_LOOP:
	LD TEMP,Y+
	CPI TEMP,0x00
	BRNE _STR_TRIM__BODY_SKIP_LOOP

_STR_TRIM__RIGHT_DIR:
	SBIW YL,0x01
	CP XL,YL
	BRNE PC+0x03
	CP XH,YH
	BREQ _STR_TRIM__END
_STR_TRIM__RIGHT_DIR_LOOP:
	LD TEMP,-Y
	CPI TEMP,0x09
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0a
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0d
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x20
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	ADIW YL,0x01
	LDI TEMP,0x00
	ST Y,TEMP

_STR_TRIM__END:
	MOV YH,XH
	MOV YL,XL

	POP TEMP
	POP_X
	RET
.endif
