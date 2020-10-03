;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Тут просто дохрена изменений
;09.08.2020  w5277c@gmail.com        Тут просто дохрена изменений
;06.09.2020  w5277c@gmail.com        Восстановлены механизмы многопоточности
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
;find -type f -name \*.asm -exec sed -i -r 's/CORE5277/C5/g' {} \;

;-----------------------------------------------------------------------------------------------------------------------
	.SET AVRA										= 0x00			;0x01 - выводит при сборке размеры блоков и некоторые смещения

;---PRCO-STATES------------------------------------------;Состояния задачи (PROC-STATES-младший ниббл, PROC-MODE-старший ниббл)
	.EQU	_CORE5277_PROC_STATE_ABSENT		= 0x00			;Нет задачи
	.EQU	_CORE5277_PROC_STATE_GHOST			= 0x01			;Инициализируется или деактивация
	.EQU	_CORE5277_PROC_STATE_BUSY			= 0x02			;Выполняется
	.EQU	_CORE5277_PROC_STATE_TIME_WAIT	= 0x03			;Ждет истечения времени(только для задач)
	.EQU	_CORE5277_PROC_STATE_RES_WAIT		= 0x04			;Ждет освобождения ресурса(только для задач)
	.EQU	_CORE5277_PROC_STATE_READY			= 0x05			;Готова к выполнению
;---PROC-ID-OPTIONS--------------------------------------;Не изменяемые опции процесса размещаемые в идентификаторе процесса
	.EQU	CORE5277_PROCID_OPT_NOSUSP			= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	CORE5277_PROCID_OPT_RESERV1		= 0x05			;Зарезервировано
	.EQU	CORE5277_PROCID_OPT_RESERV2		= 0x06			;Зарезервировано
	.EQU	CORE5277_PROCID_OPT_DRV				= 0x07			;Драйвер
;---PROC-STATE-OPTIONS-----------------------------------;Опции процесса, размещаемые в статусе процесса(старшйи ниббл)
	.EQU	_CORE5277_PROCST_OPT_NOSUSP		= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	_CORE5277_PROCST_OPT_RESERV1		= 0x05			;Зарезервировано
	.EQU	_CORE5277_PROCST_OPT_RESERV2		= 0x06			;Зарезервировано
	.EQU	_CORE5277_PROCST_OPT_RESERV3		= 0x07			;Зарезервировано

;---TIMER-STRUCTURE---------------------------------------(5b);
	.EQU	_CORE5277_TIMER_PROC_ID				= 0x00			;1b - ид процедуры(0-..., 0xff - не исп.)
	.EQU	_CORE5277_TIMER_HANDLER				= 0x01			;2b - адрес обработчика
	.EQU	_CORE5277_TIMER_CNTR					= 0x03			;1b - счетчик таймера
	.EQU	_CORE5277_TIMER_DEFAULT				= 0x04			;1b - порог

;---PROC-STRUCTURE---------------------------------------;Заголовок процедуры(6B-драйвер, 11B-задача)
	.EQU	_CORE5277_PROC_STATE					= 0x00			;1B - статус процедуры(младший ниббл) и опции(старший ниббл)
	.EQU	_CORE5277_PROC_RAM_OFFSET			= 0x01			;2B - адрес начала выделенной памяти
	.EQU	_CORE5277_PROC_RAM_SIZE				= 0x03			;1B - размер выделенной памяти
	.EQU	_CORE5277_DRIVER_OFFSET				= 0x04			;2B - адрес процедуры драйвера или
	.EQU	_CORE5277_TASK_STACK_OFFSET		= 0x04			;2B - адрес начала стека задачи
	.EQU	_CORE5277_TASK_STACK_SIZE			= 0x06			;1B - размер стека задачи
	.EQU	_CORE5277_TASK_TIMESTAMP			= 0x07			;1B - мекта времени переключения на задачу
	.EQU	_CORE5277_TASK_TIMEOUT				= 0x08			;3B - время ожидания задачи(~9 часов максимум, тик в 2мс)

;---CONSTANTS--------------------------------------------
	.EQU	_CORE5277_DRIVER_HEADER_SIZE		= 0x06			;Размер заголовка драйвера
	.EQU	_CORE5277_TASK_HEADER_SIZE			= 0x0b			;Размер заголовка задачи
	.EQU	_CORE5277_TIMERS_MAX_QNT			= 0x04
	.EQU	_CORE5277_TIMER_PERIOD				= 0x28			;40, т.е. 0.000050*40=0.002=2мс
	.EQU	_CORE5277_TASK_STACK_DEF_SIZE		= 0x13			;Размер стека задачи при создании (16 регистров + SREG + точка возврата)
	.EQU	_CORE5277_TASKS_ACTIVE_TIME		= 0x01			;Время работы задачи 1*2 = 2мс
	.EQU	_CORE5277_IRV_TABLE_SIZE			= 0x4b			;Размер таблицы прерываний
	;---CORE-FAULTS---
	.EQU	_CORE5277_FAULT_HUGE_STACK			= 0x01			;Стек залез на границу между стеком и выделяемой памяти
	.EQU	_CORE5277_FAULT_HUGE_TASK_STACK	= 0x02			;Размер стека задачи более 255 байт
	;---CORE-RESOURCES---											;Идентфикаторы ресурсов, 0-127 отданы пользователю, старшие - ядру
	.EQU	CORE5277_RES_TIMER_B					= 0xFE			;Таймер B
	;---
;---CORE-FLAGS-------------------------------------------
	.EQU	_CFL_TIMER_B_USE						= 0x00			;Флаг запущенного таймера B
	.EQU	_CFL_TIMER_UART_CORRECTION			= 0x01			;Корректировка таймера для соответствия частотам UART
	.EQU	_CFL_DISPATCHER_ORDER				= 0x02			;Заявка на отработку диспетчера(формируется когда диспетчер заблокирован)

