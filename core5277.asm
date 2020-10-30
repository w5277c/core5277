;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Тут просто дохрена изменений
;09.08.2020  w5277c@gmail.com        Тут просто дохрена изменений
;06.09.2020  w5277c@gmail.com        Восстановлены механизмы многопоточности
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;28.10.2020  w5277c@gmail.com        Багфикс программных таймеров
;-----------------------------------------------------------------------------------------------------------------------
;Задача имеет свой собственный стек, драйвер нет. Задача вызывает драйвер, диспетчер оперирует задачами.
;Стеки задач и выделяемая память динамические, сверху стеки задач, свободное пространство, затем выделяемая память.
;Для завершения задачи достаточно вызвать RET
;Драйвер не имеет персональных регистров и стека как у задачи, выполнение кода драйвера обеспечивается прерываниями или вызовами
;из задач или других драйверов.
;Идентификатор задачи или драйвера назначается разработчиком
;Выполнение процедуры драйвера может быть первано диспетчером, так как работает в инстантсе задачи.
;Все процедуры вызываемые прерываниями должны быт максимально короткими(иначе могут негативно влиять друг на друга)
;Ядро использует один из таймеров, обычно TIMER0, остальные - свободны. Стоит использовать только в исключительных ситуациях,
;т.к. использование не универсально и не имеет отношения к ядру
;В пределах процедуры можно использовать RJMP, в остальных случаях макросы MJMP и MCALL
;Острожно с записями типа 'BRTC PC+0x03' макрос MJMP и MCALL занимает 3 слова(смещение +0x03)
;Программные UART'ы есть смысл сделать только два, где RX'ы подключаются к INT0 и INT1
;TODO может быть стоит вызовы сделать короче ? C5 например.
;find -type f -name \*.asm -exec sed -i -r 's/C5/C5/g' {} \;

;-----------------------------------------------------------------------------------------------------------------------
	.SET AVRA										= 0x00			;0x01 - выводит при сборке размеры блоков и некоторые смещения

;---PRCO-STATES------------------------------------------;Состояния задачи (PROC-STATES-младший ниббл, PROC-MODE-старший ниббл)
	.EQU	_C5_PROC_STATE_ABSENT				= 0x00			;Нет задачи
	.EQU	_C5_PROC_STATE_GHOST					= 0x01			;Инициализируется или деактивация
	.EQU	_C5_PROC_STATE_BUSY					= 0x02			;Выполняется
	.EQU	_C5_PROC_STATE_TIME_WAIT			= 0x03			;Ждет истечения времени(только для задач)
	.EQU	_C5_PROC_STATE_RES_WAIT				= 0x04			;Ждет освобождения ресурса(только для задач)
	.EQU	_C5_PROC_STATE_READY					= 0x05			;Готова к выполнению
;---PROC-ID-OPTIONS--------------------------------------;Не изменяемые опции процесса размещаемые в идентификаторе процесса
	.EQU	C5_PROCID_OPT_NOSUSP					= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	C5_PROCID_OPT_RESERV1				= 0x05			;Зарезервировано
	.EQU	C5_PROCID_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	C5_PROCID_OPT_DRV						= 0x07			;Драйвер
;---PROC-STATE-OPTIONS-----------------------------------;Опции процесса, размещаемые в статусе процесса(старшйи ниббл)
	.EQU	_C5_PROCST_OPT_NOSUSP				= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	_C5_PROCST_OPT_RESERV1				= 0x05			;Зарезервировано
	.EQU	_C5_PROCST_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	_C5_PROCST_OPT_RESERV3				= 0x07			;Зарезервировано

;---TIMER-STRUCTURE---------------------------------------(5b);
	.EQU	_C5_TIMER_STRUCT_SIZE				= 0x05			;Длина блока таймера
	.EQU	_C5_TIMER_PROC_ID						= 0x00			;1b - ид процедуры(0-..., 0xff - не исп.)
	.EQU	_C5_TIMER_HANDLER						= 0x01			;2b - адрес обработчика
	.EQU	_C5_TIMER_CNTR							= 0x03			;1b - счетчик таймера
	.EQU	_C5_TIMER_DEFAULT						= 0x04			;1b - порог 0x01-0x7f, 7-бит:период отсчета,H:0.002s,L:0.00005s)

;---PROC-STRUCTURE---------------------------------------;Заголовок процедуры(6B-драйвер, 11B-задача)
	.EQU	_C5_PROC_STATE							= 0x00			;1B - статус процедуры(младший ниббл) и опции(старший ниббл)
	.EQU	_C5_PROC_RAM_OFFSET					= 0x01			;2B - адрес начала выделенной памяти
	.EQU	_C5_PROC_RAM_SIZE						= 0x03			;1B - размер выделенной памяти
	.EQU	_C5_DRIVER_OFFSET						= 0x04			;2B - адрес процедуры драйвера или
	.EQU	_C5_TASK_STACK_OFFSET				= 0x04			;2B - адрес начала стека задачи
	.EQU	_C5_TASK_STACK_SIZE					= 0x06			;1B - размер стека задачи
	.EQU	_C5_TASK_TIMESTAMP					= 0x07			;1B - мекта времени переключения на задачу
	.EQU	_C5_TASK_TIMEOUT						= 0x08			;3B - время ожидания задачи(~9 часов максимум, тик в 2мс)

;---CONSTANTS--------------------------------------------
	.EQU	_C5_DRIVER_HEADER_SIZE				= 0x06			;Размер заголовка драйвера
	.EQU	_C5_TASK_HEADER_SIZE					= 0x0b			;Размер заголовка задачи
.if TIMERS_SPEED == TIMERS_SPEED_25NS
	.EQU	_C5_TIMER_PERIOD						= 0x50			;80, т.е. 0.000025*80=0.002=2мс
.else
	.EQU	_C5_TIMER_PERIOD						= 0x28			;40, т.е. 0.000050*40=0.002=2мс
