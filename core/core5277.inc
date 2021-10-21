;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;02.08.2020	w5277c@gmail.com			Тут просто дохрена изменений
;09.08.2020	w5277c@gmail.com			Тут просто дохрена изменений
;06.09.2020	w5277c@gmail.com			Восстановлены механизмы многопоточности
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;28.10.2020	w5277c@gmail.com			Багфикс программных таймеров
;10.11.2020	w5277c@gmail.com			Исправлена критическая ошибка в _C5_STACK_TO_TOP
;11.11.2020	w5277c@gmail.com			Добавлена метка реинициализации (не влияет на UPTIME)
;22.11.2020	w5277c@gmail.com			v0.2.8	Механизм выдерживания паузы использует время запуска задачи вместо дельты
;															времени
;22.12.2020	w5277c@gmail.com			Исправлена работа TIMER-TASK
;06.01.2021	w5277c@gmail.com			v0.2.9	Исправлена критическая ошибка в _C5_RESUME (перетирался SREG задачи)
;22.01.2021	w5277c@gmail.com			v0.3.0	Различные мелкие (и не очень) изменения
;01.02.2021	w5277c@gmail.com			v0.3.1	Учтено разное начало RAM, мелкие доработки
;06.02.2021	w5277c@gmail.com			v0.3.2	Примитивы и проверка на отсутствие драйверов
;16.04.2021	w5277c@gmail.com			v0.3.3	Вынесен C5_WAIT_2MS из диспетчера, дефолтное значение для BUFFER_SIZE
;24.05.2021	w5277c@gmail.com			v0.3.4	Исправлена ошибка в работе программных таймеров
;29.05.2021	w5277c@gmail.com			v0.3.5	Ввод TS_MODE, выделены константы C0x00,C0xff и прочее.
;31.05.2021	w5277c@gmail.com			v0.3.6	Корректная проверка на выход стека в зону выделяемой памяти
;14.06.2021	w5277c@gmail.com			v0.3.7	Запрещены прерывания при обработке таймеров, конечный обработчик сам
;															принимает решение для разрешения прерываний. Иначе возникают трудно
;															разрешимые коллизии.
;29.06.2021	w5277c@gmail.com			v0.3.8	Множественные, мелкие изменения в основном TIMER_C, введен OPT_MODE
;22.08.2021	w5277c@gmail.com			Атомарная операция записи адреса вершины стека.
;05.09.2021	w5277c@gmail.com			INPUT_PORT->C5_IN_PORT
;16.09.2021	w5277c@gmail.com			v0.3.9	Множественные, незначительные изменения.
;22.09.2021	w5277c@gmail.com			v0.4.0	Все программные таймеры содержат адрес выделенной памяти
;09.10.2021	w5277c@gmail.com			v0.4.1	Оптимизация, убраны NOP из макросов MCALL и MJMP
;-----------------------------------------------------------------------------------------------------------------------
;Задача имеет свой собственный стек, драйвер нет. Задача вызывает драйвер, диспетчер оперирует задачами.
;Стеки задач и выделяемая память динамические, сверху стеки задач, свободное пространство, затем выделяемая память.
;Для завершения задачи достаточно вызвать RET
;Драйвер не имеет персональных регистров и стека как у задачи, выполнение кода драйвера обеспечивается прерываниями или вызовами
;из задач или других драйверов.
;Идентификатор задачи или драйвера назначается разработчиком
;Выполнение процедуры драйвера может быть первано диспетчером, так как работает в инстантсе задачи.
;Все процедуры вызываемые прерываниями должны быт максимально короткими(иначе могут негативно влиять друг на друга)
;Ядро использует один из таймеров, обычно TIMER0, остальные - свободны(если не заявлено использование timer C).
;Стоит использовать только в исключительных ситуациях, т.к. использование не универсально и не имеет отношения к ядру
;В пределах процедуры можно использовать RJMP, в остальных случаях макросы MJMP и MCALL
;Осторожно с записями типа 'BREQ PC+0x02' макрос MJMP и MCALL занимает 1/2 слова, т.е. BREQ PC+0x01+_MCALL_SIZE
;Некоторые драйверы имеют жесткую привязку к портам, например SD, так как процедуры PORT... для них слишком медленны
;Функционал драйверов (также как и различные дополнительные процедуры) дорабатывается по мере необходимости

;Обработчики программных таймеров и таймера C могут не сохранять значения регистров ZH,ZL

;TODO нужна еще одна модель динамической памяти но с 16 битным размером. Плюс возможность задавать модель в main-е.
;TODO проверить использование _PID, специално введен, чтобы драйвер знал кто его вызвал (не нужно передавать отдельно
;как параметр)
;TODO добавить проверку на превышение максимального кол-ва задач и драйверов при создании процесса
;TODO проинспектировать код ядра на необходимость блокировки ресурсов, проверить процедуры вида C5_RES_LOCK.
;TODO в таблице прерываний отсчет начинается с 1, похоже выделяю памяти на 3 байта больше, чем нужно
;TODO убрать работу с таймером C в блоке инициализации драйверов
;TODO добавить блокировку ресурса 'таймер B' и 'таймер C' в драйвера
;TODO добавить в процедуры работы с программными таймерами  TIMER_B. Сделать так, чтобы была возможность использовать
;драйвера без прерываний по таймеру. Например I2C_MS - нет необходимости использовать таймеры, достаточно разрешить
;диспетчер и использовать паузы из таймера B(TID_TIMER_B = 0x82?)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef TS_MODE
.else
	.ifdef REALTIME
		.EQU	TS_MODE								= REALTIME
	.else
		.EQU	TS_MODE								= TS_MODE_EVENT
	.endif
.endif

;---CORE-PUBLIC-PROCS------------------------------------;Доступные процедуры ядра
	.EQU	C5_PROC_EXEC							= 0x01			;Запуск процедуры драйвера
	.EQU	C5_PROC_WAIT_2MS						= 0x02			;Ожидание *2мс
	.EQU	C5_PROC_DISPATCHER_LOCK				= 0x03			;Блокировка диспетчера
	.EQU	C5_PROC_DISPATCHER_UNLOCK			= 0x04			;Разблокировка диспетчера
	.EQU	C5_PROC_SUSPEND						= 0x05			;Передать управление следующей задаче

