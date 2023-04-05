;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;28.10.2020	w5277c@gmail.com			Обновляю размер выделенной памяти в заголовке задачи
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_EXTEND
.else
.set DEF_C5_RAM_EXTEND = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RAM_EXTEND"
.endif

.include	"./core/ram/_ram_find_free_block.inc"
.include	"./core/ram/_ram_resize.inc"

;--------------------------------------------------------
C5_RAM_EXTEND:
;--------------------------------------------------------
;Увеличиваем блок выделенной памяти
;IN: PID, ACCUM-размер, на который нужно увеличить
;общий выделенный объем(сумма не может превышать 0x100)
;OUT: Y-адрес на всю выделенную часть, X-адрес на
;добавленную часть, flag Z(false-error,owerflow)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	MCALL C5_DISPATCHER_LOCK

	;Получаем заголовок
	MCALL _C5_PROC_HEADER_GET
	;Получаем размер уже выделенной памяти
	LDD TEMP,Z+_C5_PROC_RAM_SIZE
	ADD ACCUM,TEMP
	BRCS _C5_RAM_EXTEND__ERROR

	;Читаем адрес первой свободной ячейки выделяемой памяти
	LDD YH,Z+_C5_PROC_RAM_OFFSET+0x00
	LDD YL,Z+_C5_PROC_RAM_OFFSET+0x01
	;Проверяем на уже выделенную память
	CPI YH,0x00
	BRNE _C5_RAM_EXTEND__ALLOCATED
	CPI YL,0x00
	BRNE _C5_RAM_EXTEND__ALLOCATED

	PUSH TEMP
	MOV TEMP,ACCUM
	MCALL _C5_RAM_FIND_FREE_BLOCK
	POP TEMP
	BRNE _C5_RAM_EXTEND__ERROR

	;Проверяем, нужно ли обновлять вершину выделяемой памяти
	MOVW XL,YL
	ADD XL,ACCUM
	ADC XH,C0x00
	PUSH ACCUM
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x00
	CP ACCUM,XH
	BRCS PC+0x06
	LDS ACCUM,_C5_TOP_OF_FREE_RAM+0x01
	CP ACCUM,XL
	POP ACCUM
	BRCC _C5_RAM_EXTEND__SUCCESS
	;Обновляем адрес вершины выделяемой памяти в ядре и записываем маркеры безопасности
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL
	LDI ACCUM,high(_C5_SAFETY_THRESHOLD_VALUE)
	ST X,ACCUM
	ADIW XL,0x01
	LDI ACCUM,low(_C5_SAFETY_THRESHOLD_VALUE)
	ST X,ACCUM
	POP ACCUM
	RJMP _C5_RAM_EXTEND__SUCCESS

_C5_RAM_EXTEND__ALLOCATED:
	PUSH TEMP
	MOV TEMP,ACCUM
	MCALL _C5_RAM_RESIZE
	MOV ACCUM,TEMP
	POP TEMP
	BRNE _C5_RAM_EXTEND__ERROR

_C5_RAM_EXTEND__SUCCESS:
	STD Z+_C5_PROC_RAM_OFFSET+0x00,YH
	STD Z+_C5_PROC_RAM_OFFSET+0x01,YL
	STD Z+_C5_PROC_RAM_SIZE,ACCUM
	MOVW XL,YL
	ADD XL,TEMP
	ADC XH,C0x00
	SEZ
	RJMP _C5_RAM_EXTEND__END
_C5_RAM_EXTEND__ERROR:
	CLZ
_C5_RAM_EXTEND__END:
	MCALL C5_DISPATCHER_UNLOCK
	POP ACCUM
	POP TEMP
	POP_Z
	RET
.endif
