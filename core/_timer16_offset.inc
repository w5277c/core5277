;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.03.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.if TIMERS16>0x00
.ifdef DEF_C5_TIMER16_OFFSET
.else
.set DEF_C5_TIMER16_OFFSET = 1

;--------------------------------------------------------
_C5_TIMER16_OFFSET:
;--------------------------------------------------------
;Получаем адрес на данные 16 бит таймера
;IN: PID,TEMP-номер таймера(0-15, максимум 28)
;OUT: Z-адрес на данные таймера
;--------------------------------------------------------
	PUSH TEMP

	MOV ZL,TEMP
	LSL TEMP
	LSL TEMP
	LSL TEMP
	ADD TEMP,ZL

	LDI_Z _C5_TIMERS16
	ADD ZL,TEMP
	ADC ZH,C0x00

	POP TEMP
	RET
.endif
.endif