;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.09.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_CONSIST
.else
.set DEF_RAM_CONSIST = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_CONSIST"
.endif

;--------------------------------------------------------
RAM_CONSIST:
;--------------------------------------------------------
;Проверям содержимое блока на равенство 1 байту
;IN: X-addr, LOOP_CNTR-длина, TEMP-значение
;OUT: флаг Z(HI-полностью состоит из заданного значения)
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_H
	PUSH LOOP_CNTR

_RAM_CONSIST__LOOP:
	LD TEMP_H,X+
	CP TEMP_H,TEMP
	BRNE _RAM_CONSIST__END
	DEC LOOP_CNTR
	BRNE _RAM_CONSIST__LOOP

_RAM_CONSIST__END:
	POP LOOP_CNTR
	POP TEMP_H
	POP_X
	RET
.endif
