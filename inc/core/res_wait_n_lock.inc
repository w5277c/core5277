;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;04.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_RES_WAIT_N_LOCK
.else
.set DEF_CORE5277_RES_WAIT_N_LOCK = 1

;--------------------------------------------------------
CORE5277_RES_WAIT_N_LOCK:
;--------------------------------------------------------
;Занимаем ресурс, если ресурс занят, то ждем освобождения
;IN:ACCUM - ID ресурса,
;TEMP_H/TEMP_L/TEMP - время ожидания в тиках ядра(2ms),
;0x000000 - не ждать
;OUT:флаг Z - результат(HI - успех)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH FLAGS
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH LOOP_CNTR
	MCALL CORE5277_DISPATCHER_LOCK
	LDI ZH,high(_CORE5277_RESOURCE_QUEUE)
	LDI ZL,low(_CORE5277_RESOURCE_QUEUE)
	LDI FLAGS,0x00
	LDI LOOP_CNTR,_CORE5277_RES_QUEUE_SIZE
_CORE5277_RES_WAIT_N_LOCK__QUEUE_LOOP:
	LD TEMP_EL,Z+														;RES_ID
	LD TEMP_EH,Z+														;PID
	;Не пустые элементы закончились
	CPI TEMP_EH,0x00
	BREQ _CORE5277_RES_WAIT_N_LOCK__QUEUE_LOOP_END
	CP TEMP_EL,ACCUM
	BRNE _CORE5277_RES_WAIT_N_LOCK__QUEUE_NEXT
	;Ресурс занят
	LDI FLAGS,0x01
	RJMP _CORE5277_RES_WAIT_N_LOCK__QUEUE_LOOP_END
_CORE5277_RES_WAIT_N_LOCK__QUEUE_NEXT:
	SUBI LOOP_CNTR,0x02
	BRNE _CORE5277_RES_WAIT_N_LOCK__QUEUE_LOOP
	;Весь список пройден, в очереди нет места
	RJMP _CORE5277_RES_WAIT_N_LOCK__QUEUE_FULL

_CORE5277_RES_WAIT_N_LOCK__QUEUE_LOOP_END:
	;Возвращается на адрес свободной ячейки
	SUBI ZL,0x02
	SBCI ZH,0x00
	;Помещаем в очередь ид ресурса и ид задачи
	ST Z+,ACCUM
	ST Z+,_PID
	;Получаем адрес заголовка задачи
	PUSH TEMP
	;TODO??? MOV TEMP,TASK_ID PID or _PID?
	MCALL _CORE5277_PROC_HEADER_GET
	POP TEMP
	;Проверяем на занятость ресурса
	CPI FLAGS,0x00
	BREQ _CORE5277_RES_WAIT_N_LOCK__NOT_LOCKED
	;Проверяем на заданный таймаут
	CPI TEMP,0x00
	BRNE PC+0x05
	CPI TEMP_L,0x00
	BRNE PC+0x03
	CPI TEMP_H,0x00
	BREQ _CORE5277_RES_WAIT_N_LOCK__ERROR

	;Записываем время выдерживание паузы
	STD Z+_CORE5277_TASK_TIMEOUT+0x00,TEMP_H
	STD Z+_CORE5277_TASK_TIMEOUT+0x01,TEMP_L
	STD Z+_CORE5277_TASK_TIMEOUT+0x02,TEMP
	;Восстанавливаем регистры
	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP FLAGS
	POP TEMP
	POP_Z
	;Помещаем в стек рабочие регистры задачи
	_CORE5277_MACRO__PUSH_RDS
	;Запоминаем SREG
	LDS r2,SREG
	LDI TEMP,0x80
	OR r2,TEMP															;Включаю флаг перываний
	PUSH r2																;Помещаем в стек регистр флагов
	;Получаем адрес заголовка задачи
;TODO??? MOV TEMP,TASK_ID PID or _PID?
	;Обновляем информацию о стеке задаи и возвращаемся в диспетчер
	LDI TEMP,_CORE5277_PROC_STATE_RES_WAIT
	MJMP _CORE5277_SUSPEND__BODY


_CORE5277_RES_WAIT_N_LOCK__QUEUE_FULL:
_CORE5277_RES_WAIT_N_LOCK__ERROR:
	MCALL CORE5277_DISPATCHER_UNLOCK
	CLZ
	RJMP _CORE5277_RES_WAIT_N_LOCK__END
_CORE5277_RES_WAIT_N_LOCK__NOT_LOCKED:
	MCALL CORE5277_DISPATCHER_UNLOCK
	SEZ
_CORE5277_RES_WAIT_N_LOCK__END:
	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP FLAGS
	POP TEMP
	POP_Z
	RET
.endif