;---PROC-STATES------------------------------------------;Состояния задачи (PROC-STATES-младший ниббл, PROC-MODE-старший ниббл)
	.EQU	_C5_PROC_STATE_ABSENT				= 0x00			;Нет задачи
	.EQU	_C5_PROC_STATE_GHOST					= 0x01			;Инициализируется или деактивация
	.EQU	_C5_PROC_STATE_BUSY					= 0x02			;Выполняется
	.EQU	_C5_PROC_STATE_TIME_WAIT			= 0x03			;Ждет истечения времени(только для задач)
	.EQU	_C5_PROC_STATE_RES_WAIT				= 0x04			;Ждет освобождения ресурса(только для задач)
	.EQU	_C5_PROC_STATE_TIMER_WAIT			= 0x05			;Ждет истечения времени для нового запуска
	.EQU	_C5_PROC_STATE_READY					= 0x06			;Готова к выполнению
;---PROC-STATE-OPTIONS-----------------------------------;Опции процесса, размещаемые в статусе процесса(старшйи ниббл)
	.EQU	_C5_PROCST_OPT_NOSUSP				= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	_C5_PROCST_OPT_RESERV1				= 0x05			;Зарезервировано
	.EQU	_C5_PROCST_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	_C5_PROCST_OPT_RESERV3				= 0x07			;Зарезервировано

;---TIMER-STRUCTURE--------------------------------------;Структура таймера(8B)
	.EQU	_C5_TIMER_STRUCT_SIZE				= 0x08			;Длина блока таймера
	.EQU	_C5_TIMER_PROC_ID						= 0x00			;1B-ид процедуры(0-..., 0xff - не исп.)
	.EQU	_C5_TIMER_HANDLER						= 0x01			;2B-адрес обработчика
	.EQU	_C5_TIMER_RAM_ADDR					= 0x03			;2B-адрес на выделенную память
	.EQU	_C5_TIMER_CNTR							= 0x05			;1B-счетчик таймера
	.EQU	_C5_TIMER_DEFAULT						= 0x06			;1B-порог 0x01-0x7f, 7-бит:период отсчета,H:0.002s,L:0.00005s)
	.EQU	_C5_TIMER_RESERVED					= 0x07			;1B-зарезервировано

;---PROC-STRUCTURE---------------------------------------;Заголовок процедуры(6B-драйвер, 11B-задача)
	.EQU	_C5_PROC_STATE							= 0x00			;1B-статус процедуры(младший ниббл) и опции(старший ниббл)
	.EQU	_C5_PROC_RAM_OFFSET					= 0x01			;2B-адрес начала выделенной памяти
	.EQU	_C5_PROC_RAM_SIZE						= 0x03			;1B-размер выделенной памяти
	.EQU	_C5_DRIVER_OFFSET						= 0x04			;2B-адрес процедуры драйвера или
	.EQU	_C5_TASK_STACK_OFFSET				= 0x04			;2B-адрес начала стека задачи
	.EQU	_C5_TASK_STACK_SIZE					= 0x06			;1B-размер стека задачи
	.EQU	_C5_TASK_TIMESTAMP					= 0x07			;1B-мекта времени переключения на задачу
	.EQU	_C5_TASK_EXECTIME						= 0x08			;3B-время запуска задачи(~9 часов максимум, тик в 2мс)

;---DRIVER-RESULT----------------------------------------;Идентификаторы результата работы драйвера
	.EQU	DRV_RESULT_OK							= 0x80			;Ок
	.EQU	DRV_RESULT_ABSENT						= 0x81			;Датчик отсутствует
	.EQU	DRV_RESULT_ERROR						= 0x82			;Ошибка выполнения
	.EQU	DRV_RESULT_TIMEOUT					= 0x83			;Не получен ответ за определенный таймаут
	.EQU	DRV_RESULT_INVALID_DATA				= 0x84			;Не корректные данные(например, не сошлось CRC)
	.EQU	DRV_RESULT_UNSUPPORTED				= 0x85			;Не поддерживается

;---CONSTANTS--------------------------------------------
	.EQU	_C5_DRIVER_HEADER_SIZE				= 0x06			;Размер заголовка драйвера
	.EQU	_C5_TASK_HEADER_SIZE					= 0x0b			;Размер заголовка задачи
.IF TIMERS_SPEED == TIMERS_SPEED_25US
	.EQU	_C5_TIMER_PERIOD						= 0x50			;80, т.е. 0.000025*80=0.002=2мс
.ELSE
	.EQU	_C5_TIMER_PERIOD						= 0x28			;40, т.е. 0.000050*40=0.002=2мс
.ENDIF
	.EQU	_C5_TASK_STACK_DEF_SIZE				= 0x13			;Размер стека задачи при создании (16 регистров+SREG+точка возврата)
	.EQU	_C5_TASKS_ACTIVE_TIME				= 0x01			;Время работы задачи 1*2=2мс
	.EQU	_C5_IRV_TABLE_SIZE					= 0x4b			;Размер таблицы прерываний
	.EQU	TID_TIMER_C								= 0x83			;Идентификатор тамера C
	;---CORE-FAULTS---
	.EQU	_C5_FAULT_HUGE_STACK					= 0x01			;Стек перешел границу выделяемой памяти
	.EQU	_C5_FAULT_HUGE_TASK_STACK			= 0x02			;Размер стека задачи более 255 байт
	.EQU	_C5_FAULT_HUGE_TASK_STACK2			= 0x03			;Размер стека задачи более 255 байт
	;---CORE-RESOURCES---											;Идентфикаторы ресурсов, 0-127 отданы пользователю, старшие-ядру
	.EQU	C5_RES_TIMER_B							= 0xFE			;Таймер B
	.EQU	C5_RES_ADC								= 0xFD			;ADC
	;---
;---CORE-FLAGS-------------------------------------------
	.EQU	_CFL_TIMER_B_USE						= 0x00			;Флаг запущенного таймера B
	.EQU	_CFL_TIMER_UART_CORRECTION			= 0x01			;Корректировка таймера для соответствия частотам UART
	.EQU	_CFL_DISPATCHER_ORDER				= 0x02			;Заявка на отработку диспетчера(формируется когда диспетчер заблокирован)
	.EQU	CFL_VIDEO_SYNC							= 0x03			;

