;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.02.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_WD_RESET
.else
.set DEF_WD_RESET = 1

.if REPORT_INCLUDES == 0x01
.message "included WD_RESET"
.endif

;--------------------------------------------------------
WD_RESET:
;--------------------------------------------------------
;Выполняем перезагрузку МК по Watchdog через 1 секунду.
;--------------------------------------------------------
	CLI
	WDR
	LDS TEMP,WDTCR
	ORI TEMP,(1<<WDCE)|(1<<WDE)
	STS WDTCR,TEMP
	LDI TEMP,(0<<WDCE)|(1<<WDE)|(0<<WDIE)|(0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0)
	STS WDTCR,TEMP
	RJMP PC
.endif