;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_REDUCE
.else
.set DEF_C5_RAM_REDUCE = 1

.include	"./core/ram/ram_realloc.inc"

;--------------------------------------------------------
C5_RAM_REDUCE:
;--------------------------------------------------------
;Освобождаем часть памяти
;IN: TEMP - PID, ACCUM - размер, на который нужно
;освободить
;OUT: Z - адрес на выделенную часть, TEMP - общий объем
;flag Z(false - error,owerflow)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH ACCUM
	PUSH TEMP
	MCALL C5_DISPATCHER_LOCK

	;Получаем заголовок
	MCALL _C5_PROC_HEADER_GET
	;Получаем размер уже выделенной памяти
	LDD TEMP,Z+_C5_PROC_RAM_SIZE
	SUB TEMP,ACCUM
	BRCS _C5_RAM_REDUCE__ERROR
	RJMP _C5_RAM_REALLOC__EP2
_C5_RAM_REDUCE__ERROR:
	CLZ
	MOV ZH,YH
	MOV ZL,YL
	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	POP ACCUM
	POP_Y
	POP_X
	RET
.endif