;---RAM-OFFSETS------------------------------------------
	.EQU	_C5_UPTIME								= RAMSTART+0x00;5B-отсчет времени (~75 лет при тике каждые 0.002мс)
	.EQU	_C5_CORE_STACK							= RAMSTART+0x05;2B-хранит смещение стека ядра
	.EQU	_C5_TOP_OF_STACK						= RAMSTART+0x07;2B-хранит вершину стека
	.EQU	_C5_TOP_OF_FREE_RAM					= RAMSTART+0x09;2B-хранит вершину свободной памяти
	.EQU	_C5_FLAGS								= RAMSTART+0x0b;4B-флагов (32 флага)
	.EQU	_C5_TIMER_C_PID						= RAMSTART+0x0f;1B-PID задачи использующей таймер C
	.EQU	_C5_MAIN_TIMER_CNTR					= _C5_TIMER_C_PID+0x01							;1B-счетчик таймера ядра (каждые 0.000050с)
	.EQU	_C5_MAIN_TIMER_TRHLD					= _C5_MAIN_TIMER_CNTR+0x01						;1B-порог срабатывания таймера ядра
.IFDEF C5_IN_PORT
	.EQU	_C5_INPUT_BUFFER						= _C5_MAIN_TIMER_TRHLD+0x01					;xB-буфер ввода
	.EQU	_C5_INPUT_BUFFER_POS					= _C5_INPUT_BUFFER+INPUT_BUFFER_SIZE		;1B-позиция в буфере на свободный элемент
	.EQU	_C5_TIMERS								= _C5_INPUT_BUFFER+INPUT_BUFFER_SIZE+0x01	;xB-таблица таймеров(TIMERS*8)
.ELSE
	.EQU	_C5_TIMERS								= _C5_MAIN_TIMER_TRHLD+0x01					;xB-таблица таймеров(TIMERS*8)
.ENDIF																	;TIMERS*8
	.EQU	_C5_IR_VECTORS_TABLE					= _C5_TIMERS+TIMERS*8
																			;xB-таблица прерываний (_C5_IR_QNT*3 для каждого прерывания, исключая RESET)
	.EQU	_C5_DRIVERS_HEADER					= _C5_IR_VECTORS_TABLE+_C5_IR_QNT*3
																			;xB-заголовки драйвера (X=C5_DRIVERS_QNT*_C5_DRIVER_HEADER_SIZE)
	.EQU	_C5_TASKS_HEADER						= _C5_DRIVERS_HEADER+C5_DRIVERS_QNT*_C5_DRIVER_HEADER_SIZE
																			;xB-заголовки задач (X=C5_TASKS_QNT*_C5_TASK_HEADER_SIZE)
	.EQU	_C5_RESOURCE_QUEUE					= _C5_TASKS_HEADER+C5_TASKS_QNT*_C5_TASK_HEADER_SIZE
																			;xB-очередь задач к ресурсам(1B-RES_ID, 1B-TASK_ID...)
	.EQU	C5_BUFFER								= _C5_RESOURCE_QUEUE+_C5_RES_QUEUE_SIZE
	.EQU	_C5_FREE_RAM							= C5_BUFFER+BUFFER_SIZE
	.EQU	_C5_STACK_END							= RAMEND-_C5_STACK_SIZE	;xB-стек ядра, далее буфер, стек задач и выделяемая память
														;TODO проверить что стек не перетирает первый байт буфера
.IF AVRA == 0x01
	.MESSAGE "######## BUFFER OFFSET:",C5_BUFFER
	.IF BUFFER_SIZE > 0x00
		.MESSAGE "######## BUFFER SIZE:",BUFFER_SIZE
	.ENDIF
	.MESSAGE "######## AVAILABLE RAM:",((_C5_STACK_END-_C5_FREE_RAM)-_C5_RAM_BORDER_SIZE)
	.MESSAGE "######## DRIVERS HEADERS OFFSET:",_C5_DRIVERS_HEADER
	.MESSAGE "######## TASKS HEADERS OFFSET:",_C5_TASKS_HEADER
	.MESSAGE "######## FREE RAM OFFSET:",_C5_FREE_RAM
	.MESSAGE "######## STACK END OFFSET:",_C5_STACK_END
	.MESSAGE "######## CORE FREQ:",CORE_FREQ
	.IF TIMERS_SPEED == TIMERS_SPEED_50US
		.MESSAGE "######## TIMERS NORMAL SPEED (",TIMERS_SPEED,")"
	.ELSEIF TIMERS_SPEED == TIMERS_SPEED_25US
		.MESSAGE "######## TIMERS x2 SPEED (",TIMERS_SPEED,")"
	.ELSE
		.MESSAGE "######## TIMERS WRONG SPEED"
	.ENDIF
.ELSE
	.IF TIMERS_SPEED == TIMERS_SPEED_50US
		.MESSAGE "######## TIMERS NORMAL SPEED"
	.ELSE
		.IF TIMERS_SPEED == TIMERS_SPEED_25US
			.MESSAGE "######## TIMERS x2 SPEED"
		.ELSE
			.MESSAGE "######## TIMERS WRONG SPEED"
		.ENDIF
	.ENDIF
.ENDIF
.IF TIMER_C_ENABLE == 0x01
	.MESSAGE "######## TIMER C ENABLED"
.ENDIF

;---REGISTERS--------------------------------------------;r0-r14 выделено для нужд ядра
	.DEF	_RESULT_L								= r0
	.DEF	_RESULT_H								= r1
	;...r2-r6 используются в диспетчере
	.DEF	_C5_DISPATCHER_LOCK_CNTR			= r9				;Счетчик вложенных блокировок диспетчера
	.DEF	_C5_DRIVER_EXEC_ZH					= r10
	.DEF	_C5_DRIVER_EXEC_ZL					= r11
	.DEF	_C5_TEMP_H								= r12
	.DEF	_C5_TEMP_L								= r13
	.DEF	_C5_COREFLAGS							= r14				;Флаги ядра
	.DEF	_PID										= r15				;ИД корневого процесса(обычно задачи)


	.INCLUDE	"./mem/ram_copy.inc"
	.INCLUDE	"./mem/ram_fill16.inc"
.if TS_MODE != TS_MODE_NO
	.INCLUDE "./mem/ram_copy_desc.inc"
	.INCLUDE "./mem/ram_copy16_desc.inc"
.endif
	.INCLUDE "./conv/bitnum_to_num.inc"
.IFDEF C5_IN_PORT
	.INCLUDE "./core/ui/input.inc"
