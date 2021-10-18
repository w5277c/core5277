;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.10.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_RAM_STK_ALLOC
.else
.set DEF_RAM_STK_ALLOC = 1

.if REPORT_INCLUDES == 0x01
.message "included RAM_STK_ALLOC"
.endif

;--------------------------------------------------------
RAM_STK_ALLOC:
;--------------------------------------------------------
;Выделяем в стеке блок памяти
;IN: ACCUM-размер в байтах
;OUT: Y-адрес на память 
;--------------------------------------------------------
	PUSH FLAGS
	LDS FLAGS,SREG
	CLI
	;Меняем адрес стека
	LDS YH,SPH
	LDS YL,SPL
	ADIW YL,0x03
	SUB YL,ACCUM
	SBC YH,C0x00
	STS SPH,YH
	STS SPL,YL

	ADD YL,ACCUM
	ADC YH,C0x00
	SBIW YL,0x02
	STS SREG,FLAGS
	LDD FLAGS,Y+0x02
	PUSH FLAGS
	LDD FLAGS,Y+0x01
	PUSH FLAGS
	LDD FLAGS,Y+0x00

	;Устанвливаем Y на адрес выделенного  блока
	SUB YL,ACCUM
	SBC YH,C0x00
	ADIW YL,0x03
	RET
.endif
