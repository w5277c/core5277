;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.10.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_STK_FREE
.else
.set DEF_RAM_STK_FREE = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_STK_FREE"
.endif

;--------------------------------------------------------
RAM_STK_FREE:
;--------------------------------------------------------
;Освобождаем в стеке блок памяти
;IN: ACCUM-размер в байтах
;OUT: Y-0x0000
;--------------------------------------------------------
	PUSH FLAGS
	LDS FLAGS,SREG
	CLI
	LDS YH,SPH
	LDS YL,SPL
	ADD YL,ACCUM
	ADC YH,C0x00
	ADIW YL,0x03
	STS SPH,YH
	STS SPL,YL
	SUB YL,ACCUM
	SBC YH,C0x00
	STS SREG,FLAGS
	SBIW YL,0x02
	LDD FLAGS,Y+0x02
	PUSH FLAGS
	LDD FLAGS,Y+0x01
	PUSH FLAGS
	LDD FLAGS,Y+0x00
	CLR YH
	CLR YL
	RET
.endif
