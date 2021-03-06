;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
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
;IN: TEMP_H,TEMP_L - 16b число, TEMP - 8b число
;OUT: TEMP_H, TEMP_L - 16b результат
;-START--------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR

	CLR XH
	CLR XL
	INC XL
	CLR LOOP_CNTR

_DIV16X8__A:
	CLC
	ROL TEMP_L
	ROL TEMP_H
	ROL LOOP_CNTR
	BRCS _DIV16X8__B
	CP LOOP_CNTR,TEMP
	BRCS _DIV16X8__C
_DIV16X8__B:
	SUB LOOP_CNTR,TEMP
	SEC
	RJMP _DIV16X8__D
_DIV16X8__C:
	CLC
_DIV16X8__D:
	ROL XL
	ROL XH
	BRCC _DIV16X8__A
	MOV TEMP_H,XH
	MOV TEMP_L,XL

   POP LOOP_CNTR
	POP_X
	RET
.endif