;---RAM-OFFSETS------------------------------------------
	.EQU	_CORE5277_UPTIME						= 0x100			;5B - отсчет времени (273 года)
	.EQU	_CORE5277_CORE_STACK					= 0x105			;2B - хранит смещение стека ядра
	.EQU	_CORE5277_TOP_OF_STACK				= 0x107			;2B - хранит вершину стека
	.EQU	_CORE5277_TOP_OF_FREE_RAM			= 0x109			;2B - хранит вершину свободной памяти
	.EQU	_CORE5277_FLAGS						= 0x10b			;4B - флагов (32 флага)
	.EQU	_CORE5277_MAIN_TIMER_CNTR			= 0x10f			;1B - счетчик таймера ядра (каждые 0.000050с)
	.EQU	_CORE5277_TIMERS						= 0x110			;XB - таблица таймеров(TIMERS*5)
																			;TIMERS*5
	.EQU	_CORE5277_IR_VECTORS_TABLE			= _CORE5277_TIMERS+TIMERS*5
																			;XB - таблица прерываний (_CORE5277_IR_QNT*3 для каждгого прерывания, исключая RESET)
	.EQU	_CORE5277_DRIVERS_HEADER			= _CORE5277_IR_VECTORS_TABLE+_CORE5277_IR_QNT*3
																			;XB - заголовки драйвера (X=_CORE5277_DRIVERS_MAX_QNT*_CORE5277_DRIVER_HEADER_SIZE)
	.EQU	_CORE5277_TASKS_HEADER				= _CORE5277_DRIVERS_HEADER+_CORE5277_DRIVERS_MAX_QNT*_CORE5277_DRIVER_HEADER_SIZE
																			;XB - заголовки задач (X=_CORE5277_TASKS_MAX_QNT*_CORE5277_TASK_HEADER_SIZE)
	.EQU	_CORE5277_RESOURCE_QUEUE			= _CORE5277_TASKS_HEADER+_CORE5277_TASKS_MAX_QNT*_CORE5277_TASK_HEADER_SIZE
																			;XB - очередь задач к ресурсам(1b - RES_ID, 1b - TASK_ID...)
	.EQU	_CORE5277_FREE_RAM					= _CORE5277_RESOURCE_QUEUE+_CORE5277_RES_QUEUE_SIZE
	.EQU	CORE5277_BUFFER						= RAMEND-(_CORE5277_STACK_SIZE+BUFFER_SIZE)
	.EQU	_CORE5277_STACK_END					= CORE5277_BUFFER	;XB - стек ядра, далее буфер, стек задач и выделяемая память
														;TODO проверить что стек не перетирает первый байт буфера
.IF AVRA == 0x01
	.IF BUFFER_SIZE > 0x00
		.MESSAGE "######## BUFFER SIZE:",BUFFER_SIZE
	.ENDIF
	.MESSAGE "######## AVAILABLE RAM:",_CORE5277_STACK_END-(_CORE5277_FREE_RAM+_CORE5277_RAM_BORDER_SIZE)
	.MESSAGE "######## DRIVERS HEADERS OFFSET:",_CORE5277_DRIVERS_HEADER
	.MESSAGE "######## TASKS HEADERS OFFSET:",_CORE5277_TASKS_HEADER
	.MESSAGE "######## FREE RAM OFFSET:",_CORE5277_FREE_RAM
.ENDIF

;---REGISTERS--------------------------------------------;r0-r14 выделено под нужды ядра
	.DEF	_RESULT_L								= r0
	.DEF	_RESULT_H								= r1
	;...
	.DEF	_CORE5277_DISPATCHER_LOCK_CNTR	= r9				;Счетчик вложенных блокировок диспетчера
	.DEF	_CORE5277_DRIVER_EXEC_ZH			= r10
	.DEF	_CORE5277_DRIVER_EXEC_ZL			= r11
	.DEF	_CORE5277_TEMP_H						= r12
	.DEF	_CORE5277_TEMP_L						= r13
	.DEF	_CORE5277_COREFLAGS					= r14				;флаги ядра
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

.MACRO _CORE5277_MACRO__PUSH_RDS
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

.MACRO _CORE5277_MACRO__POP_RDS
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
CORE5277_DISPATCHER_LOCK:
CORE5277_DISPATCHER_UNLOCK:
	RET
.ENDIF

.IF TIMERS > 0x00
	.MESSAGE "######## TIMERS ENABLED ########"
	.INCLUDE "./inc/core/timer_set.inc"
	.INCLUDE "./inc/core/timer_start.inc"
	.INCLUDE "./inc/core/timer_stop.inc"
.ELSE
	.MESSAGE "######## TIMERS DISABLED ########"
	CORE5277_TIMER_SET:
	CORE5277_TIMER_START:
	CORE5277_TIMER_STOP:
		RET
.ENDIF