.endif
	.EQU	_C5_TASK_STACK_DEF_SIZE				= 0x13			;Размер стека задачи при создании (16 регистров + SREG + точка возврата)
	.EQU	_C5_TASKS_ACTIVE_TIME				= 0x01			;Время работы задачи 1*2 = 2мс
	.EQU	_C5_IRV_TABLE_SIZE					= 0x4b			;Размер таблицы прерываний
	;---CORE-FAULTS---
	.EQU	_C5_FAULT_HUGE_STACK					= 0x01			;Стек залез на границу между стеком и выделяемой памяти
	.EQU	_C5_FAULT_HUGE_TASK_STACK			= 0x02			;Размер стека задачи более 255 байт
	;---CORE-RESOURCES---											;Идентфикаторы ресурсов, 0-127 отданы пользователю, старшие - ядру
	.EQU	C5_RES_TIMER_B							= 0xFE			;Таймер B
	.EQU	C5_RES_ADC								= 0xFD			;ADC
	;---
;---CORE-FLAGS-------------------------------------------
	.EQU	_CFL_TIMER_B_USE						= 0x00			;Флаг запущенного таймера B
	.EQU	_CFL_TIMER_UART_CORRECTION			= 0x01			;Корректировка таймера для соответствия частотам UART
	.EQU	_CFL_DISPATCHER_ORDER				= 0x02			;Заявка на отработку диспетчера(формируется когда диспетчер заблокирован)

;---RAM-OFFSETS------------------------------------------
	.EQU	_C5_UPTIME								= 0x100			;5B - отсчет времени (273 года)
	.EQU	_C5_CORE_STACK							= 0x105			;2B - хранит смещение стека ядра
	.EQU	_C5_TOP_OF_STACK						= 0x107			;2B - хранит вершину стека
	.EQU	_C5_TOP_OF_FREE_RAM					= 0x109			;2B - хранит вершину свободной памяти
	.EQU	_C5_FLAGS								= 0x10b			;4B - флагов (32 флага)
	.EQU	_C5_MAIN_TIMER_CNTR					= 0x10f			;1B - счетчик таймера ядра (каждые 0.000050с)
	.EQU	_C5_TIMERS								= 0x110			;XB - таблица таймеров(TIMERS*5)
																			;TIMERS*5
	.EQU	_C5_IR_VECTORS_TABLE					= _C5_TIMERS+TIMERS*5
																			;XB - таблица прерываний (_C5_IR_QNT*3 для каждгого прерывания, исключая RESET)
	.EQU	_C5_DRIVERS_HEADER					= _C5_IR_VECTORS_TABLE+_C5_IR_QNT*3
																			;XB - заголовки драйвера (X=_C5_DRIVERS_MAX_QNT*_C5_DRIVER_HEADER_SIZE)
	.EQU	_C5_TASKS_HEADER						= _C5_DRIVERS_HEADER+_C5_DRIVERS_MAX_QNT*_C5_DRIVER_HEADER_SIZE
																			;XB - заголовки задач (X=_C5_TASKS_MAX_QNT*_C5_TASK_HEADER_SIZE)
	.EQU	_C5_RESOURCE_QUEUE					= _C5_TASKS_HEADER+_C5_TASKS_MAX_QNT*_C5_TASK_HEADER_SIZE
																			;XB - очередь задач к ресурсам(1b - RES_ID, 1b - TASK_ID...)
	.EQU	_C5_FREE_RAM							= _C5_RESOURCE_QUEUE+_C5_RES_QUEUE_SIZE
	.EQU	C5_BUFFER								= RAMEND-(_C5_STACK_SIZE+BUFFER_SIZE)
	.EQU	_C5_STACK_END							= C5_BUFFER	;XB - стек ядра, далее буфер, стек задач и выделяемая память
														;TODO проверить что стек не перетирает первый байт буфера
.IF AVRA == 0x01
	.MESSAGE "########  MAIN TIMER SPEED:",TIMERS_SPEED
	.IF BUFFER_SIZE > 0x00
		.MESSAGE "######## BUFFER SIZE:",BUFFER_SIZE
	.ENDIF
	.MESSAGE "######## AVAILABLE RAM:",_C5_STACK_END-(_C5_FREE_RAM+_C5_RAM_BORDER_SIZE)
	.MESSAGE "######## DRIVERS HEADERS OFFSET:",_C5_DRIVERS_HEADER
	.MESSAGE "######## TASKS HEADERS OFFSET:",_C5_TASKS_HEADER
	.MESSAGE "######## FREE RAM OFFSET:",_C5_FREE_RAM
.ENDIF

;---REGISTERS--------------------------------------------;r0-r14 выделено под нужды ядра
	.DEF	_RESULT_L								= r0
	.DEF	_RESULT_H								= r1
	;...
	.DEF	_C5_DISPATCHER_LOCK_CNTR			= r9				;Счетчик вложенных блокировок диспетчера
	.DEF	_C5_DRIVER_EXEC_ZH					= r10
	.DEF	_C5_DRIVER_EXEC_ZL					= r11
	.DEF	_C5_TEMP_H								= r12
	.DEF	_C5_TEMP_L								= r13
	.DEF	_C5_COREFLAGS							= r14				;флаги ядра
	.DEF	_PID										= r15				;ид корневого процесса(обычно задачи)

	.DEF	TEMP_L									= r16				;Младший регистр общего назначения
	.DEF	TEMP_H									= r17				;Старший регистр общего назначения
	.DEF	TEMP										= r18				;Регистр общего назначения
	.DEF	TEMP_EL									= r19				;Младший расширенный регистр общего назначения
	.DEF	TEMP_EH									= r20				;Старший расширенный регистр общего назначения
	.DEF	LOOP_CNTR								= r21				;Регистр счета циклов
	.DEF	FLAGS										= r22				;Регистр флагов
	.DEF	TRY_CNTR									= r23				;Счетчик ошибок
	.DEF	ACCUM										= r24				;Аккумулятор
	.DEF	PID										= r25				;ид процедуры
	.DEF	XL											= r26
	.DEF	XH											= r27
	.DEF	YL											= r28
	.DEF	YH											= r29
	.DEF	ZL											= r30
	.DEF	ZH											= r31

.MACRO PUSH_X
	PUSH XH
	PUSH XL
