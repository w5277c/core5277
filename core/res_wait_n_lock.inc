;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;04.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RES_WAIT_N_LOCK
.else
.set DEF_C5_RES_WAIT_N_LOCK = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RES_WAIT_N_LOCK"
.endif

;--------------------------------------------------------
C5_RES_WAIT_N_LOCK:
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
	MCALL C5_DISPATCHER_LOCK
	LDI ZH,high(_C5_RESOURCE_QUEUE)
	LDI ZL,low(_C5_RESOURCE_QUEUE)
	LDI FLAGS,0x00
	LDI LOOP_CNTR,_C5_RES_QUEUE_SIZE
_C5_RES_WAIT_N_LOCK__QUEUE_LOOP:
	LD TEMP_EL,Z+														;RES_ID
	LD TEMP_EH,Z+														;PID
	;Не пустые элементы закончились
	CPI TEMP_EH,0x00
	BREQ _C5_RES_WAIT_N_LOCK__QUEUE_LOOP_END
	CP TEMP_EL,ACCUM
	BRNE _C5_RES_WAIT_N_LOCK__QUEUE_NEXT
	;Ресурс занят
	LDI FLAGS,0x01
	RJMP _C5_RES_WAIT_N_LOCK__QUEUE_LOOP_END
_C5_RES_WAIT_N_LOCK__QUEUE_NEXT:
	SUBI LOOP_CNTR,0x02
	BRNE _C5_RES_WAIT_N_LOCK__QUEUE_LOOP
	;Весь список пройден, в очереди нет места
	RJMP _C5_RES_WAIT_N_LOCK__QUEUE_FULL

_C5_RES_WAIT_N_LOCK__QUEUE_LOOP_END:
	;Возвращается на адрес свободной ячейки
	SUBI ZL,0x02
	SBCI ZH,0x00
	;Помещаем в очередь ид ресурса и ид задачи
	ST Z+,ACCUM
	ST Z+,_PID
	;Получаем адрес заголовка задачи
	PUSH TEMP
	;TODO??? MOV TEMP,TASK_ID PID or _PID?
	MCALL _C5_PROC_HEADER_GET
	POP TEMP
	;Проверяем на занятость ресурса
	CPI FLAGS,0x00
	BREQ _C5_RES_WAIT_N_LOCK__NOT_LOCKED
	;Проверяем на заданный таймаут
	CPI TEMP,0x00
	BRNE PC+0x05
	CPI TEMP_L,0x00
	BRNE PC+0x03
	CPI TEMP_H,0x00
	BREQ _C5_RES_WAIT_N_LOCK__ERROR

	;Записываем время окончания паузы
	LDS TEMP_EL,SREG
	CLI
	LDS TEMP_EH,_C5_UPTIME+0x04
	ADD TEMP_EH,TEMP
	STD Z+_C5_TASK_EXECTIME+0x02,TEMP_EH
	LDS TEMP_EH,_C5_UPTIME+0x03
	ADC TEMP_EH,TEMP_L
	STD Z+_C5_TASK_EXECTIME+0x01,TEMP_EH
	LDS TEMP_EH,_C5_UPTIME+0x02
	ADC TEMP_EH,TEMP_H
	STD Z+_C5_TASK_EXECTIME+0x00,TEMP_EH
	STS SREG,TEMP_EL

	;Восстанавливаем регистры
	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP FLAGS
	POP TEMP
	POP_Z
	;Помещаем в стек рабочие регистры задачи
	_C5_MACRO__PUSH_RDS
	;Запоминаем SREG
	LDS r2,SREG
	LDI TEMP,0x80
	OR r2,TEMP															;Включаю флаг перываний
	PUSH r2																;Помещаем в стек регистр флагов
	;Получаем адрес заголовка задачи
;TODO??? MOV TEMP,TASK_ID PID or _PID?
	;Обновляем информацию о стеке задаи и возвращаемся в диспетчер
	LDI TEMP,_C5_PROC_STATE_RES_WAIT
	MJMP _C5_SUSPEND__BODY


_C5_RES_WAIT_N_LOCK__QUEUE_FULL:
_C5_RES_WAIT_N_LOCK__ERROR:
	MCALL C5_DISPATCHER_UNLOCK
	CLZ
	RJMP _C5_RES_WAIT_N_LOCK__END
_C5_RES_WAIT_N_LOCK__NOT_LOCKED:
	MCALL C5_DISPATCHER_UNLOCK
	SEZ
_C5_RES_WAIT_N_LOCK__END:
	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP FLAGS
	POP TEMP
	POP_Z
	RET
.endif