;========================================================================
;===Обработка прерываний=================================================
;========================================================================
;--------------------------------------------------------
_CORE5277_IR:
;--------------------------------------------------------
;Обработчик всех прерываний
;--------------------------------------------------------
	;Получаем адрес по которому определим тип прерывания
	POP _CORE5277_TEMP_H
	POP _CORE5277_TEMP_L

	;Сохраняем регистры
	PUSH TEMP
	LDS TEMP,SREG
	CLI
	PUSH TEMP
	PUSH_Z
	PUSH PID
	PUSH ACCUM
	;Находим смещение в таблице прерываний _CORE5277_IR_VECTORS_TABLE
	LDI ZH,high(_CORE5277_IR_VECTORS_TABLE)
	LDI ZL,low(_CORE5277_IR_VECTORS_TABLE)

	_CORRE5277_IR_OFFSET_CORRECTOR

	MOV ACCUM,_CORE5277_TEMP_L										;Номер строки в таблице перывания
	DEC _CORE5277_TEMP_L												;Первая запись в таблице прерываний не используется
	BREQ PC+0x04
	MOV TEMP,_CORE5277_TEMP_L										;Умножаю на 3(1 байт - PID, 2 байта - адрес перехода)
	LSL _CORE5277_TEMP_L
	ADD _CORE5277_TEMP_L,TEMP

	ADD ZL,_CORE5277_TEMP_L
	LD PID,Z+
	CPI PID,0xff
	BREQ _CORE5277_IR__END

	LD TEMP,Z+
	LD ZL,Z
	MOV ZH,TEMP
	;Корневой процесс - текущий и единственный
	PUSH _PID
	MOV _PID,PID
	;Переходим если адрес указан(в ACCUM номер строки таблицы прерываний)
	ICALL																	;Возвращаться надо по RET(не RETI), TEMP и SREG сохранять не нужно
	POP _PID
_CORE5277_IR__END:
	;Восстанавливаю регистры и выхожу
	POP ACCUM
	POP PID
	POP_Z
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;--------------------------------------------------------
_CORE5277_TIMER_A_IR:
;--------------------------------------------------------
;Обработчик прерывания таймера
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP

	_CORRE5277_TIMERA_CORRECTOR

	PUSH_Z
	PUSH PID

.IF TIMERS > 0x00
	;23t max
	LDS PID,_CORE5277_TIMERS+0x00*0x05+0x00					;2
	CPI PID,0xff														;1
	BREQ _CORE5277_TIMER_IR__TIMER2								;1/2
	LDS ZH,_CORE5277_TIMERS+0x00*0x05+0x01						;2
	LDS ZL,_CORE5277_TIMERS+0x00*0x05+0x02						;2
	LDS TEMP,_CORE5277_TIMERS+0x00*0x05+0x03					;2
	DEC TEMP																;1
	BRNE PC+0x04														;1/2
	ICALL																	;3+4
	LDS TEMP,_CORE5277_TIMERS+0x00*0x05+0x04					;2
	STS _CORE5277_TIMERS+0x00*0x05+0x03,TEMP					;2
_CORE5277_TIMER_IR__TIMER2:
.ENDIF
.IF TIMERS > 0x01
	;23t max
	LDS PID,_CORE5277_TIMERS+0x01*0x05+0x00					;2
	CPI PID,0xff														;1
	BREQ _CORE5277_TIMER_IR__TIMER3								;1/2
	LDS ZH,_CORE5277_TIMERS+0x01*0x05+0x01						;2
	LDS ZL,_CORE5277_TIMERS+0x01*0x05+0x02						;2
	LDS TEMP,_CORE5277_TIMERS+0x01*0x05+0x03					;2
	DEC TEMP																;1
	BRNE PC+0x04														;1/2
	ICALL																	;3+4
	LDS TEMP,_CORE5277_TIMERS+0x01*0x05+0x04					;2
	STS _CORE5277_TIMERS+0x01*0x05+0x03,TEMP					;2
_CORE5277_TIMER_IR__TIMER3:
.ENDIF
.IF TIMERS > 0x02
	;23t max
	LDS PID,_CORE5277_TIMERS+0x02*0x05+0x00					;2
	CPI PID,0xff														;1
	BREQ _CORE5277_TIMER_IR__TIMER4								;1/2
	LDS ZH,_CORE5277_TIMERS+0x02*0x05+0x01						;2
	LDS ZL,_CORE5277_TIMERS+0x02*0x05+0x02						;2
	LDS TEMP,_CORE5277_TIMERS+0x02*0x05+0x03					;2
	DEC TEMP																;1
	BRNE PC+0x04														;1/2
	ICALL																	;3+4
	LDS TEMP,_CORE5277_TIMERS+0x02*0x05+0x04					;2
	STS _CORE5277_TIMERS+0x02*0x05+0x03,TEMP					;2
_CORE5277_TIMER_IR__TIMER4:
.ENDIF
.IF TIMERS > 0x03
	;23t max
	LDS PID,_CORE5277_TIMERS+0x03*0x05+0x00					;2
	CPI PID,0xff														;1
	BREQ _CORE5277_TIMER_IR__DISPATCHER							;1/2
	LDS ZH,_CORE5277_TIMERS+0x03*0x05+0x01						;2
	LDS ZL,_CORE5277_TIMERS+0x03*0x05+0x02						;2
	LDS TEMP,_CORE5277_TIMERS+0x03*0x05+0x03					;2
	DEC TEMP																;1
	BRNE PC+0x04														;1/2
	ICALL																	;3+4
	LDS TEMP,_CORE5277_TIMERS+0x03*0x05+0x04					;2
	STS _CORE5277_TIMERS+0x03*0x05+0x03,TEMP					;2
.ENDIF
.IF TIMERS > 0x04
	.MESSAGE "########  ERR: Exceeded maximum timers ########"
