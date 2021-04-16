;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_BIT_FROM_WORD
.else
.set DEF_BIT_FROM_WORD = 1

.if REPORT_INCLUDES == 0x01
.message "included BIT_FROM_WORD"
.endif

;--------------------------------------------------------
BIT_FROM_WORD:
;--------------------------------------------------------
;Возвращает значение бита в флаге C
;IN:Y - 16бит значение, TEMP - номер бита(0-15)
;OUT: флаг C
;-START--------------------------------------------------
	SBRC TEMP,0x03
	MOV YL,YH
	SBRC TEMP,0x02
	SWAP YL
	ANDI TEMP,0x03
	INC TEMP
_BIT_FROM_WORD__LOOP:
	LSR YL
	DEC TEMP
	BRNE _BIT_FROM_WORD__LOOP
	RET
.endif