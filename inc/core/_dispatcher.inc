;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.02.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_WAIT_2MS
.else
.set DEF_C5_WAIT_2MS = 1

.include "./inc/io/log_corefault.inc"

;--------------------------------------------------------
_C5_DISPATCHER_EVENT:
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
	LDI ZH,high(_C5_TASKS_HEADER)
	LDI ZL,low(_C5_TASKS_HEADER)
_C5_DISPATCHER_EVENT__TICK_TASKS_LOOP:
	;Проверяем на активную задачу
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_TIME_WAIT
	BREQ PC+0x05
	CPI TEMP,_C5_PROC_STATE_TIMER_WAIT
	BREQ PC+0x03
	CPI TEMP,_C5_PROC_STATE_RES_WAIT
	BRNE _C5_DISPATCHER_EVENT__TICK_NEXT_TASK

	CLR ACCUM
	LDD TEMP,Z+_C5_TASK_TIMEOUT+0x02
	SUBI TEMP,0x01
	OR ACCUM,TEMP
	STD Z+_C5_TASK_TIMEOUT+0x02,TEMP
	LDD TEMP,Z+_C5_TASK_TIMEOUT+0x01
	SBCI TEMP,0x00
	OR ACCUM,TEMP
	STD Z+_C5_TASK_TIMEOUT+0x01,TEMP
	LDD TEMP,Z+_C5_TASK_TIMEOUT+0x00
	SBCI TEMP,0x00
	OR ACCUM,TEMP
	STD Z+_C5_TASK_TIMEOUT+0x00,TEMP

	CPI ACCUM,0x00
	BRNE _C5_DISPATCHER_EVENT__TICK_NEXT_TASK
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_C5_PROC_STATE_READY
	STD Z+_C5_PROC_STATE,TEMP

_C5_DISPATCHER_EVENT__TICK_NEXT_TASK:
	LDI TEMP,_C5_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_C5_TASKS_MAX_QNT
	BREQ PC+0x02
	RJMP _C5_DISPATCHER_EVENT__TICK_TASKS_LOOP

	;Цикл проверки на SUSPEND
	LDI LOOP_CNTR,0x00
	LDI ZH,high(_C5_TASKS_HEADER)
	LDI ZL,low(_C5_TASKS_HEADER)
_C5_DISPATCHER_EVENT__SUSPEND_TASKS_LOOP:
	;Проверяем на активную задачу
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_BUSY
	BRNE _C5_DISPATCHER_EVENT__SUSPEND_NEXT_TASK
	;Пропускаем проверку SUSPEND, если включен режим NO_SUSPEND
	LDD TEMP,Z+_C5_PROC_STATE
	SBRC TEMP,_C5_PROCST_OPT_NOSUSP
	RJMP _C5_DISPATCHER_EVENT__SUSPEND_NEXT_TASK

	;Сверяем дельту времени текущего и времени запуска(RESUME) задачи
	PUSH TEMP_H
	LDS TEMP_H,_C5_UPTIME+0x04
	LDD TEMP,Z+_C5_TASK_TIMESTAMP
	SUB TEMP_H,TEMP
	CPI TEMP_H,_C5_TASKS_ACTIVE_TIME
	POP TEMP_H
	BRCS _C5_DISPATCHER_EVENT__SUSPEND_NEXT_TASK

	MOV TEMP,_C5_DISPATCHER_LOCK_CNTR
	CPI TEMP,0x00
	BREQ _C5_DISPATCHER_EVENT__UNLOCKED
	LDI TEMP,(1<<_CFL_DISPATCHER_ORDER)
	OR _C5_COREFLAGS,TEMP
	RJMP _C5_DISPATCHER_EVENT__SKIP_TASKS
_C5_DISPATCHER_EVENT__UNLOCKED:

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
	POP r5																;вызов _C5_DISPATCHER_EVENT
	POP r5																;вызов _C5_DISPATCHER_EVENT
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

	LDI TEMP,_C5_PROC_STATE_READY
	MCALL _C5_SUSPEND__BODY
	RETI

_C5_DISPATCHER_EVENT__SUSPEND_NEXT_TASK:
	LDI TEMP,_C5_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_C5_TASKS_MAX_QNT
	BREQ PC+0x02
	RJMP _C5_DISPATCHER_EVENT__SUSPEND_TASKS_LOOP

_C5_DISPATCHER_EVENT__SKIP_TASKS:
	POP LOOP_CNTR
	POP ZL
	POP ZH
	RET


;TODO TIMER_MARK?
;--------------------------------------------------------
C5_WAIT_2MS:
;--------------------------------------------------------
;Ждем истечения времени с момента прошлого сна или
;с момента вызова C5_TIMER_MARK
;IN TEMP_H,TEMP_L,TEMP - время в 0.002s
;--------------------------------------------------------
	LDS r2,SREG															;Запоминаем регистр флагов
	MCALL C5_DISPATCHER_LOCK
	_C5_MACRO__PUSH_RDS										;Пмещаем в стек рабочие регистры задачи
	PUSH r2																;Помещаем в стек регистр флагов

	MOV PID,_PID														;Затираю PID, так как он уже записан в стек
	MCALL _C5_PROC_HEADER_GET

	;Записываем время выдерживание паузы
	STD Z+_C5_TASK_TIMEOUT+0x02,TEMP
	STD Z+_C5_TASK_TIMEOUT+0x01,TEMP_L
	STD Z+_C5_TASK_TIMEOUT+0x00,TEMP_H
	;Если значение нулевое, то задача сразу переводится в состояние READY
	OR TEMP,TEMP_H
	OR TEMP,TEMP_L
	CPI TEMP,0x00
	LDI TEMP,_C5_PROC_STATE_TIME_WAIT
	BRNE PC+0x02
	LDI TEMP,_C5_PROC_STATE_READY
	MCALL _C5_SUSPEND__BODY
	RET

;--------------------------------------------------------
C5_DISPATCHER_LOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,0xff
	CP _C5_DISPATCHER_LOCK_CNTR,TEMP
	BREQ PC+0x02
	INC _C5_DISPATCHER_LOCK_CNTR
	POP TEMP
	RET
;--------------------------------------------------------
C5_DISPATCHER_UNLOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,0x00
	CP _C5_DISPATCHER_LOCK_CNTR,TEMP
	POP TEMP
	BREQ _C5_DISPATCHER_UNLOCK__END

	SBRS _C5_COREFLAGS,_CFL_DISPATCHER_ORDER
	RJMP _C5_DISPATCHER_UNLOCK__DEC
	PUSH TEMP
	LDI TEMP,0x01
	CP _C5_DISPATCHER_LOCK_CNTR,TEMP
	POP TEMP
	BRNE _C5_DISPATCHER_UNLOCK__DEC
	;Отрабатываем SUSPEND логику
;	PUSH TEMP
;	LDI TEMP,~(1<<_CFL_DISPATCHER_ORDER)
;	AND _C5_COREFLAGS,TEMP
;	POP TEMP
;	MCALL C5_SUSPEND

_C5_DISPATCHER_UNLOCK__DEC:
	DEC _C5_DISPATCHER_LOCK_CNTR
_C5_DISPATCHER_UNLOCK__END:
	RET
.endif