.ENDIF
_CORE5277_TIMER_IR__DISPATCHER:
	POP PID

	;14t max
	LDS TEMP,_CORE5277_MAIN_TIMER_CNTR							;2
	DEC TEMP																;1
	STS _CORE5277_MAIN_TIMER_CNTR,TEMP							;2
	BRNE _CORE5277_TIMER_A_IR__UPTIME_SKIP						;1/2 + 1
	LDI TEMP,_CORE5277_TIMER_PERIOD								;1
	STS _CORE5277_MAIN_TIMER_CNTR,TEMP							;2
	;Тик 1 кванта
	LDI ZL,0x01
	LDS ZH,_CORE5277_UPTIME+0x04
	ADD ZH,ZL
	STS _CORE5277_UPTIME+0x04,ZH
	LDI ZL,0x00
	LDS ZH,_CORE5277_UPTIME+0x03
	ADC ZH,ZL
	STS _CORE5277_UPTIME+0x03,ZH
	LDS ZH,_CORE5277_UPTIME+0x02
	ADC ZH,ZL
	STS _CORE5277_UPTIME+0x02,ZH
	LDS ZH,_CORE5277_UPTIME+0x01
	ADC ZH,ZL
	STS _CORE5277_UPTIME+0x01,ZH
	LDS ZH,_CORE5277_UPTIME+0x00
	ADC ZH,ZL
	STS _CORE5277_UPTIME+0x00,ZH
	POP_Z
.IF REALTIME == 1
	MCALL _CORE5277_DISPATCHER_EVENT								;3+4
.ENDIF
	RJMP _CORE5277_TIMER_A_IR__END
_CORE5277_TIMER_A_IR__UPTIME_SKIP:
	POP_Z
_CORE5277_TIMER_A_IR__END:

	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;========================================================================
;===Базовые процедуры ядра===============================================
;========================================================================
;--------------------------------------------------------
CORE5277_INIT:
;--------------------------------------------------------
;Инициализация ядра
;--------------------------------------------------------
	;Запрещение прерываний
	CLI

	;Инициализация логирования
	MCALL CORE5277_LOG_INIT

	MCALL CORE5277_LOG_TASKSDUMP
	;Сбрасываем флаги причины сброса
	CLR TEMP
	STS MCUSR,TEMP

	;Заполнение всей памяти 0xF7 значением для(диагностики)
	LDI TEMP,0xF7
	LDI TEMP_H,high(RAMEND-0x010f)									;RAM_SIZE-16(stack)
	LDI TEMP_L,low(RAMEND-0x010f)
	LDI XH,0x01
	LDI XL,0x00
	MCALL CORE5277_RAM_FILL16

	;Чистка таблицы прерываний
	LDI XL,low(_CORE5277_IR_VECTORS_TABLE)
	LDI XH,high(_CORE5277_IR_VECTORS_TABLE)
	LDI TEMP,0xff
	LDI LOOP_CNTR,_CORE5277_IRV_TABLE_SIZE						;IV_VECTORS_TABLE
	MCALL CORE5277_RAM_FILL8
	;Чистка очереди
	LDI XH,high(_CORE5277_RESOURCE_QUEUE)
	LDI XL,low(_CORE5277_RESOURCE_QUEUE)
	LDI LOOP_CNTR,_CORE5277_RES_QUEUE_SIZE
	LDI TEMP,0xff
	MCALL CORE5277_RAM_FILL8

	;Чистка переменных в памяти
	CLR _CORE5277_COREFLAGS
	;Сброс счетчика времени
	LDI XH,high(_CORE5277_UPTIME)
	LDI XL,low(_CORE5277_UPTIME)
	LDI LOOP_CNTR,0x05
	LDI TEMP,0x00
	MCALL CORE5277_RAM_FILL8

	;Задаю вершину стека
	LDI XH,high(_CORE5277_STACK_END)
	LDI XL,low(_CORE5277_STACK_END)
	STS _CORE5277_TOP_OF_STACK+0x00,XH
	STS _CORE5277_TOP_OF_STACK+0x01,XL
	;Задаю вершину свободной памяти
	LDI XH,high(_CORE5277_FREE_RAM)
	LDI XL,low(_CORE5277_FREE_RAM)
	STS _CORE5277_TOP_OF_FREE_RAM+0x00,XH
	STS _CORE5277_TOP_OF_FREE_RAM+0x01,XL

	;Чистка флагов
	CLR TEMP
	STS _CORE5277_FLAGS+0x00,TEMP
	STS _CORE5277_FLAGS+0x01,TEMP
	STS _CORE5277_FLAGS+0x02,TEMP
	STS _CORE5277_FLAGS+0x03,TEMP
	STS _CORE5277_FLAGS+0x04,TEMP
	STS _CORE5277_FLAGS+0x05,TEMP
	STS _CORE5277_FLAGS+0x06,TEMP
	STS _CORE5277_FLAGS+0x07,TEMP

	;Задаем счетчик основному таймеру
	LDI TEMP,_CORE5277_TIMER_PERIOD
	STS _CORE5277_MAIN_TIMER_CNTR,TEMP
.IF TIMERS > 0x00
	;Установка таймеров как не исполльзуемых
	LDI XH,high(_CORE5277_TIMERS)
	LDI XL,low(_CORE5277_TIMERS)
	LDI LOOP_CNTR,TIMERS*0x05
	LDI TEMP,0xff
	MCALL CORE5277_RAM_FILL8
.ENDIF

	;Чистка заголовков драйверов
	LDI XH,high(_CORE5277_DRIVERS_HEADER)
	LDI XL,low(_CORE5277_DRIVERS_HEADER)
	LDI TEMP_H,high(_CORE5277_DRIVER_HEADER_SIZE*_CORE5277_DRIVERS_MAX_QNT)
	LDI TEMP_L,low(_CORE5277_DRIVER_HEADER_SIZE*_CORE5277_DRIVERS_MAX_QNT)
	LDI TEMP,_CORE5277_PROC_STATE_ABSENT
	MCALL CORE5277_RAM_FILL16
	;Чистка заголовков задач
	LDI XH,high(_CORE5277_TASKS_HEADER)
	LDI XL,low(_CORE5277_TASKS_HEADER)
	LDI TEMP_H,high(_CORE5277_TASK_HEADER_SIZE*_CORE5277_TASKS_MAX_QNT)
	LDI TEMP_L,low(_CORE5277_TASK_HEADER_SIZE*_CORE5277_TASKS_MAX_QNT)
	LDI TEMP,_CORE5277_PROC_STATE_ABSENT
	MCALL CORE5277_RAM_FILL16

	;Чистка регистров
	CLR _CORE5277_DISPATCHER_LOCK_CNTR

	;Инициализирую таймер (0.000050s - 800 тактов)
	_CORE5277_TIMERA_INIT
	RET

