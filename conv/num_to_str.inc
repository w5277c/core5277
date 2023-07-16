;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_NUM_TO_STR
.else
.set DEF_NUM_TO_STR = 1

.if REPORT_INCLUDES == 0x01
.message "included NUM_TO_STR"
.endif

.include	"./math/div16x8.inc"

;--------------------------------------------------------
NUM_TO_STR:
;--------------------------------------------------------
;Конвертирование числа 8b в текст
;Конец строки не указывается
;IN: TEMP_L-число (0-255), Z-dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	CLR TEMP_H
	LDI LOOP_CNTR,0x02
_NUM_TO_STR__LOOP:
	LDI TEMP,0x0a
	MCALL DIV16x8
	SUBI TEMP,(0x100-0x30)
	PUSH TEMP
	DEC LOOP_CNTR
	BRNE _NUM_TO_STR__LOOP
	SUBI TEMP_L,(0x100-0x30)
	PUSH TEMP_L

	LDI LOOP_CNTR,0x02
_NUM_TO_STR__LOOP2:
	POP TEMP
	SBRC TEMP_L,0x07
	RJMP PC+0x03
	CPI TEMP,0x30
	BREQ PC+0x03
	ST Z+,TEMP
	ORI TEMP_L,(1<<0x07)
	DEC LOOP_CNTR
	BRNE _NUM_TO_STR__LOOP2
	POP TEMP
	ST Z+,TEMP
	
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	RET
.endif
