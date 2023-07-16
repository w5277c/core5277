;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;23.10.2022	konstantin@5277.ru			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_FIND_RANGE
.else
.set DEF_RAM_FIND_RANGE = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_FIND_RANGE"
.endif

;--------------------------------------------------------
RAM_FIND_RANGE:
;--------------------------------------------------------
;Поиск байта(в диапазоне) в блоке
;IN: X-адрес начала блока,
;TEMP_L-меньшее значение, TEMP_H-большее значение,
;LOOP_CNTR-длина
;OUT: TEMP-смещение, flag Z-(true-найдено)
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH ACCUM

	MOV TEMP,LOOP_CNTR
RAM_FIND_RANGE__LOOP:
	LD ACCUM,X+
	CP TEMP_H,ACCUM
	BRCS RAM_FIND_RANGE__NEXT
	CP ACCUM,TEMP_L
	BRCC RAM_FIND_RANGE__SUCCESS
RAM_FIND_RANGE__NEXT:
	DEC LOOP_CNTR
	BRNE _RAM_FIND__LOOP
	CLZ
	RJMP RAM_FIND_RANGE__END
RAM_FIND_RANGE__SUCCESS:
	SUB TEMP,LOOP_CNTR
	SEZ
RAM_FIND_RANGE__END:

	POP ACCUM
	POP LOOP_CNTR
	POP_X
	RET
.endif