;--------------------------------------------------------
CORE5277_IR_VECTOR_SET:
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
	BREQ _CORE5277_IR_VECTOR_SET__END

	CLI
	DEC ACCUM
	MOV ZL,ACCUM
	LSL ACCUM
	ADD ACCUM,ZL
	LDI ZH,high(_CORE5277_IR_VECTORS_TABLE)
	LDI ZL,low(_CORE5277_IR_VECTORS_TABLE)
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	STD Z+0x00,PID
	STD Z+0x01,TEMP_H
	STD Z+0x02,TEMP_L

_CORE5277_IR_VECTOR_SET__END:
	POP ACCUM
	POP ZL
	STS SREG,ZL
	POP_Z
	RET

;--------------------------------------------------------
CORE5277_SOFT_UART_MODE_SET:
;--------------------------------------------------------
;Установка режима программного UART
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,(1<<_CFL_TIMER_UART_CORRECTION)
	OR _CORE5277_COREFLAGS,TEMP
	POP TEMP
	RET
;--------------------------------------------------------
CORE5277_SOFT_UART_MODE_RESET:
;--------------------------------------------------------
;Снятие режима программного UART
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,~((1<<_CFL_TIMER_UART_CORRECTION))
	AND _CORE5277_COREFLAGS,TEMP
	POP TEMP
	RET

;--------------------------------------------------------
CORE5277_UPTIME_WRITE:
;--------------------------------------------------------
;Записываем в память 5 байт UPTIME
;IN: Y - адрес для записи
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDS TEMP,_CORE5277_UPTIME+0x00
	STD Y+0x00,TEMP
	LDS TEMP,_CORE5277_UPTIME+0x01
	STD Y+0x01,TEMP
	LDS TEMP,_CORE5277_UPTIME+0x02
	STD Y+0x02,TEMP
	LDS TEMP,_CORE5277_UPTIME+0x03
	STD Y+0x03,TEMP
	LDS TEMP,_CORE5277_UPTIME+0x04
	STD Y+0x04,TEMP
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RET

;--------------------------------------------------------
CORE5277_UPTIME_GET:
;--------------------------------------------------------
;Помещаем в регистры 5 байт UPTIME
;OUT: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L,TEMP - UPTIME
;--------------------------------------------------------
	PUSH ACCUM
	LDS ACCUM,SREG
	CLI
	LDS TEMP_EH,_CORE5277_UPTIME+0x00
	LDS TEMP_EL,_CORE5277_UPTIME+0x01
	LDS TEMP_H,_CORE5277_UPTIME+0x02
	LDS TEMP_L,_CORE5277_UPTIME+0x03
	LDS TEMP,_CORE5277_UPTIME+0x04
	STS SREG,ACCUM
	POP ACCUM
	RET

;========================================================================
;===Процедуры работы с таймерами=========================================
;========================================================================
;--------------------------------------------------------
CORE5277_TCNT_GET:
;--------------------------------------------------------
;Получаем значение счетчика таймера
;Осчет кратный 0.000 000 5s
;OUT: TEMP - значение счетчика таймера
;--------------------------------------------------------
	_CORE5277_TIMER_TCNT
	RET

;--------------------------------------------------------
CORE5277_WAIT_500NS:
;--------------------------------------------------------
;Ждем истечения времени
;IN TEMP - время в 0.000 000 5s(не менее 40 ~20us)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	SUBI TEMP,36
	;Настройка таймера
	_CORE5277_TIMERB


	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET

_CORE5277_TIMER_B_IR:
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP

	;Сброс флага
	LDI TEMP,~(1<<_CFL_TIMER_B_USE)
	AND _CORE5277_COREFLAGS,TEMP

	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI

;========================================================================
;===Процедуры работы с задачами и драйверами=============================
;========================================================================
;--------------------------------------------------------
CORE5277_START:
;--------------------------------------------------------
;Запуск ядра
;--------------------------------------------------------
	CORE5277_LOG_ROMSTR LOGSTR_CORE

	;Запуск таймера
	_CORRE5277_TIMERA_START
	;Разрешаем прерывания
	SEI
_CORE5277_START__LOOP:
	CLT
	LDI PID,0x00													;Счетчик задач
_CORE5277_START__TASKS_LOOP:
	MCALL _CORE5277_PROC_HEADER_GET
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_READY
	BRNE _CORE5277_START__NEXT_TASK
	PUSH PID
	MOV _PID,PID
	MCALL _CORE5277_RESUME										;Возвращаемся по CORE5277_SUSPEND
	POP PID
	SET
_CORE5277_START__NEXT_TASK:
	INC PID
	CPI PID,_CORE5277_TASKS_MAX_QNT
	BRNE _CORE5277_START__TASKS_LOOP
	BRTS _CORE5277_START__LOOP
	SLEEP
	RJMP _CORE5277_START__LOOP

