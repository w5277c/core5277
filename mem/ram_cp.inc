;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.09.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_CP
.else
.set DEF_RAM_CP = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_CP"
.endif

;--------------------------------------------------------
RAM_CP:
;--------------------------------------------------------
;Сравнение блоков памяти
;IN: X-v1 addr, Z-v2 addr, LOOP_CNTR-длина
;OUT: флаг С(X<Z), флаг Z(X=Z)
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L

	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
	ADD XL,LOOP_CNTR
	ADC XH,C0x00

_RAM_CP__LOOP:
	LD TEMP_H,-X
	LD TEMP_L,-Z
	CP TEMP_H,TEMP_L
	BRNE _RAM_CP__END
	DEC LOOP_CNTR
	BRNE _RAM_CP__LOOP

_RAM_CP__END:
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_X
	RET
.endif