.ENDMACRO
.MACRO POP_X
	POP XL
	POP XH
.ENDMACRO
.MACRO PUSH_Y
	PUSH YH
	PUSH YL
.ENDMACRO
.MACRO POP_Y
	POP YL
	POP YH
.ENDMACRO
.MACRO PUSH_Z
	PUSH ZH
	PUSH ZL
.ENDMACRO
.MACRO POP_Z
	POP ZL
	POP ZH
.ENDMACRO

.INCLUDE	"./inc/io/log_init.inc"
.INCLUDE	"./inc/io/log_romstr.inc"
.INCLUDE	"./inc/io/log_tasksdump.inc"
.INCLUDE "./inc/io/log_corefault.inc"
.INCLUDE	"./inc/mem/ram_copy8.inc"
.INCLUDE	"./inc/mem/ram_fill16.inc"
.INCLUDE	"./inc/mem/ram_fill8.inc"
.INCLUDE "./inc/mem/ram_copy8_desc.inc"
.INCLUDE "./inc/mem/ram_copy16_desc.inc"
.INCLUDE "./inc/conv/bitnum_to_num.inc"

.MACRO _C5_MACRO__PUSH_RDS
	PUSH r16
	PUSH r17
	PUSH r18
	PUSH r19
	PUSH r20
	PUSH r21
	PUSH r22
	PUSH r23
	PUSH r24
	PUSH r25
	PUSH r26
	PUSH r27
	PUSH r28
	PUSH r29
	PUSH r30
	PUSH r31
.ENDMACRO

.MACRO _C5_MACRO__POP_RDS
	POP r31
	POP r30
	POP r29
	POP r28
	POP r27
	POP r26
	POP r25
	POP r24
	POP r23
	POP r22
	POP r21
	POP r20
	POP r19
	POP r18
	POP r17
	POP r16
.ENDMACRO

;---OPTIONS-ANALYZE--------------------------------------
.IFDEF LOGGING_PORT
	.MESSAGE "######## LOGGING ENABLED ########"
	LOGSTR_CORE:
	.db   0x0d,0x0a,"core5277_v0.2.7",0x0d,0x0a,0x00
	.INCLUDE "./inc/io/log_init.inc"
.ELSE
	.MESSAGE "######## LOGGING DISABLED ########"
	.INCLUDE "./inc/io/log_fake.inc"
.ENDIF
.IF REALTIME == 1
	.MESSAGE "######## DISPATCHER ENABLED ########"
	.INCLUDE "./inc/core/_dispatcher.inc"
.ELSE
	.MESSAGE "######## DISPATCHER DISABLED ########"
	.INCLUDE "./inc/core/wait_2ms.inc"
C5_DISPATCHER_LOCK:
C5_DISPATCHER_UNLOCK:
	RET
.ENDIF

.IF TIMERS > 0x00
	.MESSAGE "######## TIMERS ENABLED ########"
	.INCLUDE "./inc/core/timer_set.inc"
	.INCLUDE "./inc/core/timer_set_period.inc"
	.INCLUDE "./inc/core/timer_start.inc"
	.INCLUDE "./inc/core/timer_stop.inc"
.ELSE
	.MESSAGE "######## TIMERS DISABLED ########"
	C5_TIMER_SET:
	C5_TIMER_START:
	C5_TIMER_STOP:
		RET
.ENDIF

;========================================================================
;===Обработка прерываний=================================================
;========================================================================
;--------------------------------------------------------
_C5_IR:
;--------------------------------------------------------
;Обработчик всех прерываний
;--------------------------------------------------------
	;Получаем адрес по которому определим тип прерывания
	POP _C5_TEMP_H
	POP _C5_TEMP_L

	;Сохраняем регистры
	PUSH TEMP
	LDS TEMP,SREG
	CLI
	PUSH TEMP
	PUSH_Z
	PUSH PID
	PUSH ACCUM
	;Находим смещение в таблице прерываний _C5_IR_VECTORS_TABLE
	LDI ZH,high(_C5_IR_VECTORS_TABLE)
	LDI ZL,low(_C5_IR_VECTORS_TABLE)

	_CORRE5277_IR_OFFSET_CORRECTOR

	MOV ACCUM,_C5_TEMP_L												;Номер строки в таблице перывания
	DEC _C5_TEMP_L														;Первая запись в таблице прерываний не используется
	BREQ PC+0x04
	MOV TEMP,_C5_TEMP_L												;Умножаю на 3(1 байт - PID, 2 байта - адрес перехода)
	LSL _C5_TEMP_L
	ADD _C5_TEMP_L,TEMP

	ADD ZL,_C5_TEMP_L
	LD PID,Z+
	CPI PID,0xff
	BREQ _C5_IR__END

	LD TEMP,Z+
	LD ZL,Z
	MOV ZH,TEMP
	;Корневой процесс - текущий и единственный
	PUSH _PID
	MOV _PID,PID
	;Переходим если адрес указан(в ACCUM номер строки таблицы прерываний)
	ICALL																	;Возвращаться надо по RET(не RETI), TEMP и SREG сохранять не нужно
	POP _PID
_C5_IR__END:
	;Восстанавливаю регистры и выхожу
	POP ACCUM
	POP PID
	POP_Z
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;--------------------------------------------------------
_C5_TIMER_A_IR:
;--------------------------------------------------------
;Обработчик прерывания таймера
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP
	_CORRE5277_TIMERA_CORRECTOR
	PUSH_Z
.IF TIMERS > 0x00
	PUSH LOOP_CNTR
	PUSH PID
	PUSH_Y
.ENDIF

.IF TIMERS > 0x00
	LDI YH,high(_C5_TIMERS)
	LDI YL,low(_C5_TIMERS)
	LDI LOOP_CNTR,TIMERS
