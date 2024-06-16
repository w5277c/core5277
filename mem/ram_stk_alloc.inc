;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.10.2021	w5277c@gmail.com			Начало, не тестировано
;16.06.2024	konstantin@527.ru			Не работало на значениях ACCUM менее 4 байт
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
	PUSH ZL
	PUSH ZH
	PUSH FLAGS

	LDS FLAGS,SREG
	CLI
	;Запоминаем старое значение SP
	LDS YH,SPH
	LDS YL,SPL
	;Получаем новое место SP
	MOVW ZL,YL
	SUB ZL,ACCUM
	SBC ZH,C0x00
	;Задаю новый арес стека
	STS SPH,ZH
	STS SPL,ZL
	;Можно разрешать прерывания
	STS SREG,FLAGS

	;Перемещаем хранящиеся в старом стеке ZH,ZL,FLAGS и два байта точки возврата, на новое место
	LDD FLAGS,Y+0x01	;FLAGS
	STD Z+0x01,FLAGS
	LDD FLAGS,Y+0x02	;ZH
	STD Z+0x02,FLAGS
	LDD FLAGS,Y+0x03	;ZL
	STD Z+0x03,FLAGS
	LDD FLAGS,Y+0x04	;старший байт точки возврата
	STD Z+0x04,FLAGS
	LDD FLAGS,Y+0x05	;младший байт точки возврата
	STD Z+0x05,FLAGS
	POP FLAGS
	POP ZH
	POP ZL

	ADIW YL,0x06
	SUB YL,ACCUM
	SBC YH,C0x00

	RET
.endif
