;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.10.2022	konstantin@5277.ru			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_FIND
.else
.set DEF_RAM_FIND = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_FIND"
.endif

;--------------------------------------------------------
RAM_FIND:
;--------------------------------------------------------
;Поиск байта в блоке
;IN: X-адрес начала блока, TEMP-значение, LOOP_CNTR-длина
;OUT: TEMP-смещение, flag Z-(true-найдено)
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH ACCUM

	MOV TEMP,LOOP_CNTR
_RAM_FIND__LOOP:
	LD ACCUM,X+
	CP ACCUM,TEMP
	BREQ _RAM_FIND__SUCCESS
	DEC LOOP_CNTR
	BRNE _RAM_FIND__LOOP
	CLZ
	RJMP _RAM_FIND__END
_RAM_FIND__SUCCESS:
	SUB TEMP,LOOP_CNTR
	SEZ
_RAM_FIND__END:

	POP ACCUM
	POP LOOP_CNTR
	POP_X
	RET
.endif