_C5_TIMER_A_IR__HF_LOOP:
	LDD PID,Y+_C5_TIMER_PROC_ID
	CPI PID,0xff
	BREQ _C5_TIMER_A_IR__HF_NEXT
	LDD ACCUM,Y+_C5_TIMER_DEFAULT
	SBRC ACCUM,0x07
	RJMP _C5_TIMER_A_IR__HF_NEXT
	LDD TEMP,Y+_C5_TIMER_CNTR
	DEC TEMP
	BRNE _C5_TIMER_IR__DISPATCHER__TIMER_HF_SKIP
	LDD ZH,Y+_C5_TIMER_HANDLER+0x00
	LDD ZL,Y+_C5_TIMER_HANDLER+0x01
	ICALL
	MOV TEMP,ACCUM
	ANDI TEMP,0x7f
_C5_TIMER_IR__DISPATCHER__TIMER_HF_SKIP:
	STD Y+_C5_TIMER_CNTR,TEMP
_C5_TIMER_A_IR__HF_NEXT:
	ADIW YL,0x05
	DEC LOOP_CNTR
	BRNE _C5_TIMER_A_IR__HF_LOOP
.ENDIF
_C5_TIMER_IR__DISPATCHER:

	;14t max
	LDS TEMP,_C5_MAIN_TIMER_CNTR									;2
	DEC TEMP																;1
	STS _C5_MAIN_TIMER_CNTR,TEMP									;2
	BRNE _C5_TIMER_A_IR__END								;1/2 + 1
	LDI TEMP,_C5_TIMER_PERIOD										;1
	STS _C5_MAIN_TIMER_CNTR,TEMP									;2
	;Тик 1 кванта
	LDI ZL,0x01
	LDS ZH,_C5_UPTIME+0x04
	ADD ZH,ZL
	STS _C5_UPTIME+0x04,ZH
	LDI ZL,0x00
	LDS ZH,_C5_UPTIME+0x03
	ADC ZH,ZL
	STS _C5_UPTIME+0x03,ZH
	LDS ZH,_C5_UPTIME+0x02
	ADC ZH,ZL
	STS _C5_UPTIME+0x02,ZH
	LDS ZH,_C5_UPTIME+0x01
	ADC ZH,ZL
	STS _C5_UPTIME+0x01,ZH
	LDS ZH,_C5_UPTIME+0x00
	ADC ZH,ZL
	STS _C5_UPTIME+0x00,ZH

.IF TIMERS > 0x00
	LDI YH,high(_C5_TIMERS)
	LDI YL,low(_C5_TIMERS)
	LDI LOOP_CNTR,TIMERS
_C5_TIMER_A_IR__LF_LOOP:
	LDD PID,Y+_C5_TIMER_PROC_ID
	CPI PID,0xff
	BREQ _C5_TIMER_A_IR__LF_NEXT
	LDD ACCUM,Y+_C5_TIMER_DEFAULT
	SBRS ACCUM,0x07
	RJMP _C5_TIMER_A_IR__LF_NEXT
	LDD TEMP,Y+_C5_TIMER_CNTR
	DEC TEMP
	BRNE _C5_TIMER_IR__DISPATCHER__TIMER_LF_SKIP
	LDD ZH,Y+_C5_TIMER_HANDLER+0x00
	LDD ZL,Y+_C5_TIMER_HANDLER+0x01
	ICALL
	MOV TEMP,ACCUM
	ANDI TEMP,0x7f
_C5_TIMER_IR__DISPATCHER__TIMER_LF_SKIP:
	STD Y+_C5_TIMER_CNTR,TEMP
_C5_TIMER_A_IR__LF_NEXT:
	ADIW YL,0x05
	DEC LOOP_CNTR
	BRNE _C5_TIMER_A_IR__LF_LOOP
.ENDIF

.IF REALTIME == 1
	MCALL _C5_DISPATCHER_EVENT										;3+4
.ENDIF
_C5_TIMER_A_IR__END:
.IF TIMERS > 0x00
	POP_Y
	POP PID
	POP LOOP_CNTR
.ENDIF
	POP_Z
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;========================================================================
;===Базовые процедуры ядра===============================================
;========================================================================
;--------------------------------------------------------
C5_INIT:
;--------------------------------------------------------
;Инициализация ядра
;--------------------------------------------------------
	;Запрещение прерываний
	CLI

	;Инициализация логирования
	MCALL C5_LOG_INIT

	MCALL C5_LOG_TASKSDUMP
	;Сбрасываем флаги причины сброса
	CLR TEMP
	STS MCUSR,TEMP

	;Заполнение всей памяти 0xF7 значением для(диагностики)
	LDI TEMP,0xF7
	LDI TEMP_H,high(RAMEND-0x010f)									;RAM_SIZE-16(stack)
	LDI TEMP_L,low(RAMEND-0x010f)
	LDI XH,0x01
	LDI XL,0x00
	MCALL C5_RAM_FILL16

	;Чистка таблицы прерываний
	LDI XL,low(_C5_IR_VECTORS_TABLE)
	LDI XH,high(_C5_IR_VECTORS_TABLE)
	LDI TEMP,0xff
	LDI LOOP_CNTR,_C5_IRV_TABLE_SIZE								;IV_VECTORS_TABLE
	MCALL C5_RAM_FILL8
	;Чистка очереди
	LDI XH,high(_C5_RESOURCE_QUEUE)
	LDI XL,low(_C5_RESOURCE_QUEUE)
	LDI LOOP_CNTR,_C5_RES_QUEUE_SIZE
	LDI TEMP,0xff
	MCALL C5_RAM_FILL8

	;Чистка переменных в памяти
	CLR _C5_COREFLAGS
	;Сброс счетчика времени
	LDI XH,high(_C5_UPTIME)
	LDI XL,low(_C5_UPTIME)
	LDI LOOP_CNTR,0x05
	LDI TEMP,0x00
	MCALL C5_RAM_FILL8

	;Задаю вершину стека
	LDI XH,high(_C5_STACK_END)
	LDI XL,low(_C5_STACK_END)
	STS _C5_TOP_OF_STACK+0x00,XH
	STS _C5_TOP_OF_STACK+0x01,XL
	;Задаю вершину свободной памяти
	LDI XH,high(_C5_FREE_RAM)
	LDI XL,low(_C5_FREE_RAM)
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL

	;Чистка флагов
	CLR TEMP
	STS _C5_FLAGS+0x00,TEMP
	STS _C5_FLAGS+0x01,TEMP
	STS _C5_FLAGS+0x02,TEMP
	STS _C5_FLAGS+0x03,TEMP
	STS _C5_FLAGS+0x04,TEMP
	STS _C5_FLAGS+0x05,TEMP
	STS _C5_FLAGS+0x06,TEMP
	STS _C5_FLAGS+0x07,TEMP

	;Задаем счетчик основному таймеру
	LDI TEMP,_C5_TIMER_PERIOD
	STS _C5_MAIN_TIMER_CNTR,TEMP
