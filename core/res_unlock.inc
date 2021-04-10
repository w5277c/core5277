;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;04.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RES_UNLOCK
.else
.set DEF_C5_RES_UNLOCK = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RES_UNLOCK"
.endif

;--------------------------------------------------------
C5_RES_UNLOCK:
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

	MCALL C5_DISPATCHER_LOCK
	LDI XH,high(_C5_RESOURCE_QUEUE)
	LDI XL,low(_C5_RESOURCE_QUEUE)
	LDI LOOP_CNTR,_C5_RES_QUEUE_SIZE
	LDI FLAGS,0x00
_C5_RES_UNLOCK__QUEUE_LOOP:
	MOV ZH,XH
	MOV ZL,XL
	LD TEMP_L,X+														;RES_ID
	LD TEMP_H,X+														;PID
	;Проверяем на пустой элемент очереди
	CPI TEMP_H,0x00
	BREQ _C5_RES_UNLOCK__END
	;Сверяем ид ресурсов
	CP TEMP_L,ACCUM
	BRNE _C5_RES_UNLOCK__QUEUE_NEXT
	;Проверяем на поиск последующей задачи ожидающей данного ресурса
	CPI FLAGS,0x00
	BREQ _C5_RES_UNLOCK__DATA_MOVE
	;Получаем адрес заголовка следующей задачи в очереди
	MOV TEMP,TEMP_H
	MCALL _C5_PROC_HEADER_GET
	;Переводим задачу в состоянеи готовности
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_C5_PROC_STATE_READY
	STD Z+_C5_PROC_STATE,TEMP
	RJMP _C5_RES_UNLOCK__END
_C5_RES_UNLOCK__DATA_MOVE:
	;Сверяем ид задач
	CP TEMP_H,_PID
	BRNE _C5_RES_UNLOCK__QUEUE_NEXT
	;Сдвигаю элементы очереди на один
	SUBI LOOP_CNTR,0x02
	MCALL RAM_COPY8
	;Записываем нули в последнюю ячейку
	LDI TEMP,0x00
	STS _C5_RESOURCE_QUEUE+_C5_RES_QUEUE_SIZE-0x02,TEMP
	STS _C5_RESOURCE_QUEUE+_C5_RES_QUEUE_SIZE-0x01,TEMP
	;Устанавливаю признак поиска последующей задачи
	LDI FLAGS,0x01
	CPI LOOP_CNTR,0x00
	BRNE _C5_RES_UNLOCK__QUEUE_LOOP
	RJMP _C5_RES_UNLOCK__END
_C5_RES_UNLOCK__QUEUE_NEXT:
	SUBI LOOP_CNTR,0x02
	BRNE _C5_RES_UNLOCK__QUEUE_LOOP
_C5_RES_UNLOCK__END:
	MCALL C5_DISPATCHER_UNLOCK

	POP LOOP_CNTR
	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
