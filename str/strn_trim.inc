;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.01.2021	w5277c@gmail.com			Начало, частично проверено
;16.04.2021	w5277c@gmail.com			Нужна полная проверка
;01.10.2021	w5277c@gmail.com			Y->X
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_STRN_TRIM
.else
.set DEF_STRN_TRIM = 1

.if REPORT_INCLUDES == 0x01
.message "included STRN_TRIM"
.endif

;--------------------------------------------------------
STRN_TRIM:
;--------------------------------------------------------
;Пропуск пробелов вначале и конце строки(без изменения
;данных)
;IN: X-адрес на строку, TEMP-длина
;OUT: X-адрес на новое начало строки, TEMP-длина
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_L
	PUSH TEMP_H

	MOV TEMP_L,TEMP
_STRN_TRIM__LEFT_DIR:
	CPI TEMP_L,0x00
	BREQ _STRN_TRIM__RIGHT_DIR
	MOVW YL,XL
	LD TEMP,X+
	DEC TEMP_L
	CPI TEMP,0x00
	BREQ _STRN_TRIM__RIGHT_DIR
	CPI TEMP,0x09
	BREQ _STRN_TRIM__LEFT_DIR
	CPI TEMP,0x0a
	BREQ _STRN_TRIM__LEFT_DIR
	CPI TEMP,0x0d
	BREQ _STRN_TRIM__LEFT_DIR
	CPI TEMP,0x20
	BREQ _STRN_TRIM__LEFT_DIR

_STRN_TRIM__BODY_SKIP_LOOP:
	CPI TEMP_L,0x00
	BREQ _STRN_TRIM__RIGHT_DIR
	LD TEMP,X+
	DEC TEMP_L
	CPI TEMP,0x00
	BRNE _STRN_TRIM__BODY_SKIP_LOOP

_STRN_TRIM__RIGHT_DIR:
;	SBIW YL,0x01
_STRN_TRIM__RIGHT_DIR_LOOP:
	CP XL,YL
	BRNE PC+0x03
	CP XH,YH
	BREQ _STRN_TRIM__END
	LD TEMP,-X
	CPI TEMP,0x09
	BREQ _STRN_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0a
	BREQ _STRN_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x0d
	BREQ _STRN_TRIM__RIGHT_DIR_LOOP
	CPI TEMP,0x20
	BREQ _STRN_TRIM__RIGHT_DIR_LOOP
	ADIW XL,0x01

_STRN_TRIM__END:
	MOV TEMP,XL
	SUB TEMP,YL
	MOVW XL,YL

	POP TEMP_H
	POP TEMP_L
	POP_Y
	RET
.endif