.IF TIMERS > 0x00
	;Установка таймеров как не исполльзуемых
	LDI XH,high(_C5_TIMERS)
	LDI XL,low(_C5_TIMERS)
	LDI LOOP_CNTR,TIMERS*0x05
	LDI TEMP,0xff
	MCALL C5_RAM_FILL8
.ENDIF

	;Чистка заголовков драйверов
	LDI XH,high(_C5_DRIVERS_HEADER)
	LDI XL,low(_C5_DRIVERS_HEADER)
	LDI TEMP_H,high(_C5_DRIVER_HEADER_SIZE*_C5_DRIVERS_MAX_QNT)
	LDI TEMP_L,low(_C5_DRIVER_HEADER_SIZE*_C5_DRIVERS_MAX_QNT)
	LDI TEMP,_C5_PROC_STATE_ABSENT
	MCALL C5_RAM_FILL16
	;Чистка заголовков задач
	LDI XH,high(_C5_TASKS_HEADER)
	LDI XL,low(_C5_TASKS_HEADER)
	LDI TEMP_H,high(_C5_TASK_HEADER_SIZE*_C5_TASKS_MAX_QNT)
	LDI TEMP_L,low(_C5_TASK_HEADER_SIZE*_C5_TASKS_MAX_QNT)
	LDI TEMP,_C5_PROC_STATE_ABSENT
	MCALL C5_RAM_FILL16

	;Чистка регистров
	CLR _C5_DISPATCHER_LOCK_CNTR

	;Инициализирую таймер (0.000050s - 800 тактов)
	_C5_TIMERA_INIT
	RET

;--------------------------------------------------------
C5_IR_VECTOR_SET:
;--------------------------------------------------------
;Устанавливаем адрес перехода для прерывания
;IN: PID - ид задачи/драйвера,
;ACCUM - номер прерывания (1-25), TEMP_H,TEMP_L -
;адрес перехода (0 - не использовать перерывание)
;--------------------------------------------------------
	PUSH_Z
	LDS ZL,SREG
	PUSH ZL
	PUSH ACCUM

	CPI ACCUM,0x00
	BREQ _C5_IR_VECTOR_SET__END

	CLI
	DEC ACCUM
	MOV ZL,ACCUM
	LSL ACCUM
	ADD ACCUM,ZL
	LDI ZH,high(_C5_IR_VECTORS_TABLE)
	LDI ZL,low(_C5_IR_VECTORS_TABLE)
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	STD Z+0x00,PID
	STD Z+0x01,TEMP_H
	STD Z+0x02,TEMP_L

_C5_IR_VECTOR_SET__END:
	POP ACCUM
	POP ZL
	STS SREG,ZL
	POP_Z
	RET

;--------------------------------------------------------
C5_SOFT_UART_MODE_SET:
;--------------------------------------------------------
;Установка режима программного UART
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,(1<<_CFL_TIMER_UART_CORRECTION)
	OR _C5_COREFLAGS,TEMP
	POP TEMP
	RET
;--------------------------------------------------------
C5_SOFT_UART_MODE_RESET:
;--------------------------------------------------------
;Снятие режима программного UART
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,~((1<<_CFL_TIMER_UART_CORRECTION))
	AND _C5_COREFLAGS,TEMP
	POP TEMP
	RET

;--------------------------------------------------------
C5_UPTIME_WRITE:
;--------------------------------------------------------
;Записываем в память 5 байт UPTIME
;IN: Y - адрес для записи
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDS TEMP,_C5_UPTIME+0x00
	STD Y+0x00,TEMP
	LDS TEMP,_C5_UPTIME+0x01
	STD Y+0x01,TEMP
	LDS TEMP,_C5_UPTIME+0x02
	STD Y+0x02,TEMP
	LDS TEMP,_C5_UPTIME+0x03
	STD Y+0x03,TEMP
	LDS TEMP,_C5_UPTIME+0x04
	STD Y+0x04,TEMP
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RET

;--------------------------------------------------------
C5_UPTIME_GET:
;--------------------------------------------------------
;Помещаем в регистры 5 байт UPTIME
;OUT: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L,TEMP - UPTIME
;--------------------------------------------------------
	PUSH ACCUM
	LDS ACCUM,SREG
	CLI
	LDS TEMP_EH,_C5_UPTIME+0x00
	LDS TEMP_EL,_C5_UPTIME+0x01
	LDS TEMP_H,_C5_UPTIME+0x02
	LDS TEMP_L,_C5_UPTIME+0x03
	LDS TEMP,_C5_UPTIME+0x04
	STS SREG,ACCUM
	POP ACCUM
	RET

;========================================================================
;===Процедуры работы с таймерами=========================================
;========================================================================
;--------------------------------------------------------
C5_TCNT_GET:
;--------------------------------------------------------
;Получаем значение счетчика таймера
;Осчет кратный 0.000 000 5s
;OUT: TEMP - значение счетчика таймера
;--------------------------------------------------------
	_C5_TIMER_TCNT
	RET

;--------------------------------------------------------
C5_WAIT_500NS:
;--------------------------------------------------------
;Ждем истечения времени
;IN TEMP - время в 0.000 000 5s(не менее 40 ~20us)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	SUBI TEMP,36
	;Настройка таймера
	_C5_TIMERB


	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET

_C5_TIMER_B_IR:
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP

	;Сброс флага
	LDI TEMP,~(1<<_CFL_TIMER_B_USE)
	AND _C5_COREFLAGS,TEMP

	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;========================================================================
