;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_RAM_EXTEND
.else
.set DEF_CORE5277_RAM_EXTEND = 1

.include	"./inc/mem/_ram_find_free_block.inc"
.include	"./inc/mem/_ram_resize.inc"

;--------------------------------------------------------
CORE5277_RAM_EXTEND:
;--------------------------------------------------------
;Увеличиваем блок выделенной памяти
;IN: PID, ACCUM - размер, на который нужно увеличить
;общий выделенный объем(сумма не может превышать 0x100)
;OUT: Z - адрес на всю выделенную часть, TEMP - общий
;объем, X - адрес на добавленную часть,
;flag Z(false - error,owerflow)
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM
	MCALL CORE5277_DISPATCHER_LOCK

	;Получаем заголовок
	MCALL _CORE5277_HEADER_GET
	;Получаем размер уже выделенной памяти
	LDD TEMP,Z+_CORE5277_PROC_RAM_SIZE
	ADD ACCUM,TEMP
	BRCS _CORE5277_RAM_EXTEND__ERROR

	;Читаем адрес первой свободной ячейки выделяемой памяти
	LDD YH,Z+_CORE5277_PROC_RAM_OFFSET+0x00
	LDD YL,Z+_CORE5277_PROC_RAM_OFFSET+0x01
	;Проверяем на уже выделенную память
	CPI YH,0x00
	BRNE _CORE5277_RAM_EXTEND__ALLOCATED
	CPI YL,0x00
	BRNE _CORE5277_RAM_EXTEND__ALLOCATED

	MOV TEMP,ACCUM
	MCALL _CORE5277_RAM_FIND_FREE_BLOCK
	BRNE _CORE5277_RAM_EXTEND__ERROR

	;Проверяем, нужно ли обновлять вершину выделяемой памяти
	MOV XH,YH
	MOV XL,YL
	ADD XL,ACCUM
	LDI ACCUM,0x00
	ADC XH,ACCUM
	LDS ACCUM,_CORE5277_TOP_OF_FREE_RAM+0x00
	CP ACCUM,XH
	BRCS PC+0x05
	LDS ACCUM,_CORE5277_TOP_OF_FREE_RAM+0x01
	CP ACCUM,XL
	BRCC _CORE5277_RAM_EXTEND__SUCCESS
	;Обновляем адрес вершины выделяемой памяти в ядре
	STS _CORE5277_TOP_OF_FREE_RAM+0x00,XH
	STS _CORE5277_TOP_OF_FREE_RAM+0x01,XL
	RJMP _CORE5277_RAM_EXTEND__SUCCESS

_CORE5277_RAM_EXTEND__ALLOCATED:
	MOV TEMP,ACCUM
	MCALL _CORE5277_RAM_RESIZE
	BRNE _CORE5277_RAM_EXTEND__ERROR

_CORE5277_RAM_EXTEND__SUCCESS:
	MOV ZH,YH
	MOV ZL,YL
	POP ACCUM
	PUSH ACCUM
	MOV XH,YH
	MOV XL,YL
	ADD XL,ACCUM
	CLR ACCUM
	ADC XH,ACCUM
	SEZ
	RJMP _CORE5277_RAM_REALLOC__END
_CORE5277_RAM_EXTEND__ERROR:
	CLZ
_CORE5277_RAM_EXTEND__END:
	MCALL CORE5277_DISPATCHER_UNLOCK
	POP ACCUM
	POP TEMP
	POP_Y
	RET
.endif
