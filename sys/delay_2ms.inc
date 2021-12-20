;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.12.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DELAY_2MS
.else
.set DEF_DELAY_2MS = 1

.if REPORT_INCLUDES == 0x01
.message "included DELAY_2MS"
.endif

;--------------------------------------------------------
DELAY_2MS:
;--------------------------------------------------------
;Отсчитываем такты для выдерживания паузы
;IN TEMP_H,TEMP_L,TEMP - время в 0.002s
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
_DELAY_2MS__LOOP1:
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,(59*CORE_FREQ)/10
_DELAY_2MS__LOOP2:
	LDI TEMP_L,0x4f
_DELAY_2MS__LOOP3:
	NOP
	DEC TEMP_L
	BRNE _DELAY_2MS__LOOP3
	DEC TEMP_H
	BRNE _DELAY_2MS__LOOP2
	POP TEMP_L
	POP TEMP_H

	SUBI TEMP,0x01
	SBCI TEMP_L,0x00
	SBCI TEMP_H,0x00
	CPI TEMP,0x00
	BRNE _DELAY_2MS__LOOP1
	CPI TEMP_L,0x00
	BRNE _DELAY_2MS__LOOP1
	CPI TEMP_H,0x00
	BRNE _DELAY_2MS__LOOP1

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
