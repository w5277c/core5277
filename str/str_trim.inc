;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.01.2021	w5277c@gmail.com			Начало, не проверено
;01.10.2021	w5277c@gmail.com			Y->X
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
;Игнорирование пробелов в начале и в конце строки
;(пререписывает конец строки)
;IN: X-адрес на строку
;OUT: X-адрес на новое начало строки
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP

_STR_TRIM__LEFT_DIR:
	MOVW YL,XL
	LD TEMP,X+
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
	LD TEMP,X+
	CPI TEMP,0x00
	BRNE _STR_TRIM__BODY_SKIP_LOOP

_STR_TRIM__RIGHT_DIR:
	SBIW XL,0x01
	CP YL,XL
	BRNE PC+0x03
	CP YH,XH
	BREQ _STR_TRIM__END
_STR_TRIM__RIGHT_DIR_LOOP:
	LD TEMP,-X
	CPI TEMP,0x09
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0a
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0d
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x20
	BREQ _STR_TRIM__RIGHT_DIR_LOOP
	ADIW XL,0x01
	ST X,C0x00

_STR_TRIM__END:
	MOVW XL,YL

	POP TEMP
	POP_Y
	RET
.endif