.ENDIF

;---OPTIONS-ANALYZE--------------------------------------
.IF AVRA == 0x01
	.MESSAGE "######## IO BAUDRATE:",14400*CORE_FREQ
.ENDIF
;TODO unexpected INTEGER, expecting STRING in AtmelStudio
; 	.MESSAGE 28800*CORE_FREQ
.IFDEF LOGGING_PORT
	.MESSAGE "######## LOGGING ENABLED"
	OUTSTR_CORE:
	.db   0x0d,0x0a,"Powered by core5277 v0.4.1 [",0x00,0x00
	OUTSTR_MCUSR_NO:
	.db   "NO ",0x00
	OUTSTR_MCUSR_WD:
	.db   "WD ",0x00
	OUTSTR_MCUSR_BO:
	.db   "BO ",0x00
	OUTSTR_MCUSR_EX:
	.db   "EX ",0x00
	OUTSTR_MCUSR_PO:
	.db   "PO ",0x00
	OUTSTR_MCUSR_RESET:
	.db   "reset]",0x0d,0x0a,0x00,0x00

	.INCLUDE "./core/io/out_init.inc"
	.INCLUDE	"./core/io/out_romstr.inc"
	.INCLUDE	"./core/io/out_tasksdump.inc"
	.INCLUDE "./core/io/out_corefault.inc"
.ELSE
	.MESSAGE "######## LOGGING DISABLED"
	.INCLUDE "./core/io/out_fake.inc"
.ENDIF
.IFDEF C5_IN_PORT
	.MESSAGE "######## INPUT ENABLED"
.ENDIF
.IF TS_MODE == TS_MODE_TIME
	.MESSAGE "######## DISPATCHER ENABLED"
	.INCLUDE "./core/_dispatcher.inc"
.ELSE
	.MESSAGE "######## DISPATCHER DISABLED"
	.INCLUDE "./core/wait_2ms.inc"
C5_DISPATCHER_LOCK:
C5_DISPATCHER_UNLOCK:
	RET
.ENDIF

.IF TIMERS > 0x00 || TIMER_C_ENABLE == 1
	.MESSAGE "######## TIMERS ENABLED"
	.INCLUDE "./core/timer_set.inc"
	.INCLUDE "./core/timer_start.inc"
	.INCLUDE "./core/timer_start_at.inc"
	.INCLUDE "./core/timer_stop.inc"
.ELSE
	.MESSAGE "######## TIMERS DISABLED"
	C5_TIMER_SET:
	C5_TIMER_START:
	C5_TIMER_START_AT:
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
	;Получаем адрес по которому определяем тип прерывания
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
	LDI_Z _C5_IR_VECTORS_TABLE

.if _MCALL_SIZE == 2
	LSR _C5_TEMP_L
.endif
	DEC _C5_TEMP_L														;Получено на базе адреса выхода из процедуры(не входа), т.е. увеличен на 1

	MOV ACCUM,_C5_TEMP_L												;Номер строки в таблице перываний
	DEC _C5_TEMP_L														;Первая запись в таблице прерываний не используется
	BREQ PC+0x04
	MOV TEMP,_C5_TEMP_L												;Умножаем на 3(1B-PID, 2B-адрес перехода)
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
	;Переходим, если адрес указан(в ACCUM номер строки таблицы прерываний)
	ICALL																	;Возвращаться надо по RET(не RETI), TEMP и SREG сохранять не нужно
	POP _PID
_C5_IR__END:
	;Восстанавливаем регистры и выходим
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
;Обработчик прерывания основного таймера
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP

	LDI TEMP,TIMERS_SPEED-0x02
	SBRC _C5_COREFLAGS,_CFL_TIMER_UART_CORRECTION
	LDI TEMP,TIMERS_SPEED+0x02
	_CORRE5277_TIMERA_CORRECTOR TEMP

	PUSH_Z

.IF TIMERS > 0x00
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH PID
	PUSH_Y

	LDI_Y _C5_TIMERS
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
	STD Y+_C5_TIMER_CNTR,TEMP
	CPI TEMP,0x00
	BRNE _C5_TIMER_A_IR__HF_NEXT
	LDD ZH,Y+_C5_TIMER_HANDLER+0x00
	LDD ZL,Y+_C5_TIMER_HANDLER+0x01
	PUSH_Y
	LDD TEMP,Y+_C5_TIMER_RAM_ADDR+0x00
	LDD YL,Y+_C5_TIMER_RAM_ADDR+0x01
	MOV YH,TEMP
	ICALL
	POP_Y
	LDD TEMP,Y+_C5_TIMER_DEFAULT
	ANDI TEMP,0x7f
	STD Y+_C5_TIMER_CNTR,TEMP
_C5_TIMER_A_IR__HF_NEXT:
	ADIW YL,_C5_TIMER_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE _C5_TIMER_A_IR__HF_LOOP

	POP_Y
	POP PID
	POP LOOP_CNTR
	POP ACCUM
.ENDIF
_C5_TIMER_IR__DISPATCHER:

	PUSH ACCUM
	LDS ACCUM,_C5_MAIN_TIMER_CNTR
	INC ACCUM
	STS _C5_MAIN_TIMER_CNTR,ACCUM
	LDS TEMP,_C5_MAIN_TIMER_TRHLD
	CP TEMP,ACCUM
	POP ACCUM
	BREQ PC+0x04
	POP_Z
	RJMP _C5_TIMER_A_IR__END
	SUBI TEMP,(0x100-_C5_TIMER_PERIOD)
	STS _C5_MAIN_TIMER_TRHLD,TEMP

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
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH PID
	PUSH_Y

	LDI_Y _C5_TIMERS
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
	STD Y+_C5_TIMER_CNTR,TEMP
	CPI TEMP,0x00
	BRNE _C5_TIMER_A_IR__LF_NEXT
	LDD ZH,Y+_C5_TIMER_HANDLER+0x00
	LDD ZL,Y+_C5_TIMER_HANDLER+0x01
	PUSH_Y
	LDD TEMP,Y+_C5_TIMER_RAM_ADDR+0x00
	LDD YL,Y+_C5_TIMER_RAM_ADDR+0x01
	MOV YH,TEMP
	ICALL
	POP_Y
	LDD TEMP,Y+_C5_TIMER_DEFAULT
	ANDI TEMP,0x7f
	STD Y+_C5_TIMER_CNTR,TEMP
