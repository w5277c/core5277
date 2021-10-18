;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020	w5277c@gmail.com			-с5
;18.10.2021	w5277c@gmail.com			Возврат остатка
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DIV16X8
.else
.set DEF_DIV16X8 = 1

.if REPORT_INCLUDES == 0x01
.message "included DIV16X8"
.endif

;--------------------------------------------------------
DIV16X8:
;--------------------------------------------------------
;Деление 16b числа на 8б число
;IN: TEMP_H/L-16b делимое, TEMP-8b делитель
;OUT: TEMP_H/L-16b результат, TEMP-8b остаток
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,0x11
	SUB XH,XH
	CLR XL

_DIV16X8__LOOP:
	ROL TEMP_L
	ROL TEMP_H
	DEC LOOP_CNTR
	BREQ _DIV16X8__END
	ROL XL
	ROL XH
	SUB XL,TEMP
	SBC XH,C0x00
	BRCS PC+0x03
	SEC
	RJMP _DIV16X8__LOOP
	ADD XL,TEMP
	ADC XH,C0x00 
	CLC
	RJMP _DIV16X8__LOOP
_DIV16X8__END:
	MOV TEMP,XL

	POP LOOP_CNTR
	POP_X
	RET
.endif
