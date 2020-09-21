;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;04.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_RES_UNLOCK
.else
.set DEF_CORE5277_RES_UNLOCK = 1

;--------------------------------------------------------
CORE5277_RES_UNLOCK:
;--------------------------------------------------------
;Занимаем ресурс
;IN:ACCUM - ID ресурса
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH FLAGS
	PUSH LOOP_CNTR

	MCALL CORE5277_DISPATCHER_LOCK
	LDI XH,high(_CORE5277_RESOURCE_QUEUE)
	LDI XL,low(_CORE5277_RESOURCE_QUEUE)
	LDI LOOP_CNTR,_CORE5277_RES_QUEUE_SIZE
	LDI FLAGS,0x00
_CORE5277_RES_UNLOCK__QUEUE_LOOP:
	MOV ZH,XH
	MOV ZL,XL
	LD TEMP_L,X+														;RES_ID
	LD TEMP_H,X+														;PID
	;Проверяем на пустой элемент очереди
	CPI TEMP_H,0x00
	BREQ _CORE5277_RES_UNLOCK__END
	;Сверяем ид ресурсов
	CP TEMP_L,ACCUM
	BRNE _CORE5277_RES_UNLOCK__QUEUE_NEXT
	;Проверяем на поиск последующей задачи ожидающей данного ресурса
	CPI FLAGS,0x00
	BREQ _CORE5277_RES_UNLOCK__DATA_MOVE
	;Получаем адрес заголовка следующей задачи в очереди
	MOV TEMP,TEMP_H
	MCALL _CORE5277_PROC_HEADER_GET
	;Переводим задачу в состоянеи готовности
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_CORE5277_PROC_STATE_READY
	STD Z+_CORE5277_PROC_STATE,TEMP
	RJMP _CORE5277_RES_UNLOCK__END
_CORE5277_RES_UNLOCK__DATA_MOVE:
	;Сверяем ид задач
	CP TEMP_H,_PID
	BRNE _CORE5277_RES_UNLOCK__QUEUE_NEXT
	;Сдвигаю элементы очереди на один
	SUBI LOOP_CNTR,0x02
	MCALL CORE5277_RAM_COPY8
	;Записываем нули в последнюю ячейку
	LDI TEMP,0x00
	STS _CORE5277_RESOURCE_QUEUE+_CORE5277_RES_QUEUE_SIZE-0x02,TEMP
	STS _CORE5277_RESOURCE_QUEUE+_CORE5277_RES_QUEUE_SIZE-0x01,TEMP
	;Устанавливаю признак поиска последующей задачи
	LDI FLAGS,0x01
	CPI LOOP_CNTR,0x00
	BRNE _CORE5277_RES_UNLOCK__QUEUE_LOOP
	RJMP _CORE5277_RES_UNLOCK__END
_CORE5277_RES_UNLOCK__QUEUE_NEXT:
	SUBI LOOP_CNTR,0x02
	BRNE _CORE5277_RES_UNLOCK__QUEUE_LOOP
_CORE5277_RES_UNLOCK__END:
	MCALL CORE5277_DISPATCHER_UNLOCK

	POP LOOP_CNTR
	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