;===Процедуры работы с задачами и драйверами=============================
;========================================================================
;--------------------------------------------------------
C5_START:
;--------------------------------------------------------
;Запуск ядра
;--------------------------------------------------------
	C5_LOG_ROMSTR LOGSTR_CORE

	;Запуск таймера
	_CORRE5277_TIMERA_START
	;Разрешаем прерывания
	SEI
_C5_START__LOOP:
	CLT
	LDI PID,0x00													;Счетчик задач
_C5_START__TASKS_LOOP:
	MCALL _C5_PROC_HEADER_GET
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_READY
	BRNE _C5_START__NEXT_TASK
	PUSH PID
	MOV _PID,PID
	MCALL _C5_RESUME												;Возвращаемся по C5_SUSPEND
	POP PID
	SET
_C5_START__NEXT_TASK:
	INC PID
	CPI PID,_C5_TASKS_MAX_QNT
	BRNE _C5_START__TASKS_LOOP
	BRTS _C5_START__LOOP
	SLEEP
	RJMP _C5_START__LOOP

;--------------------------------------------------------
C5_CREATE:
;--------------------------------------------------------
;Создание процесса(задачи или драйвера).
;После создания будет выполнен переход на процесс
;для инициализации. После инициализации процесса
;необходимо вызвать процедуру C5_READY
;IN: PID, Z-точка входа
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL _C5_PROC_HEADER_GET
	;Записываем состояние и опции задачи
	MOV TEMP,PID
	ANDI TEMP,0x70														;Исключаем флаг драйвера
	ORI TEMP,_C5_PROC_STATE_GHOST
	STD Z+_C5_PROC_STATE,TEMP
	;Обнуляем адрес выделенной памяти и размер
	CLR TEMP
	STD Z+_C5_PROC_RAM_OFFSET+0x00,TEMP
	STD Z+_C5_PROC_RAM_OFFSET+0x01,TEMP
	STD Z+_C5_PROC_RAM_SIZE,TEMP

	;Проверяем на признак драйвера, пропускаем блок кода для задачи, если признак есть
	SBRC PID,C5_PROCID_OPT_DRV
	RJMP C5_CREATE__TASK_CODE_SKIP
	;Записываем начало стека на базе вершины стека задач
	LDS r0,_C5_TOP_OF_STACK+0x00
	STD Z+_C5_TASK_STACK_OFFSET+0x00,r0
	LDS r1,_C5_TOP_OF_STACK+0x01
	STD Z+_C5_TASK_STACK_OFFSET+0x01,r1
	;Записываем размер стека
	CLR TEMP
	STD Z+_C5_TASK_STACK_SIZE,TEMP
	;Восстанавливаем все регистры из стека
	POP TEMP
	POP_Z
	;Запоминаем текущее положение стека
	LDS r2,SPH
	STS _C5_CORE_STACK+0x00,r2
	LDS r2,SPL
	STS _C5_CORE_STACK+0x01,r2
	;Переключаемся на стек задачи
	STS SPH,r0
	STS SPL,r1

	;Помещаем точку возврата
	MOV r0,TEMP
	LDI TEMP,low(_C5_TASK_ENDPOINT)
	PUSH TEMP
	LDI TEMP,high(_C5_TASK_ENDPOINT)
	PUSH TEMP
	MOV TEMP,r0
	RJMP C5_CREATE__DRIVER_CODE_SKIP

C5_CREATE__TASK_CODE_SKIP:
	POP TEMP
	POP_Z
C5_CREATE__DRIVER_CODE_SKIP:
	;Единственный процесс, нет предков
	ANDI PID,(1<<C5_PROCID_OPT_DRV)|0x0f
	MOV _PID,PID
	;Переходим на нашу процедуру
	PUSH ZL
	PUSH ZH
	RET

;--------------------------------------------------------
C5_READY:
;--------------------------------------------------------
;Вызывается процессом после инициализации. В стеке должен
;лежать только адрес возврата. Т.е. стек задачи должен
;быть пуст, а данная процедура вызвана через CALL. При
;этом, после CALL должно быть размещено тело процесса.
;--------------------------------------------------------
	LDS r0,SREG
	SBRC _PID,C5_PROCID_OPT_DRV
	RJMP C5_READY__IS_DRIVER

	;Записываем в стек 16 старших регистров(могут быть проинициализированы при вызове процедуры)
	_C5_MACRO__PUSH_RDS
	;Записываем SREG (при RESUME будут насильно разрешены прерывания)
	PUSH r0

	;Обновляем вершину стека задач
	LDS r0,SPH
	LDS r1,SPL
	STS _C5_TOP_OF_STACK+0x00,r0
	STS _C5_TOP_OF_STACK+0x01,r1

	;Могли затереть PID, восстанавливаем
	MOV PID,_PID
	MCALL _C5_PROC_HEADER_GET

	;Обновляем размер стека
	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	SUB TEMP_L,r1
	SBC TEMP_H,r0
	;Проверка на размер стека более 255 байт
	TST TEMP_H
	BREQ PC+0x04
	LDI TEMP,_C5_FAULT_HUGE_TASK_STACK
	MCALL C5_LOG_COREFAULT
	STD Z+_C5_TASK_STACK_SIZE,TEMP_L

	;Запись временной метки
	LDS TEMP,_C5_UPTIME+0x04
	STD Z+_C5_TASK_TIMESTAMP,TEMP

	;Восстанавливем стек ядра
	LDS r5,_C5_CORE_STACK+0x00
	STS SPH,r5
	LDS r5,_C5_CORE_STACK+0x01
	STS SPL,r5
	RJMP C5_READY__END

C5_READY__IS_DRIVER:
	POP r0
	POP r1
	;Извлекаем точку входа на сновной код драйвера
	MOV PID,_PID
	MCALL _C5_PROC_HEADER_GET

	;Записываем адрес основной процедуры драйвера
	STD Z+_C5_DRIVER_OFFSET+0x00,r0
	STD Z+_C5_DRIVER_OFFSET+0x01,r1

C5_READY__END:
	;Записываем состояние не меняя опций
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_C5_PROC_STATE_READY
	STD Z+_C5_PROC_STATE,TEMP
	;Выхожу из процедуры C5_CREATE
	RET

