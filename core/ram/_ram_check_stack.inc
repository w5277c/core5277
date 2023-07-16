;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;03.07.2021	w5277c@gmail.com			Оптимизация
;03.04.2023	konstantin@5277.ru		Вывод ошибки
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_CHECK_STACK
.else
.set DEF_C5_RAM_CHECK_STACK = 1

.ifdef LOGGING_PORT
	.include	"./core/io/out_char.inc"
	.include	"./core/io/out_byte.inc"
	.include	"./core/io/out_word.inc"
.endif

;--------------------------------------------------------
_C5_RAM_CHECK_STACK:
;--------------------------------------------------------
;Проверяем на влияние на стек
;IN: Y-адрес блока, ACCUM - длина
;OUT: flag Z(true-влияния нет)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y

	;Считаем конец блока
	ADD YL,ACCUM
	ADC YH,C0x00
	;Получаем адрес вершины стека
	LDS XH,_C5_TOP_OF_STACK+0x00
	LDS XL,_C5_TOP_OF_STACK+0x01
	;Находим разницу
	SUB XL,YL
	SBC XH,YH
	BRCS _C5_RAM_CHECK_STACK__FAIL
	;Учитываем минимально допустимый размер
	SBIW XL,_C5_RAM_BORDER_SIZE
	BRCS _C5_RAM_CHECK_STACK__FAIL
	SEZ
	RJMP _C5_RAM_CHECK_STACK__END
_C5_RAM_CHECK_STACK__FAIL:
.ifdef LOGGING_PORT
	PUSH TEMP
	PUSH_T16
	LDI TEMP,'E'
	MCALL C5_OUT_CHAR
	MOV TEMP,PID
	MCALL C5_OUT_BYTE
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	MCALL C5_OUT_WORD
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	LDS TEMP_H,_C5_TOP_OF_STACK+0x00
	LDS TEMP_L,_C5_TOP_OF_STACK+0x01
	MCALL C5_OUT_WORD
	LDI TEMP,'!'
	MCALL C5_OUT_CHAR
	POP_T16
	POP TEMP
.endif
	CLZ
_C5_RAM_CHECK_STACK__END:
	POP_Y
	POP_X
	RET
.endif
