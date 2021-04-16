;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIMER_OFFSET
.else
.set DEF_C5_TIMER_OFFSET = 1

;--------------------------------------------------------
_C5_TIMER_OFFSET:
;--------------------------------------------------------
;Получаем адрес на данные таймера
;IN: PID,TEMP - номер таймера(0-...)
;OUT: Z-адрес на данные таймера
;--------------------------------------------------------
	PUSH TEMP

	MOV ZL,TEMP
	LSL TEMP
	LSL TEMP
	ADD TEMP,ZL

	LDI ZH,high(_C5_TIMERS)
	LDI ZL,low(_C5_TIMERS)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	POP TEMP
	RET
.endif