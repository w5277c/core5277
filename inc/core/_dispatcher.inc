;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;15.02.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_WAIT_2MS
.else
.set DEF_CORE5277_WAIT_2MS = 1

.include "./inc/io/log_corefault.inc"

;--------------------------------------------------------
_CORE5277_DISPATCHER_EVENT:
;--------------------------------------------------------
;Очередная итерация диспетчера
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH LOOP_CNTR

	;Разрешаю прерывания - ключевой момент - дальнейший код жрет много времени, но нам никто не мешает его прерывать
	SEI

	;Цикл обновления таймингов
	LDI LOOP_CNTR,0x00
	LDI ZH,high(_CORE5277_TASKS_HEADER)
	LDI ZL,low(_CORE5277_TASKS_HEADER)
_CORE5277_DISPATCHER_EVENT__TICK_TASKS_LOOP:
	;Проверяем на активную задачу
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_TIME_WAIT
	BREQ PC+0x03
	CPI TEMP,_CORE5277_PROC_STATE_RES_WAIT
	BRNE _CORE5277_DISPATCHER_EVENT__TICK_NEXT_TASK

	CLR ACCUM
	LDD TEMP,Z+_CORE5277_TASK_TIMEOUT+0x02
	SUBI TEMP,0x01
	OR ACCUM,TEMP
	STD Z+_CORE5277_TASK_TIMEOUT+0x02,TEMP
	LDD TEMP,Z+_CORE5277_TASK_TIMEOUT+0x01
	SBCI TEMP,0x00
	OR ACCUM,TEMP
	STD Z+_CORE5277_TASK_TIMEOUT+0x01,TEMP
	LDD TEMP,Z+_CORE5277_TASK_TIMEOUT+0x00
	SBCI TEMP,0x00
	OR ACCUM,TEMP
	STD Z+_CORE5277_TASK_TIMEOUT+0x00,TEMP

	CPI ACCUM,0x00
	BRNE _CORE5277_DISPATCHER_EVENT__TICK_NEXT_TASK
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_CORE5277_PROC_STATE_READY
	STD Z+_CORE5277_PROC_STATE,TEMP
_CORE5277_DISPATCHER_EVENT__TICK_NEXT_TASK:
	LDI TEMP,_CORE5277_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_CORE5277_TASKS_MAX_QNT
	BREQ PC+0x02
	RJMP _CORE5277_DISPATCHER_EVENT__TICK_TASKS_LOOP

	;Цикл проверки на SUSPEND
	LDI LOOP_CNTR,0x00
	LDI ZH,high(_CORE5277_TASKS_HEADER)
	LDI ZL,low(_CORE5277_TASKS_HEADER)
_CORE5277_DISPATCHER_EVENT__SUSPEND_TASKS_LOOP:
	;Проверяем на активную задачу
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_BUSY
	BRNE _CORE5277_DISPATCHER_EVENT__SUSPEND_NEXT_TASK
	;Пропускаем проверку SUSPEND, если включен режим NO_SUSPEND
	LDD TEMP,Z+_CORE5277_PROC_STATE
	SBRC TEMP,_CORE5277_PROCST_OPT_NOSUSP
	RJMP _CORE5277_DISPATCHER_EVENT__SUSPEND_NEXT_TASK

	;Сверяем дельту времени текущего и времени запуска(RESUME) задачи
	PUSH TEMP_H
	LDS TEMP_H,_CORE5277_UPTIME+0x04
	LDD TEMP,Z+_CORE5277_TASK_TIMESTAMP
	SUB TEMP_H,TEMP
	CPI TEMP_H,_CORE5277_TASKS_ACTIVE_TIME
	POP TEMP_H
	BRCS _CORE5277_DISPATCHER_EVENT__SUSPEND_NEXT_TASK

	MOV TEMP,_CORE5277_DISPATCHER_LOCK_CNTR
	CPI TEMP,0x00
	BREQ _CORE5277_DISPATCHER_EVENT__UNLOCKED
	LDI TEMP,(1<<_CFL_DISPATCHER_ORDER)
	OR _CORE5277_COREFLAGS,TEMP
	RJMP _CORE5277_DISPATCHER_EVENT__SKIP_TASKS
