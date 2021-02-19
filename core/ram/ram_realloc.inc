;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_REALLOC
.else
.set DEF_C5_RAM_REALLOC = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RAM_REALLOC"
.endif

.include	"./core/ram/_ram_find_free_block.inc"
.include	"./core/ram/_ram_resize.inc"

;--------------------------------------------------------
C5_RAM_REALLOC:
;--------------------------------------------------------
;Перевыделяем память(в том числе освобождаем)
;IN: PID, ACCUM - общий размер памяти
;OUT: Z - адрес на выделенную часть
;flag Z(false - error,owerflow)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH ACCUM
	PUSH TEMP
	MCALL C5_DISPATCHER_LOCK

	;Получаем заголовок
	MCALL _C5_PROC_HEADER_GET
	;Точка входа для C5_RAM_EXTEND
_C5_RAM_REALLOC__EP2:
	;Читаем адрес первой свободной ячейки выделяемой памяти
	LDD YH,Z+_C5_PROC_RAM_OFFSET+0x00
	LDD YL,Z+_C5_PROC_RAM_OFFSET+0x01
	;Проверяем на уже выделенную память
	CPI YH,0x00
	BRNE _C5_RAM_REALLOC__ALLOCATED
	CPI YL,0x00
	BRNE _C5_RAM_REALLOC__ALLOCATED

	MOV TEMP,ACCUM
	MCALL _C5_RAM_FIND_FREE_BLOCK
	BRNE _C5_RAM_REALLOC__ERROR

	;Проверяем, нужно ли обновлять вершину выделяемой памяти
	MOV XH,YH
	MOV XL,YL
	ADD XL,ACCUM
	LDI ACCUM,0x00
	ADC XH,ACCUM
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x00
	CP ACCUM,XH
	BRCS PC+0x05
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x01
	CP ACCUM,XL
	BRCC _C5_RAM_REALLOC__SUCCESS
	;Обновляем адрес вершины выделяемой памяти в ядре
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL
	RJMP _C5_RAM_REALLOC__SUCCESS

_C5_RAM_REALLOC__ALLOCATED:
	MOV TEMP,ACCUM
	MCALL _C5_RAM_RESIZE
	BRNE _C5_RAM_REALLOC__ERROR
_C5_RAM_REALLOC__SUCCESS:
	;Обновляем заголовок
	;Если длина нулевая, то и адрес блока нулевой
	CPI TEMP,0x00
	BRNE PC+0x03
	CLR YH
	CLR YL
	STD Z+_C5_PROC_RAM_OFFSET+0x00,YH
	STD Z+_C5_PROC_RAM_OFFSET+0x01,YL
	STD Z+_C5_PROC_RAM_SIZE,TEMP
	SEZ
	RJMP _C5_RAM_REALLOC__END
_C5_RAM_REALLOC__ERROR:
	CLZ
_C5_RAM_REALLOC__END:
	MOV ZH,YH
	MOV ZL,YL
	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	POP ACCUM
	POP_Y
	POP_X
	RET
.endif