;--------------------------------------------------------
CORE5277_CREATE:
;--------------------------------------------------------
;Создание процесса(задачи или драйвера).
;После создания будет выполнен переход на процесс
;для инициализации. После инициализации процесса
;необходимо вызвать процедуру CORE5277_READY
;IN: PID, Z-точка входа
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL _CORE5277_PROC_HEADER_GET
	;Записываем состояние и опции задачи
	MOV TEMP,PID
	ANDI TEMP,0x70														;Исключаем флаг драйвера
	ORI TEMP,_CORE5277_PROC_STATE_GHOST
	STD Z+_CORE5277_PROC_STATE,TEMP
	;Обнуляем адрес выделенной памяти и размер
	CLR TEMP
	STD Z+_CORE5277_PROC_RAM_OFFSET+0x00,TEMP
	STD Z+_CORE5277_PROC_RAM_OFFSET+0x01,TEMP
	STD Z+_CORE5277_PROC_RAM_SIZE,TEMP

	;Проверяем на признак драйвера, пропускаем блок кода для задачи, если признак есть
	SBRC PID,CORE5277_PROCID_OPT_DRV
	RJMP CORE5277_CREATE__TASK_CODE_SKIP
	;Записываем начало стека на базе вершины стека задач
	LDS r0,_CORE5277_TOP_OF_STACK+0x00
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x00,r0
	LDS r1,_CORE5277_TOP_OF_STACK+0x01
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x01,r1
	;Записываем размер стека
	CLR TEMP
	STD Z+_CORE5277_TASK_STACK_SIZE,TEMP
	;Восстанавливаем все регистры из стека
	POP TEMP
	POP_Z
	;Запоминаем текущее положение стека
	LDS r2,SPH
	STS _CORE5277_CORE_STACK+0x00,r2
	LDS r2,SPL
	STS _CORE5277_CORE_STACK+0x01,r2
	;Переключаемся на стек задачи
	STS SPH,r0
	STS SPL,r1

	;Помещаем точку возврата
	MOV r0,TEMP
	LDI TEMP,low(_CORE5277_TASK_ENDPOINT)
	PUSH TEMP
	LDI TEMP,high(_CORE5277_TASK_ENDPOINT)
	PUSH TEMP
	MOV TEMP,r0
	RJMP CORE5277_CREATE__DRIVER_CODE_SKIP

CORE5277_CREATE__TASK_CODE_SKIP:
	POP TEMP
	POP_Z
CORE5277_CREATE__DRIVER_CODE_SKIP:
	;Единственный процесс, нет предков
	ANDI PID,(1<<CORE5277_PROCID_OPT_DRV)|0x0f
	MOV _PID,PID
	;Переходим на нашу процедуру
	PUSH ZL
	PUSH ZH
	RET

;--------------------------------------------------------
CORE5277_READY:
;--------------------------------------------------------
;Вызывается процессом после инициализации. В стеке должен
;лежать только адрес возврата. Т.е. стек задачи должен
;быть пуст, а данная процедура вызвана через CALL. При
;этом, после CALL должно быть размещено тело процесса.
;--------------------------------------------------------
	LDS r0,SREG
	SBRC _PID,CORE5277_PROCID_OPT_DRV
	RJMP CORE5277_READY__IS_DRIVER

	;Записываем в стек 16 старших регистров(могут быть проинициализированы при вызове процедуры)
	_CORE5277_MACRO__PUSH_RDS
	;Записываем SREG (при RESUME будут насильно разрешены прерывания)
	PUSH r0

	;Обновляем вершину стека задач
	LDS r0,SPH
	LDS r1,SPL
	STS _CORE5277_TOP_OF_STACK+0x00,r0
	STS _CORE5277_TOP_OF_STACK+0x01,r1

	;Могли затереть PID, восстанавливаем
	MOV PID,_PID
	MCALL _CORE5277_PROC_HEADER_GET

	;Обновляем размер стека
	LDD TEMP_H,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	SUB TEMP_L,r1
	SBC TEMP_H,r0
	;Проверка на размер стека более 255 байт
	TST TEMP_H
	BREQ PC+0x04
	LDI TEMP,_CORE5277_FAULT_HUGE_TASK_STACK
	MCALL CORE5277_LOG_COREFAULT
	STD Z+_CORE5277_TASK_STACK_SIZE,TEMP_L

	;Запись временной метки
	LDS TEMP,_CORE5277_UPTIME+0x04
	STD Z+_CORE5277_TASK_TIMESTAMP,TEMP

	;Восстанавливем стек ядра
	LDS r5,_CORE5277_CORE_STACK+0x00
	STS SPH,r5
	LDS r5,_CORE5277_CORE_STACK+0x01
	STS SPL,r5
	RJMP CORE5277_READY__END

CORE5277_READY__IS_DRIVER:
	POP r0
	POP r1
	;Извлекаем точку входа на сновной код драйвера
	MOV PID,_PID
	MCALL _CORE5277_PROC_HEADER_GET

	;Записываем адрес основной процедуры драйвера
	STD Z+_CORE5277_DRIVER_OFFSET+0x00,r0
	STD Z+_CORE5277_DRIVER_OFFSET+0x01,r1

CORE5277_READY__END:
	;Записываем состояние не меняя опций
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_CORE5277_PROC_STATE_READY
	STD Z+_CORE5277_PROC_STATE,TEMP
	;Выхожу из процедуры CORE5277_CREATE
	RET