;--------------------------------------------------------
C5_EXEC:
;--------------------------------------------------------
;Выполнение процедуры драйвера
;IN: TEMP - ид вызываемого драйвера
;--------------------------------------------------------
	PUSH PID

	;Процедура применима только для драйвера
	SBRS TEMP,C5_PROCID_OPT_DRV
	RJMP _C5_EXEC__DONE

	MCALL C5_DISPATCHER_LOCK
	;Сохраняем значение Z
	MOV _C5_DRIVER_EXEC_ZH,ZH
	MOV _C5_DRIVER_EXEC_ZL,ZL
	;Размещаем в стек адрес возврата из вызываемой процедуры
	LDI ZH,high(_C5_EXEC__DONE)
	LDI ZL,low(_C5_EXEC__DONE)
	PUSH ZL
	PUSH ZH
	;Получаем заголовок вызываемого драйвера
	MOV PID,TEMP
	MCALL _C5_PROC_HEADER_GET
	PUSH TEMP
	LDD TEMP,Z+_C5_DRIVER_OFFSET+0x00
	LDD ZL,Z+_C5_DRIVER_OFFSET+0x01
	MOV ZH,TEMP
	POP TEMP
	;Помещаем в стек адрес перехода на процедуру вызываемого драйвера
	PUSH ZL
	PUSH ZH
	;Восстанавливаем значение Z
	MOV ZH,_C5_DRIVER_EXEC_ZH
	MOV ZL,_C5_DRIVER_EXEC_ZL
	MCALL C5_DISPATCHER_UNLOCK
	;Перехожим на процедуру драйвера
	RET
_C5_EXEC__DONE:
	POP PID

	RET

;--------------------------------------------------------
_C5_RESUME:
;--------------------------------------------------------
;Продолжаем выполнение задачи
;IN: PID - ид задачи, Z - адрес заголовка задачи
;--------------------------------------------------------
	MCALL C5_DISPATCHER_LOCK
	;Записываем адрес стека ядра
	LDS TEMP,SPH
	STS _C5_CORE_STACK+0x00,TEMP
	LDS TEMP,SPL
	STS _C5_CORE_STACK+0x01,TEMP

	LDS TEMP,_C5_UPTIME+0x04
	STD Z+_C5_TASK_TIMESTAMP,TEMP

	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_C5_PROC_STATE_BUSY
	STD Z+_C5_PROC_STATE,TEMP

	;Переключаем стек на вершину стека, он же стек текущей задачи
	MCALL _C5_STACK_TO_TOP
	LDD TEMP, Z+_C5_TASK_STACK_SIZE
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	SUB TEMP_L,TEMP
	SBCI TEMP_H,0x00
	STS SPH,TEMP_H
	STS SPL,TEMP_L

	;Восстанавливаю SREG
	POP TEMP
	STS SREG,TEMP
	_C5_MACRO__POP_RDS
	SEI
	MCALL C5_DISPATCHER_UNLOCK
	RET

;--------------------------------------------------------
_C5_TASK_ENDPOINT:
;--------------------------------------------------------
;Отрабатывает при завершении задачи
;--------------------------------------------------------
	MCALL C5_DISPATCHER_LOCK

	MOV PID,_PID
	MCALL _C5_PROC_HEADER_GET

	LDI TEMP,0x00
	STD Z+_C5_TASK_STACK_SIZE,TEMP
	MCALL _C5_STACK_TO_TOP
	LDI TEMP,_C5_PROC_STATE_ABSENT
	STD Z+_C5_PROC_STATE,TEMP

	LDS TEMP,_C5_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_C5_CORE_STACK+0x01
	STS SPL,TEMP

	MCALL C5_DISPATCHER_UNLOCK
	RET

;--------------------------------------------------------
C5_SUSPEND:
;--------------------------------------------------------
;Приостанавливаем текущую задачу
;IN: _PID
;--------------------------------------------------------
	LDS r0,SREG
	;Записываем в стек 16 старших регистров(могут быть проинициализированы при вызове процедуры)
	_C5_MACRO__PUSH_RDS
	;Записываем SREG
	PUSH r0

	LDI TEMP,_C5_PROC_STATE_READY
	MCALL _C5_SUSPEND__BODY
	RET
;--------------------------------------------------------
_C5_SUSPEND__BODY:
	MOV PID,_PID														;Затираю PID, так как он уже должен быть записан в стек
	MCALL _C5_PROC_HEADER_GET

	LDD ACCUM,Z+_C5_PROC_STATE
	ANDI ACCUM,0xf0
	OR ACCUM,TEMP
	STD Z+_C5_PROC_STATE,ACCUM

	;Выгружаю и запоминаю адрес процедуры, которая нас вызвала
	POP _RESULT_H
	POP _RESULT_L

	;Получаем текущую позицию в стеке задачи
	LDS TEMP_H,SPH
	LDS TEMP_L,SPL
	;Стек задачи сформирован полностью, можно переключаться на стек ядра
	;Восстанавливаем стек ядра
	;Значение регистров не существенно, диспетчер нужные регистры уже сохранил в стек
	;После прерывания вернемся в диспетчер(после вызова TASK_RESUME)
	LDS TEMP,_C5_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_C5_CORE_STACK+0x01
	STS SPL,TEMP

	;проверяем на выход стека в зону общего буфера задач
	;TODO нет проверки на защитный порог(_C5_RAM_BORDER_SIZE)
	LDS TEMP,_C5_TOP_OF_FREE_RAM+0x00
	CP TEMP,TEMP_H
	BRCS _C5_SUSPEND__CORRECT_STACK_SIZE
	LDI TEMP,_C5_FAULT_HUGE_STACK
	MJMP C5_LOG_COREFAULT											;Стеки значения не имеют, ядро упало
	LDS TEMP,_C5_TOP_OF_FREE_RAM+0x01
	CP TEMP,TEMP_L
	BRCS _C5_SUSPEND__CORRECT_STACK_SIZE
	LDI TEMP,_C5_FAULT_HUGE_STACK
	MJMP C5_LOG_COREFAULT											;Стеки значения не имеют, ядро упало
