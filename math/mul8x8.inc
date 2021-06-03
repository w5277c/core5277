;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020	w5277c@gmail.com			-с5
;29.05.2021	w5277c@gmail.com			Программная реализация MUL для MUL_SUPPORT != 0x01
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_MUL8X8
.else
.set DEF_MUL8X8 = 1

.if REPORT_INCLUDES == 0x01
.message "included MUL8X8"
.endif

;--------------------------------------------------------
MUL8X8:
;--------------------------------------------------------
;Умножение 8b числа на 8б число
;IN: TEMP_L-8b число, TEMP-8b число
;OUT: TEMP_H/L-16b результат
;--------------------------------------------------------
.if MUL_SUPPORT != 0x01
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH TEMP

	MOV ACCUM,TEMP_L
	CLR TEMP_H
	LDI LOOP_CNTR,0x08
	LSR TEMP
_MUL8X8_LOOP:
	BRCC PC+0x02
	AND TEMP_H,ACCUM
	ROR TEMP_H
	ROR TEMP_L
	DEC LOOP_CNTR
	BRNE _MUL8X8_LOOP

	POP TEMP
	POP LOOP_CNTR
	POP ACCUM
.endif
.if MUL_SUPPORT == 0x01
	PUSH _RESULT_H
	PUSH _RESULT_L

	MUL TEMP_L,TEMP
	MOV TEMP_L,_RESULT_L
	MOV TEMP_H,_RESULT_H

	POP _RESULT_L
	POP _RESULT_H
.endif
	RET
.endif