;--------------------------------------------------------
CORE5277_EXEC:
;--------------------------------------------------------
;Выполнение процедуры драйвера
;IN: TEMP - ид вызываемого драйвера
;--------------------------------------------------------
	PUSH PID

	;Процедура применима только для драйвера
	SBRS TEMP,CORE5277_PROCID_OPT_DRV
	RJMP _CORE5277_EXEC__DONE

	MCALL CORE5277_DISPATCHER_LOCK
	;Сохраняем значение Z
	MOV _CORE5277_DRIVER_EXEC_ZH,ZH
	MOV _CORE5277_DRIVER_EXEC_ZL,ZL
	;Размещаем в стек адрес возврата из вызываемой процедуры
	LDI ZH,high(_CORE5277_EXEC__DONE)
	LDI ZL,low(_CORE5277_EXEC__DONE)
	PUSH ZL
	PUSH ZH
	;Получаем заголовок вызываемого драйвера
	MOV PID,TEMP
	MCALL _CORE5277_PROC_HEADER_GET
	PUSH TEMP
	LDD TEMP,Z+_CORE5277_DRIVER_OFFSET+0x00
	LDD ZL,Z+_CORE5277_DRIVER_OFFSET+0x01
	MOV ZH,TEMP
	POP TEMP
	;Помещаем в стек адрес перехода на процедуру вызываемого драйвера
	PUSH ZL
	PUSH ZH
	;Восстанавливаем значение Z
	MOV ZH,_CORE5277_DRIVER_EXEC_ZH
	MOV ZL,_CORE5277_DRIVER_EXEC_ZL
	MCALL CORE5277_DISPATCHER_UNLOCK
	;Перехожим на процедуру драйвера
	RET
_CORE5277_EXEC__DONE:
	POP PID

	RET

;--------------------------------------------------------
_CORE5277_RESUME:
;--------------------------------------------------------
;Продолжаем выполнение задачи
;IN: PID - ид задачи, Z - адрес заголовка задачи
;--------------------------------------------------------
	MCALL CORE5277_DISPATCHER_LOCK
	;Записываем адрес стека ядра
	LDS TEMP,SPH
	STS _CORE5277_CORE_STACK+0x00,TEMP
	LDS TEMP,SPL
	STS _CORE5277_CORE_STACK+0x01,TEMP

	LDS TEMP,_CORE5277_UPTIME+0x04
	STD Z+_CORE5277_TASK_TIMESTAMP,TEMP

	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0xf0
	ORI TEMP,_CORE5277_PROC_STATE_BUSY
	STD Z+_CORE5277_PROC_STATE,TEMP

	;Переключаем стек на вершину стека, он же стек текущей задачи
	MCALL _CORE5277_STACK_TO_TOP
	LDD TEMP, Z+_CORE5277_TASK_STACK_SIZE
	LDD TEMP_L,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	LDD TEMP_H,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	SUB TEMP_L,TEMP
	SBCI TEMP_H,0x00
	STS SPH,TEMP_H
	STS SPL,TEMP_L

	;Восстанавливаю SREG
	POP TEMP
	STS SREG,TEMP
	_CORE5277_MACRO__POP_RDS
	SEI
	MCALL CORE5277_DISPATCHER_UNLOCK
	RET

;--------------------------------------------------------
_CORE5277_TASK_ENDPOINT:
;--------------------------------------------------------
;Отрабатывает при завершении задачи
;--------------------------------------------------------
	MCALL CORE5277_DISPATCHER_LOCK

	MOV PID,_PID
	MCALL _CORE5277_PROC_HEADER_GET

	LDI TEMP,0x00
	STD Z+_CORE5277_TASK_STACK_SIZE,TEMP
	MCALL _CORE5277_STACK_TO_TOP
	LDI TEMP,_CORE5277_PROC_STATE_ABSENT
	STD Z+_CORE5277_PROC_STATE,TEMP

	LDS TEMP,_CORE5277_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_CORE5277_CORE_STACK+0x01
	STS SPL,TEMP

	MCALL CORE5277_DISPATCHER_UNLOCK
	RET

;--------------------------------------------------------
CORE5277_SUSPEND:
;--------------------------------------------------------
;Приостанавливаем текущую задачу
;IN: _PID
;--------------------------------------------------------
	LDS r0,SREG
	;Записываем в стек 16 старших регистров(могут быть проинициализированы при вызове процедуры)
	_CORE5277_MACRO__PUSH_RDS
	;Записываем SREG
	PUSH r0

	LDI TEMP,_CORE5277_PROC_STATE_READY
	MCALL _CORE5277_SUSPEND__BODY
	RET
;--------------------------------------------------------
_CORE5277_SUSPEND__BODY:
	MOV PID,_PID														;Затираю PID, так как он уже должен быть записан в стек
	MCALL _CORE5277_PROC_HEADER_GET

	LDD ACCUM,Z+_CORE5277_PROC_STATE
	ANDI ACCUM,0xf0
	OR ACCUM,TEMP
	STD Z+_CORE5277_PROC_STATE,ACCUM

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
	LDS TEMP,_CORE5277_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_CORE5277_CORE_STACK+0x01
	STS SPL,TEMP

	;проверяем на выход стека в зону общего буфера задач
	;TODO нет проверки на защитный порог(_CORE5277_RAM_BORDER_SIZE)
	LDS TEMP,_CORE5277_TOP_OF_FREE_RAM+0x00
	CP TEMP,TEMP_H
	BRCS _CORE5277_SUSPEND__CORRECT_STACK_SIZE
	LDI TEMP,_CORE5277_FAULT_HUGE_STACK
	MJMP CORE5277_LOG_COREFAULT									;Стеки значения не имеют, ядро упало
	LDS TEMP,_CORE5277_TOP_OF_FREE_RAM+0x01
	CP TEMP,TEMP_L
	BRCS _CORE5277_SUSPEND__CORRECT_STACK_SIZE
	LDI TEMP,_CORE5277_FAULT_HUGE_STACK
	MJMP CORE5277_LOG_COREFAULT									;Стеки значения не имеют, ядро упало