_C5_TIMER_A_IR__LF_NEXT:
	ADIW YL,_C5_TIMER_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE _C5_TIMER_A_IR__LF_LOOP

	POP_Y
	POP PID
	POP LOOP_CNTR
	POP ACCUM
.ENDIF

	POP_Z

.IF TS_MODE == TS_MODE_TIME
	MCALL _C5_DISPATCHER_EVENT
.ENDIF
_C5_TIMER_A_IR__END:

	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RETI


.IF TIMER_C_ENABLE == 0x01
;--------------------------------------------------------
_C5_TIMER_C_IR:
;--------------------------------------------------------
;Обработчик прерывания таймера C
;--------------------------------------------------------
	PUSH_Z
	PUSH_Y
	LDS ZH,SREG
	PUSH ZH
	PUSH PID

	LDS PID,_C5_TIMER_C_PID
	LDS ZH,_C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x00
	LDS ZL,_C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x01
	LDS YH,_C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x00
	LDS YL,_C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x01
	ICALL

	POP PID
	POP ZH
	STS SREG,ZH
	POP_Y
	POP_Z
	RETI
;--------------------------------------------------------
C5_TIMER_C_SET_VECTOR:
;TODO: убрать, код вынесен в программные таймеры
;--------------------------------------------------------
;Установка адреса обработчика таймера C
;IN: PID,X-адрес обработчика,Y-адрес выделенной памяти
;--------------------------------------------------------
	STS _C5_TIMER_C_PID,PID
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x00,YH
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER1*3+0x01,YL
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x00,XH
	STS _C5_IR_VECTORS_TABLE+C5_IR_RESERVED_CORE_TIMER2*3+0x01,XL
	RET
;--------------------------------------------------------
C5_TIMER_C_SET_PERIOD:
;TODO: убрать, код вынесен в программные таймеры
;--------------------------------------------------------
;Установка периода таймера C
;IN: TEMP-длина периода
;--------------------------------------------------------
	_C5_TIMERC_SET_PERIOD
	RET
;--------------------------------------------------------
C5_TIMER_C_START_AT:
;TODO: убрать, код вынесен в программные таймеры
;--------------------------------------------------------
;Запуск таймера C
;IN: TEMP-длина периода
;--------------------------------------------------------
	PUSH TEMP
	_C5_TIMERC_SET_PERIOD
	MCALL C5_TIMER_C_START
	POP TEMP
	RET
;--------------------------------------------------------
C5_TIMER_C_START:
;TODO: убрать, код вынесен в программные таймеры
;--------------------------------------------------------
;Запуск таймера C
;--------------------------------------------------------
	_C5_TIMERC_START
	RET
;--------------------------------------------------------
C5_TIMER_C_SET_COM:
;--------------------------------------------------------
;Устанавливаю режим работы вывода(OC2A) таймера C
;IN: TEMP_H-TIMERC_COM_x
;--------------------------------------------------------
	PUSH TEMP
	_C5_TIMERC_SET_COM
	POP TEMP
	RET
;--------------------------------------------------------
C5_TIMER_C_STOP:
;TODO: убрать, код вынесен в программные таймеры
;--------------------------------------------------------
;Останов таймера C
;--------------------------------------------------------
	PUSH TEMP
	_C5_TIMERC_STOP
	POP TEMP
	RET
.ENDIF

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
	LDS FLAGS,MCUSR

	;WATCHDOG STOP
	WDR
	LDS ACCUM,MCUSR
	ANDI ACCUM,~(1<<WDRF)
	STS MCUSR,ACCUM
	LDS ACCUM,WDTCR
	ORI ACCUM,(1<<WDCE)|(1<<WDE)
	STS WDTCR,ACCUM
	LDI ACCUM,(0<<WDE)
	STS WDTCR,ACCUM

	;Инициализация логирования
.ifdef LOGGING_PORT
	MCALL C5_OUT_INIT

	LDI_Z OUTSTR_CORE|0x8000
	MCALL C5_OUT_STR
	SBRS FLAGS,WDRF
	RJMP PC+0x03+_MCALL_SIZE
	LDI_Z OUTSTR_MCUSR_WD|0x8000
	MCALL C5_OUT_STR
	SBRS FLAGS,BORF
	RJMP PC+0x03+_MCALL_SIZE
	LDI_Z OUTSTR_MCUSR_BO|0x8000
	MCALL C5_OUT_STR
	SBRS FLAGS,EXTRF
	RJMP PC+0x03+_MCALL_SIZE
	LDI_Z OUTSTR_MCUSR_EX|0x8000
	MCALL C5_OUT_STR
	SBRS FLAGS,PORF
	RJMP PC+0x03+_MCALL_SIZE
	LDI_Z OUTSTR_MCUSR_PO|0x8000
	MCALL C5_OUT_STR
	MOV TEMP,FLAGS
	ANDI TEMP,(1<<WDRF)|(1<<BORF)|(1<<EXTRF)|(1<<PORF)
	BRNE PC+0x03+_MCALL_SIZE
	LDI_Z OUTSTR_MCUSR_NO|0x8000
	MCALL C5_OUT_STR
	LDI_Z OUTSTR_MCUSR_RESET|0x8000
	MCALL C5_OUT_STR

	SBRC FLAGS,PORF
	RJMP PC+0x01+_MCALL_SIZE
	MCALL C5_OUT_TASKSDUMP
.endif

	;Сбрасываем флаги причины сброса
	CLR TEMP
	STS MCUSR,TEMP

.IF TIMER_C_ENABLE == 0x01
	_C5_TIMERC_INIT
.ENDIF

	;Заполнение всей памяти 0xF7 значением (для диагностики)
	LDI TEMP,0xF7
	LDI TEMP_H,high(RAMEND-0x010f)									;RAM_SIZE-16(stack)
	LDI TEMP_L,low(RAMEND-0x010f)
	LDI_X 0x0100
	MCALL RAM_FILL16

	;Сброс счетчика времени
	LDI TEMP,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x05
	LDI_X _C5_UPTIME
	MCALL RAM_FILL16
