;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;03.07.2021	w5277c@gmail.com			Z->Y
;10.04.2023	konstantin@5277.ru		Багфиксинг
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_REALLOC
.else
.set DEF_C5_RAM_REALLOC = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RAM_REALLOC"
.endif

.include	"./core/ram/_ram_find_free_block.inc"
.include	"./core/ram/_ram_resize.inc"

.ifdef LOGGING_PORT
	.include	"./core/io/out_char.inc"
	.include	"./core/io/out_byte.inc"
	.if LOGGING_RAMUSAGE == 1
		.include	"./core/io/out_word.inc"
	.endif
.endif

;--------------------------------------------------------
C5_RAM_REALLOC:
;--------------------------------------------------------
;Перевыделяем память(в том числе освобождаем)
;IN: PID, ACCUM-общий размер памяти
;OUT: Y-адрес на выделенную часть
;flag Z(false-error,owerflow)
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
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
	MOVW XL,YL
	ADD XL,ACCUM
	ADC XH,C0x00
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x00
	CP ACCUM,XH
	BRCS PC+0x05
	BRNE _C5_RAM_REALLOC__SUCCESS
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x01
	CP ACCUM,XL
	BRCC _C5_RAM_REALLOC__SUCCESS
	;Обновляем адрес вершины выделяемой памяти в ядре
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL
.ifdef LOGGING_PORT
	.if LOGGING_RAMUSAGE == 1
		PUSH TEMP
		LDI TEMP,'M'
		MCALL C5_OUT_CHAR
		PUSH_T16
		MOV TEMP_H,XH
		MOV TEMP_L,XL
		MCALL C5_OUT_WORD
		POP_T16
		LDI TEMP,'!'
		MCALL C5_OUT_CHAR
		POP TEMP
	.endif
.endif
	LDI ACCUM,high(_C5_SAFETY_THRESHOLD_VALUE)
	ST X,ACCUM
	ADIW XL,0x01
	LDI ACCUM,low(_C5_SAFETY_THRESHOLD_VALUE)
	ST X,ACCUM
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
	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	POP ACCUM
	POP_Z
	POP_X
	RET
.endif