_CORE5277_SUSPEND__CORRECT_STACK_SIZE:

	;Получаем начало стека задачи
	LDD TEMP_EH,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	LDD TEMP_EL,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	;Считаем новую длину
	SUB TEMP_EL,TEMP_L
	SBC TEMP_EH,TEMP_H
	TST TEMP_EH
	BREQ PC+0x04
	LDI TEMP,_CORE5277_FAULT_HUGE_TASK_STACK
	MCALL CORE5277_LOG_COREFAULT									;Стеки значения не имеют, ядро упало

	;Обновляем длину стека задачи
	STD Z+_CORE5277_TASK_STACK_SIZE,TEMP_EL
	;Обновляем вершину стека задач
	STS _CORE5277_TOP_OF_STACK+0x00,TEMP_H
	STS _CORE5277_TOP_OF_STACK+0x01,TEMP_L

	MCALL CORE5277_DISPATCHER_UNLOCK
	;Восстанавливаем адрес процедуры, которая нас вызвала
	PUSH _RESULT_L
	PUSH _RESULT_H
	RET

;--------------------------------------------------------
_CORE5277_PROC_HEADER_GET:
;--------------------------------------------------------
;Возвращает смещение на заголовок процесса
;IN:PID-ид драйвера
;OUT:Z-смещение
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	SBRS PID,CORE5277_PROCID_OPT_DRV
	RJMP _CORE5277_PROC_HEADER_GET__TASK
_CORE5277_PROC_HEADER_GET__DRIVER:
	LDI ZH,high(_CORE5277_DRIVERS_HEADER)
	LDI ZL,low(_CORE5277_DRIVERS_HEADER)
	MOV TEMP_H,PID													;Умножаем на 0x06(_CORE5277_DRIVER_HEADER_SIZE)
	ANDI TEMP_H,0x0f
	LSL TEMP_H
	MOV TEMP_L,TEMP_H
	LSL TEMP_H
	ADD TEMP_L,TEMP_H
	RJMP _CORE5277_PROC_HEADER_GET__END
_CORE5277_PROC_HEADER_GET__TASK:
	LDI ZL,low(_CORE5277_TASKS_HEADER)
	LDI ZH,high(_CORE5277_TASKS_HEADER)
	MOV TEMP_H,PID													;Умножаем на 0x0b(_CORE5277_TASK_HEADER_SIZE)
	ANDI TEMP_H,0x0f
	MOV TEMP_L,TEMP_H
	LSL TEMP_L
	ADD TEMP_H,TEMP_L
	LSL TEMP_L
	LSL TEMP_L
	ADD TEMP_L,TEMP_H
_CORE5277_PROC_HEADER_GET__END:
	ADD ZL,TEMP_L
	CLR TEMP_H
	ADC ZH,TEMP_H

	POP TEMP_L
	POP TEMP_H
	RET

;------------------------------------------------05.09.20
_CORE5277_STACK_TO_TOP:
;--------------------------------------------------------
;Переносим стек задачи наверх
;IN: Z-адрес на заголовок задачи
;--------------------------------------------------------
	PUSH_Z
	PUSH PID
	;Проверка если стек задачи и так наверху
	;Получаем размер стека текущей задачи
	LDD TEMP_EL,Z+_CORE5277_TASK_STACK_SIZE
	LDD TEMP_H,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	SUB TEMP_L,TEMP_EL
	SBCI TEMP_H,0x00
	LDS TEMP,_CORE5277_TOP_OF_STACK+0x00
	CP TEMP_H,TEMP
	BRNE PC+0x04
	LDS TEMP,_CORE5277_TOP_OF_STACK+0x01
	CP TEMP_L,TEMP
	BREQ _CORE5277_STACK_TO_TOP__END

	;Получаем старое смещение на стек текущей задачи
	LDD XH,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	LDD XL,Z+_CORE5277_TASK_STACK_OFFSET+0x01

	;Получаем вершину стека задач
	LDS TEMP_H,_CORE5277_TOP_OF_STACK+0x00
	LDS TEMP_L,_CORE5277_TOP_OF_STACK+0x01
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x00,TEMP_H
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x01,TEMP_L
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	PUSH_Z
	;Копируем стек задачи наверх
	;TODO из смещений нужно вычесть длину стека задачи(+/-1)
	MOV LOOP_CNTR,TEMP_EL
	MCALL CORE5277_RAM_COPY8_DESC

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
	MCALL CORE5277_RAM_COPY16_DESC	;TODO - регистры не менялись???
	POP_X																	;X теперь хранит страое смещение на стек задачи

	LDI PID,0x00
_CORE5277_STACK_TO_TOP__LOOP:
	MCALL _CORE5277_PROC_HEADER_GET

	;Ели задачи нет, пропускаем
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_ABSENT
	BREQ _CORE5277_STACK_TO_TOP__NEXT_TASK

	;Проверка, если адрес стека задачи в цикле больше чем адрес стека основной задачи(X), то пропускаем(равенство исключено)
	LDD TEMP_H,Z+_CORE5277_TASK_STACK_OFFSET+0x00
	CP XH,TEMP_H
	BRCS _CORE5277_STACK_TO_TOP__NEXT_TASK
	LDD TEMP_L,Z+_CORE5277_TASK_STACK_OFFSET+0x01
	CP XL,TEMP_L
	BRCS _CORE5277_STACK_TO_TOP__NEXT_TASK

	;Изменяем смещение на длину стека основной задачи
	ADD TEMP_L,TEMP_EL
	CLR TEMP
	ADC TEMP_H,TEMP
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x00,TEMP_H
	STD Z+_CORE5277_TASK_STACK_OFFSET+0x01,TEMP_L

_CORE5277_STACK_TO_TOP__NEXT_TASK:
	INC PID
	CPI PID,_CORE5277_TASKS_MAX_QNT
	BRNE _CORE5277_STACK_TO_TOP__LOOP

_CORE5277_STACK_TO_TOP__END:
	POP PID
	POP_Z
	RET