;--------------------------------------------------------
C5_REINIT:
;--------------------------------------------------------
;Повторная инициализация ядра
;--------------------------------------------------------
	;Чистка таблицы прерываний
	LDI TEMP,0xff
	LDI TEMP_H,0x00
	LDI TEMP_L,_C5_IRV_TABLE_SIZE									;IV_VECTORS_TABLE
	LDI_X _C5_IR_VECTORS_TABLE
	MCALL RAM_FILL16
	;Чистка очереди
	LDI TEMP,0xff
	LDI TEMP_H,0x00
	LDI TEMP_L,_C5_RES_QUEUE_SIZE
	LDI_X _C5_RESOURCE_QUEUE
	MCALL RAM_FILL16
	;Чистка переменных в памяти
	CLR _C5_COREFLAGS
	;Задаем вершину стека
	LDI_X _C5_STACK_END
	STS _C5_TOP_OF_STACK+0x00,XH
	STS _C5_TOP_OF_STACK+0x01,XL
	;Задаем вершину свободной памяти
	LDI_X _C5_FREE_RAM
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL
	;Чистка флагов
	STS _C5_FLAGS+0x00,C0x00
	STS _C5_FLAGS+0x01,C0x00
	STS _C5_FLAGS+0x02,C0x00
	STS _C5_FLAGS+0x03,C0x00
	;Задаем счетчик основному таймеру
	STS _C5_MAIN_TIMER_CNTR,C0x00
	LDI TEMP,_C5_TIMER_PERIOD
	STS _C5_MAIN_TIMER_TRHLD,TEMP
.IFDEF C5_IN_PORT
	MCALL C5_INPUT_INIT
.ENDIF
.IF TIMERS > 0x00
	;Установка таймеров как не используемых
	LDI TEMP_H,high(TIMERS*_C5_TIMER_STRUCT_SIZE)
	LDI TEMP_L,low(TIMERS*_C5_TIMER_STRUCT_SIZE)
	LDI TEMP,0xff
	LDI_X _C5_TIMERS
	MCALL RAM_FILL16
.ENDIF
	;Чистка заголовков драйверов
.IF	0x00 != C5_DRIVERS_QNT
	LDI_X _C5_DRIVERS_HEADER
	LDI TEMP_H,high(_C5_DRIVER_HEADER_SIZE*C5_DRIVERS_QNT)
	LDI TEMP_L,low(_C5_DRIVER_HEADER_SIZE*C5_DRIVERS_QNT)
	LDI TEMP,_C5_PROC_STATE_ABSENT
	MCALL RAM_FILL16
.ENDIF
	;Чистка заголовков задач
	LDI_X _C5_TASKS_HEADER
	LDI TEMP_H,high(_C5_TASK_HEADER_SIZE*C5_TASKS_QNT)
	LDI TEMP_L,low(_C5_TASK_HEADER_SIZE*C5_TASKS_QNT)
	LDI TEMP,_C5_PROC_STATE_ABSENT
	MCALL RAM_FILL16
	;Чистка регистров
	CLR _C5_DISPATCHER_LOCK_CNTR
	;Инициализируем таймер (0.000050s - 800 тактов)
	_C5_TIMERA_INIT
	RET

;--------------------------------------------------------
C5_IR_VECTOR_SET:
;--------------------------------------------------------
;Устанавливаем адрес перехода для прерывания
;IN: PID-ид процесса,
;ACCUM-номер прерывания (1-...), TEMP_H,TEMP_L-
;адрес перехода (0-не использовать перерывание)
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
	LDI_Z _C5_IR_VECTORS_TABLE
	ADD ZL,ACCUM
	ADC ZH,C0x00
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
C5_TCNT_GET:
;--------------------------------------------------------
;Получаем значение счетчика таймера
;Отсчет кратный 0.000 000 5s
;OUT: TEMP-значение счетчика таймера
;--------------------------------------------------------
	_C5_TIMER_TCNT TEMP
	RET

;--------------------------------------------------------
_C5_WAIT_500NS:
;--------------------------------------------------------
;Ждем истечения времени
;IN TEMP-время в 0.000 000 5s(>=40 ~20us, <=248)
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	;Корректировка на время исполнения кода
	SUBI TEMP,0x07
	;Настройка таймера
	_C5_TIMERB

	POP TEMP_L
	POP TEMP_H
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

;--------------------------------------------------------
C5_START:
;--------------------------------------------------------
;Запуск ядра
;--------------------------------------------------------
	;Запуск таймера
	_CORRE5277_TIMERA_START
	;Разрешаем прерывания
	SEI
.if TS_MODE == TS_MODE_NO
	;TODO простая реализаия переключения стека и переход на первую задачу
.endif
.if TS_MODE != TS_MODE_NO
_C5_START__LOOP:
	LDI PID,0x00													;Счетчик задач
	LDI FLAGS,0x00
_C5_START__TASKS_LOOP:
	MCALL _C5_PROC_HEADER_GET
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_READY
	BRNE _C5_START__NEXT_TASK
	PUSH PID
	MOV _PID,PID
	PUSH FLAGS
	MCALL _C5_RESUME												;Возвращаемся по C5_SUSPEND
	POP FLAGS
	POP PID
	ORI FLAGS,0x01
_C5_START__NEXT_TASK:
	INC PID
	CPI PID,C5_TASKS_QNT
	BRNE _C5_START__TASKS_LOOP
	SBRS FLAGS,0x00
	SLEEP
	RJMP _C5_START__LOOP
