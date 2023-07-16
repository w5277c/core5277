;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.02.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;22.12.2020	w5277c@gmail.com			Исправлена работа TIMER-TASK
;31.01.2022	w5277c@gmail.com			Исправлено условие проверки таймаута
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_DISPATCHER
.else
.set DEF_C5_DISPATCHER = 1

;--------------------------------------------------------
_C5_DISPATCHER_EVENT:
;--------------------------------------------------------
;Очередная итерация диспетчера
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH LOOP_CNTR

	PUSH ACCUM
	;Цикл обновления таймингов
	LDI LOOP_CNTR,0x00
	LDI ZH,high(_C5_TASKS_HEADER)
	LDI ZL,low(_C5_TASKS_HEADER)
_C5_DISPATCHER_EVENT__TICK_TASKS_LOOP:
	;Проверяем на активную задачу
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_TIME_WAIT
	BREQ _C5_DISPATCHER_EVENT__CHECK_WAITTIME
	CPI TEMP,_C5_PROC_STATE_TIMER_WAIT
	BRNE _C5_DISPATCHER_EVENT__TICK_NEXT_TASK
_C5_DISPATCHER_EVENT__CHECK_WAITTIME:
	LDS ACCUM,_C5_UPTIME+0x02
	LDD TEMP,Z+_C5_TASK_EXECTIME+0x00
	CP ACCUM,TEMP
	BRCS _C5_DISPATCHER_EVENT__TICK_NEXT_TASK
	BRNE PC+0x0a
	LDS ACCUM,_C5_UPTIME+0x03
	LDD TEMP,Z+_C5_TASK_EXECTIME+0x01
	CP ACCUM,TEMP
	BRCS _C5_DISPATCHER_EVENT__TICK_NEXT_TASK
	BRNE PC+0x05
	LDS ACCUM,_C5_UPTIME+0x04
	LDD TEMP,Z+_C5_TASK_EXECTIME+0x02
	CP ACCUM,TEMP
	BRCS _C5_DISPATCHER_EVENT__TICK_NEXT_TASK

	;Проверяю и обновляю метку времени для следующего срабатывания таймера
	LDD TEMP,Z+_C5_PROC_STATE
	SBRS TEMP,C5_PROCID_OPT_TIMER
	RJMP _C5_DISPATCHER_EVENT__NO_TIMER
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_TIMER_WAIT
	BRNE _C5_DISPATCHER_EVENT__NO_TIMER
	;Записываем текущую временную метку
	PUSH_Y
	LDD YH,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD YL,Z+_C5_TASK_STACK_OFFSET+0x01
	SBIW YL,0x05
	PUSH ACCUM
	LDD TEMP,Y+0x00
	LDD ACCUM,Y+0x03
	ADD TEMP,ACCUM
	STD Y+0x00,TEMP
	LDD TEMP,Y+0x01
	LDD ACCUM,Y+0x04
	ADC TEMP,ACCUM
	STD Y+0x01,TEMP
	LDD TEMP,Y+0x02
	LDD ACCUM,Y+0x05
	ADC TEMP,ACCUM
	STD Y+0x02,TEMP
	POP ACCUM
	POP_Y
_C5_DISPATCHER_EVENT__NO_TIMER:
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_C5_PROC_STATE_READY
	STD Z+_C5_PROC_STATE,TEMP

_C5_DISPATCHER_EVENT__TICK_NEXT_TASK:
	ADIW ZL,_C5_TASK_HEADER_SIZE
	INC LOOP_CNTR
	CPI LOOP_CNTR,C5_TASKS_QNT
	BREQ PC+0x02
	RJMP _C5_DISPATCHER_EVENT__TICK_TASKS_LOOP
	POP ACCUM

	;Разрешаю прерывания - ключевой момент - дальнейший код жрет много времени, но нам никто не мешает его прерывать
	SEI

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

	CP _C5_DISPATCHER_LOCK_CNTR,C0x00
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
	;Следующие два байта - ICALL, оставляем их в стеке

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
	ADIW ZL,_C5_TASK_HEADER_SIZE
	INC LOOP_CNTR
	CPI LOOP_CNTR,C5_TASKS_QNT
	BREQ PC+0x02
	RJMP _C5_DISPATCHER_EVENT__SUSPEND_TASKS_LOOP

_C5_DISPATCHER_EVENT__SKIP_TASKS:
	POP LOOP_CNTR
	POP ZL
	POP ZH
	RET

;--------------------------------------------------------
C5_DISPATCHER_LOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	CP _C5_DISPATCHER_LOCK_CNTR,C0xff
	BREQ PC+0x02
	INC _C5_DISPATCHER_LOCK_CNTR
	RET
;--------------------------------------------------------
C5_DISPATCHER_UNLOCK:
;--------------------------------------------------------
;Блокировка диспетчера
;--------------------------------------------------------
	CP _C5_DISPATCHER_LOCK_CNTR,C0x00
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