_C5_SUSPEND__CORRECT_STACK_SIZE:

	;Получаем начало стека задачи
	LDD TEMP_EH,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_EL,Z+_C5_TASK_STACK_OFFSET+0x01
	;Считаем новую длину
	SUB TEMP_EL,TEMP_L
	SBC TEMP_EH,TEMP_H
	TST TEMP_EH
	BREQ PC+0x04
	LDI TEMP,_C5_FAULT_HUGE_TASK_STACK
	MCALL C5_LOG_COREFAULT											;Стеки значения не имеют, ядро упало

	;Обновляем длину стека задачи
	STD Z+_C5_TASK_STACK_SIZE,TEMP_EL
	;Обновляем вершину стека задач
	STS _C5_TOP_OF_STACK+0x00,TEMP_H
	STS _C5_TOP_OF_STACK+0x01,TEMP_L

	MCALL C5_DISPATCHER_UNLOCK
	;Восстанавливаем адрес процедуры, которая нас вызвала
	PUSH _RESULT_L
	PUSH _RESULT_H
	RET

;--------------------------------------------------------
_C5_PROC_HEADER_GET:
;--------------------------------------------------------
;Возвращает смещение на заголовок процесса
;IN:PID-ид драйвера
;OUT:Z-смещение
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	SBRS PID,C5_PROCID_OPT_DRV
	RJMP _C5_PROC_HEADER_GET__TASK
_C5_PROC_HEADER_GET__DRIVER:
	LDI ZH,high(_C5_DRIVERS_HEADER)
	LDI ZL,low(_C5_DRIVERS_HEADER)
	MOV TEMP_H,PID													;Умножаем на 0x06(_C5_DRIVER_HEADER_SIZE)
	ANDI TEMP_H,0x0f
	LSL TEMP_H
	MOV TEMP_L,TEMP_H
	LSL TEMP_H
	ADD TEMP_L,TEMP_H
	RJMP _C5_PROC_HEADER_GET__END
_C5_PROC_HEADER_GET__TASK:
	LDI ZL,low(_C5_TASKS_HEADER)
	LDI ZH,high(_C5_TASKS_HEADER)
	MOV TEMP_H,PID													;Умножаем на 0x0b(_C5_TASK_HEADER_SIZE)
	ANDI TEMP_H,0x0f
	MOV TEMP_L,TEMP_H
	LSL TEMP_L
	ADD TEMP_H,TEMP_L
	LSL TEMP_L
	LSL TEMP_L
	ADD TEMP_L,TEMP_H
_C5_PROC_HEADER_GET__END:
	ADD ZL,TEMP_L
	CLR TEMP_H
	ADC ZH,TEMP_H

	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_C5_STACK_TO_TOP:
;--------------------------------------------------------
;Переносим стек задачи наверх
;IN: Z-адрес на заголовок задачи
;--------------------------------------------------------
	PUSH_Z
	PUSH PID
	;Проверка если стек задачи и так наверху
	;Получаем размер стека текущей задачи
	LDD TEMP_EL,Z+_C5_TASK_STACK_SIZE
	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	SUB TEMP_L,TEMP_EL
	SBCI TEMP_H,0x00
	LDS TEMP,_C5_TOP_OF_STACK+0x00
	CP TEMP_H,TEMP
	BRNE PC+0x04
	LDS TEMP,_C5_TOP_OF_STACK+0x01
	CP TEMP_L,TEMP
	BREQ _C5_STACK_TO_TOP__END

	;Получаем старое смещение на стек текущей задачи
	LDD XH,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD XL,Z+_C5_TASK_STACK_OFFSET+0x01

	;Получаем вершину стека задач
	LDS TEMP_H,_C5_TOP_OF_STACK+0x00
	LDS TEMP_L,_C5_TOP_OF_STACK+0x01
	STD Z+_C5_TASK_STACK_OFFSET+0x00,TEMP_H
	STD Z+_C5_TASK_STACK_OFFSET+0x01,TEMP_L
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	PUSH_Z
	;Копируем стек задачи наверх
	;TODO из смещений нужно вычесть длину стека задачи(+/-1)
	MOV LOOP_CNTR,TEMP_EL
	MCALL C5_RAM_COPY8_DESC

	;Записываю старое смещение на стек текущей задачи
	MOV ZH,XH
	MOV ZL,XL
	;Получаю смещение на следующий стек задачи
	SUB XL,TEMP_EL
	SBCI XH,0x00

	POP TEMP_L
	POP TEMP_H

	;Вычисляю размер копируемых данных(Z-старый адрес стека задачи, TEMP_H/L - вершина стека задач)
	PUSH_Z
	SUB ZL,TEMP_L
	SBC ZH,TEMP_H
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	POP_Z
	PUSH_Z
	;Копирую данные
	MCALL C5_RAM_COPY16_DESC
	POP_X																	;X теперь хранит страое смещение на стек задачи

	LDI PID,0x00
_C5_STACK_TO_TOP__LOOP:
	MCALL _C5_PROC_HEADER_GET

	;Ели задачи нет, пропускаем
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_ABSENT
	BREQ _C5_STACK_TO_TOP__NEXT_TASK

	;Проверка, если адрес стека задачи в цикле больше чем адрес стека основной задачи(X), то пропускаем(равенство исключено)
	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	CP XH,TEMP_H
	BRCS _C5_STACK_TO_TOP__NEXT_TASK
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	CP XL,TEMP_L
	BRCS _C5_STACK_TO_TOP__NEXT_TASK

	;Изменяем смещение на длину стека основной задачи
	ADD TEMP_L,TEMP_EL
	CLR TEMP
	ADC TEMP_H,TEMP
	STD Z+_C5_TASK_STACK_OFFSET+0x00,TEMP_H
	STD Z+_C5_TASK_STACK_OFFSET+0x01,TEMP_L

_C5_STACK_TO_TOP__NEXT_TASK:
	INC PID
	CPI PID,_C5_TASKS_MAX_QNT
	BRNE _C5_STACK_TO_TOP__LOOP

_C5_STACK_TO_TOP__END:
	POP PID
	POP_Z
	RET