_CORE5277_DISPATCHER_EVENT__UNLOCKED:

	;Теоретически здесь не нужно блокировать перывания
	;Нужно просто ждать освобождения повторно вызываемго драйвера
	;CLI
	;Далее задачи не проверяются, чтобы не занимать долго процессор данным прерыванием

	MOV _PID,LOOP_CNTR
	;Запоминаем все, что хранится в стеке
	;Т.е. выгружаю из стека все, что было записано при отработке прерывания, остались только данные задачи
	POP r2																;LOOP_CNTR
	POP r3																;ZL
	POP r4																;ZH
	POP r5																;вызов _CORE5277_DISPATCHER_EVENT
	POP r5																;вызов _CORE5277_DISPATCHER_EVENT
	POP r5																;SREG
	POP r6																;TEMP
	POP r7																;ICALL
	POP r8																;ICALL

	;Восстанавливаю стек задачи
	;В r7,r8 храниться адрес возврата на задачу, возвращаем ее обратно в стек
	PUSH r8
	PUSH r7

	;Помещаю в стек регистры задачи
	PUSH r16
	PUSH r17
	PUSH r6
	PUSH r19
	PUSH r20
	PUSH r2
	PUSH r22
	PUSH r23
	PUSH r24
	PUSH r25
	PUSH r26
	PUSH r27
	PUSH r28
	PUSH r29
	PUSH r3
	PUSH r4
	;Помещаю в стек регистр флагов
	PUSH r5

	LDI TEMP,_CORE5277_PROC_STATE_READY
	MCALL _CORE5277_SUSPEND__BODY
	RETI

_CORE5277_DISPATCHER_EVENT__SUSPEND_NEXT_TASK:
	LDI TEMP,_CORE5277_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_CORE5277_TASKS_MAX_QNT
	BREQ PC+0x02
	RJMP _CORE5277_DISPATCHER_EVENT__SUSPEND_TASKS_LOOP

_CORE5277_DISPATCHER_EVENT__SKIP_TASKS:
	POP LOOP_CNTR
	POP ZL
	POP ZH
	RET

;--------------------------------------------------------
CORE5277_WAIT_2MS:
;--------------------------------------------------------
;Ждем истечения времени с момента прошлого сна или
;с момента вызова CORE5277_TIMER_MARK
;IN TEMP_H,TEMP_L,TEMP - время в 0.002s
;--------------------------------------------------------
	LDS r2,SREG															;Запоминаем регистр флагов
	MCALL CORE5277_DISPATCHER_LOCK
	_CORE5277_MACRO__PUSH_RDS										;Пмещаем в стек рабочие регистры задачи
	PUSH r2																;Помещаем в стек регистр флагов

	MOV PID,_PID														;Затираю PID, так как он уже записан в стек
	MCALL _CORE5277_PROC_HEADER_GET

	;Записываем время выдерживание паузы
	STD Z+_CORE5277_TASK_TIMEOUT+0x02,TEMP
	STD Z+_CORE5277_TASK_TIMEOUT+0x01,TEMP_L
	STD Z+_CORE5277_TASK_TIMEOUT+0x00,TEMP_H
	;Если значение нулевое, то задача сразу переводится в состояние READY
	OR TEMP,TEMP_H
	OR TEMP,TEMP_L
	CPI TEMP,0x00
	LDI TEMP,_CORE5277_PROC_STATE_TIME_WAIT
	BRNE PC+0x02
	LDI TEMP,_CORE5277_PROC_STATE_READY
	MCALL _CORE5277_SUSPEND__BODY
	RET

;--------------------------------------------------------
CORE5277_DISPATCHER_LOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,0xff
	CP _CORE5277_DISPATCHER_LOCK_CNTR,TEMP
	BREQ PC+0x02
	INC _CORE5277_DISPATCHER_LOCK_CNTR
	POP TEMP
	RET
;--------------------------------------------------------
CORE5277_DISPATCHER_UNLOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,0x00
	CP _CORE5277_DISPATCHER_LOCK_CNTR,TEMP
	POP TEMP
	BREQ _CORE5277_DISPATCHER_UNLOCK__END

	SBRS _CORE5277_COREFLAGS,_CFL_DISPATCHER_ORDER
	RJMP _CORE5277_DISPATCHER_UNLOCK__DEC
	PUSH TEMP
	LDI TEMP,0x01
	CP _CORE5277_DISPATCHER_LOCK_CNTR,TEMP
	POP TEMP
	BRNE _CORE5277_DISPATCHER_UNLOCK__DEC
	;Отрабатываем SUSPEND логику
;	PUSH TEMP
;	LDI TEMP,~(1<<_CFL_DISPATCHER_ORDER)
;	AND _CORE5277_COREFLAGS,TEMP
;	POP TEMP
;	MCALL CORE5277_SUSPEND

_CORE5277_DISPATCHER_UNLOCK__DEC:
	DEC _CORE5277_DISPATCHER_LOCK_CNTR
_CORE5277_DISPATCHER_UNLOCK__END:
	RET
.endif