.endif
;========================================================================
;===Процедуры работы с задачами и драйверами=============================
;========================================================================
;--------------------------------------------------------
C5_CREATE:
;--------------------------------------------------------
;Создание процесса(задачи или драйвера).
;После создания будет выполнен переход на процесс
;для инициализации. После инициализации процесса
;необходимо вызвать процедуру C5_READY
;IN: PID, Z-точка входа, TEMP_H,TEMP_L,TEMP - время в 2мс
;для опции задачи C5_PROCID_OPT_TIMER
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
	STD Z+_C5_PROC_RAM_OFFSET+0x00,C0x00
	STD Z+_C5_PROC_RAM_OFFSET+0x01,C0x00
	STD Z+_C5_PROC_RAM_SIZE,C0x00

	;Проверяем на признак драйвера, пропускаем блок кода для задачи, если признак есть
	SBRC PID,C5_PROCID_OPT_DRV
	RJMP C5_CREATE__TASK_CODE_SKIP

	;Записываем начало стека на базе вершины стека задач
	LDS r0,_C5_TOP_OF_STACK+0x00
	STD Z+_C5_TASK_STACK_OFFSET+0x00,r0
	LDS r1,_C5_TOP_OF_STACK+0x01
	STD Z+_C5_TASK_STACK_OFFSET+0x01,r1
	;Записываем размер стека
	STD Z+_C5_TASK_STACK_SIZE,C0x00
	;Восстанавливаем все регистры из стека
	POP TEMP

	SBRS PID,C5_PROCID_OPT_TIMER
	RJMP C5_CREATE__TIMER_CODE_SKIP1
	;Записываем время окончания паузы
	PUSH ACCUM
	LDS ACCUM,SREG
	PUSH ACCUM
	CLI
	LDS ACCUM,_C5_UPTIME+0x04
	ADD ACCUM,TEMP
	STD Z+_C5_TASK_EXECTIME+0x02,ACCUM
	LDS ACCUM,_C5_UPTIME+0x03
	ADC ACCUM,TEMP_L
	STD Z+_C5_TASK_EXECTIME+0x01,ACCUM
	LDS ACCUM,_C5_UPTIME+0x02
	ADC ACCUM,TEMP_H
	STD Z+_C5_TASK_EXECTIME+0x00,ACCUM
	POP ACCUM
	STS SREG,ACCUM
	POP ACCUM
C5_CREATE__TIMER_CODE_SKIP1:

	POP_Z
	;Запоминаем текущее положение стека
	LDS r2,SPH
	STS _C5_CORE_STACK+0x00,r2
	LDS r2,SPL
	STS _C5_CORE_STACK+0x01,r2
	;Переключаемся на стек задачи
	STS SPH,r0
	STS SPL,r1

	;Если таймер - записываем таймаут в стек для восстановления
	SBRS PID,C5_PROCID_OPT_TIMER
	RJMP C5_CREATE__TIMER_CODE_SKIP2
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	LDS r1,SREG
	CLI
	LDS TEMP,_C5_UPTIME+0x02
	PUSH TEMP															;Выделяем дополнительно 3 байта для хранения временной метки,
	LDS TEMP,_C5_UPTIME+0x03
	PUSH TEMP															;используется при вызове процедуры RESUME,
	LDS TEMP,_C5_UPTIME+0x04
	PUSH TEMP															;т.к. EXECTIME может быть перезаписан процедурами WAIT
	STS SREG,r1
C5_CREATE__TIMER_CODE_SKIP2:

	;Помещаем точку возврата
	MOV r0,TEMP
.if TS_MODE == TS_MODE_NO
	LDI TEMP,low(SRAM_START)
.endif
.if TS_MODE != TS_MODE_NO
	LDI TEMP,low(_C5_TASK_ENDPOINT)
.endif
	PUSH TEMP
.if TS_MODE == TS_MODE_NO
	LDI TEMP,high(SRAM_START)
.endif
.if TS_MODE != TS_MODE_NO
	LDI TEMP,high(_C5_TASK_ENDPOINT)
.endif
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

	SBRC PID,C5_PROCID_OPT_DRV
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
	BREQ PC+0x02+_MCALL_SIZE
	LDI TEMP,_C5_FAULT_HUGE_TASK_STACK
	MCALL C5_OUT_COREFAULT
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
	;READY для задачи или драйвера, TIMER_WAIT - для задачи-таймера
	SBRS TEMP,C5_PROCID_OPT_TIMER
	ORI TEMP,_C5_PROC_STATE_READY
	SBRC TEMP,C5_PROCID_OPT_TIMER
	ORI TEMP,_C5_PROC_STATE_TIMER_WAIT

	STD Z+_C5_PROC_STATE,TEMP
	;Выхожу из процедуры C5_CREATE
	RET

;--------------------------------------------------------
C5_EXEC:
;--------------------------------------------------------
;Выполнение процедуры драйвера
;IN: TEMP-ид вызываемого драйвера
;--------------------------------------------------------
	PUSH PID

	;Процедура применима только для драйвера
	SBRS TEMP,C5_PROCID_OPT_DRV
	RJMP _C5_EXEC__DONE

	MCALL C5_DISPATCHER_LOCK
	;Сохраняем значение Z
	MOV _C5_DRIVER_EXEC_ZH,ZH
	MOV _C5_DRIVER_EXEC_ZL,ZL
	;Помещаем в стек адрес возврата из вызываемой процедуры
	LDI_Z _C5_EXEC__DONE
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
	;Переходим на процедуру драйвера
	RET
_C5_EXEC__DONE:
	POP PID
	RET

;--------------------------------------------------------
_C5_PROC_HEADER_GET:
;--------------------------------------------------------
;Возвращает смещение на заголовок процесса
;IN:PID-ид процеса
;OUT:Z-смещение
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	MOV TEMP_H,PID
	ANDI TEMP_H,0x0f
	SBRS PID,C5_PROCID_OPT_DRV
	RJMP _C5_PROC_HEADER_GET__TASK
_C5_PROC_HEADER_GET__DRIVER:
	LDI_Z _C5_DRIVERS_HEADER
	LSL TEMP_H														;Умножаем на 0x06(_C5_DRIVER_HEADER_SIZE)
	MOV TEMP_L,TEMP_H
	LSL TEMP_H
	RJMP _C5_PROC_HEADER_GET__END
_C5_PROC_HEADER_GET__TASK:
	LDI_Z _C5_TASKS_HEADER
	MOV TEMP_L,TEMP_H												;Умножаем на 0x0b(_C5_TASK_HEADER_SIZE)
	LSL TEMP_L
	ADD TEMP_H,TEMP_L
	LSL TEMP_L
	LSL TEMP_L
_C5_PROC_HEADER_GET__END:
	ADD TEMP_L,TEMP_H
	ADD ZL,TEMP_L
	ADC ZH,C0x00

	POP TEMP_L
	POP TEMP_H
	RET

.if TS_MODE != TS_MODE_NO
;--------------------------------------------------------
_C5_RESUME:
;--------------------------------------------------------
;Продолжаем выполнение задачи
;IN: PID-ид задачи, Z-адрес заголовка задачи
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
	CLI
	STS SPH,TEMP_H
	STS SPL,TEMP_L

	;Восстанавливаем SREG
	POP TEMP
	STS SREG,TEMP
	_C5_MACRO__POP_RDS
	SEI
	PUSH TEMP
	LDS TEMP,SREG
	MCALL C5_DISPATCHER_UNLOCK
	STS SREG,TEMP
	POP TEMP
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

	CLI
	LDS TEMP,_C5_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_C5_CORE_STACK+0x01
	STS SPL,TEMP
	SEI
	MCALL C5_DISPATCHER_UNLOCK
	RET

;--------------------------------------------------------
C5_SUSPEND:
;--------------------------------------------------------
;Приостанавливаем текущую задачу
;IN: _PID
;--------------------------------------------------------
	LDS r0,SREG
	;Помещаем в стек 16 старших регистров(могут быть проинициализированы при вызове процедуры)
	_C5_MACRO__PUSH_RDS
	;Записываем SREG
	PUSH r0

	MOV PID,_PID														;Затираем PID, так как он уже записан в стек
	MCALL _C5_PROC_HEADER_GET

	LDD TEMP,Z+_C5_PROC_STATE
	SBRS TEMP,C5_PROCID_OPT_TIMER
	RJMP __C5_SUSPEND_NO_TIMER
	;Записываем новую временную метку запуска задачи на базе времени текущего запуска и времени ожидания
	LDD YH,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD YL,Z+_C5_TASK_STACK_OFFSET+0x01
	SBIW YL,0x05
	LDD TEMP,Y+0x00
	LDD ACCUM,Y+0x3
	ADD TEMP,ACCUM
	STD Z+_C5_TASK_EXECTIME+0x02,TEMP
	LDD TEMP,Y+0x01
	LDD ACCUM,Y+0x4
	ADC TEMP,ACCUM
	STD Z+_C5_TASK_EXECTIME+0x01,TEMP
	LDD TEMP,Y+0x02
	LDD ACCUM,Y+0x05
	ADC TEMP,ACCUM
	STD Z+_C5_TASK_EXECTIME+0x00,TEMP

	LDI TEMP,_C5_PROC_STATE_TIMER_WAIT
	RJMP __C5_SUSPEND_TIMER
__C5_SUSPEND_NO_TIMER:
	LDI TEMP,_C5_PROC_STATE_READY
__C5_SUSPEND_TIMER:
	MCALL _C5_SUSPEND__BODY
	RET
;--------------------------------------------------------
_C5_SUSPEND__BODY:
	MOV PID,_PID														;Затираем PID, так как он уже должен быть записан в стек
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
	CLI
	LDS TEMP,_C5_CORE_STACK+0x00
	STS SPH,TEMP
	LDS TEMP,_C5_CORE_STACK+0x01
	STS SPL,TEMP
	SEI

	;Проверяем на выход стека в зону выделяемой памяти
	;TODO нет проверки на защитный порог(_C5_RAM_BORDER_SIZE)
	LDS TEMP,_C5_TOP_OF_FREE_RAM+0x00
	CP TEMP,TEMP_H
	BRCS _C5_SUSPEND__CORRECT_STACK_SIZE
	BRNE _C5_SUSPEND__HUGE_STACK
	LDS TEMP,_C5_TOP_OF_FREE_RAM+0x01
	CP TEMP,TEMP_L
	BRCS _C5_SUSPEND__CORRECT_STACK_SIZE
_C5_SUSPEND__HUGE_STACK:
	LDI TEMP,_C5_FAULT_HUGE_STACK
	MJMP C5_OUT_COREFAULT											;Стеки значения не имеют, ядро упало
	LDS TEMP,_C5_TOP_OF_FREE_RAM+0x01
_C5_SUSPEND__CORRECT_STACK_SIZE:

	;Получаем начало стека задачи
	LDD TEMP_EH,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_EL,Z+_C5_TASK_STACK_OFFSET+0x01
	;Считаем новую длину
	SUB TEMP_EL,TEMP_L
	SBC TEMP_EH,TEMP_H
	TST TEMP_EH
	BREQ PC+0x02+_MCALL_SIZE
	LDI TEMP,_C5_FAULT_HUGE_TASK_STACK2
	MCALL C5_OUT_COREFAULT											;Стеки значения не имеют, ядро упало

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
_C5_STACK_TO_TOP:
;--------------------------------------------------------
;Переносим стек задачи наверх
;IN: Z-адрес на заголовок задачи
;--------------------------------------------------------
	PUSH_Z
	PUSH PID
	;Проверка, если стек задачи и так наверху
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
	MCALL RAM_COPY_DESC

	;Записываем старое смещение на стек текущей задачи
	MOV ZH,XH
	MOV ZL,XL
	;Получаем смещение на следующий стек задачи
	SUB XL,TEMP_EL
	SBCI XH,0x00

	POP TEMP_L
	POP TEMP_H

	;Вычисляем размер копируемых данных(Z-старый адрес стека задачи, TEMP_H/L-вершина стека задач)
	PUSH_Z
	SUB ZL,TEMP_L
	SBC ZH,TEMP_H
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	POP_Z
	PUSH_Z
	;Копируем данные
	MCALL RAM_COPY16_DESC
	POP_X																	;X теперь хранит страое смещение на стек задачи

	LDI PID,0x00
_C5_STACK_TO_TOP__LOOP:
	MCALL _C5_PROC_HEADER_GET

	;Если задачи нет, пропускаем
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_ABSENT
	BREQ _C5_STACK_TO_TOP__NEXT_TASK

	;Проверка, если адрес стека задачи в цикле больше чем адрес стека основной задачи(X), то пропускаем(равенство исключено)
	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	CP XH,TEMP_H
	BRCS _C5_STACK_TO_TOP__NEXT_TASK
	BRNE _C5_STACK_TO_TOP__CHANGE_OFFSET
	CP XL,TEMP_L
	BRCS _C5_STACK_TO_TOP__NEXT_TASK

_C5_STACK_TO_TOP__CHANGE_OFFSET:
	;Изменяем смещение на длину стека основной задачи
	ADD TEMP_L,TEMP_EL
	CLR TEMP
	ADC TEMP_H,TEMP
	STD Z+_C5_TASK_STACK_OFFSET+0x00,TEMP_H
	STD Z+_C5_TASK_STACK_OFFSET+0x01,TEMP_L

_C5_STACK_TO_TOP__NEXT_TASK:
	INC PID
	CPI PID,C5_TASKS_QNT
	BRNE _C5_STACK_TO_TOP__LOOP

_C5_STACK_TO_TOP__END:
	POP PID
	POP_Z
	RET
.endif
