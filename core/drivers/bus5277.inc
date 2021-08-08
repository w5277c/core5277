;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;03.05.2020	w5277c@gmail.com			Начало
;12.02.2020	w5277c@gmail.com			CRC8_MAXIM_BLOCK->CRC8_BLOCK
;17.02.2021	w5277c@gmail.com			Исправлены ошибки
;19.02.2021	w5277c@gmail.com			Оптимизирован код, переделаны методы SLAVE, добавлена реализация SLAVE EXTRASCAN.
;												Код не оттестирован!
;12.06.2021	w5277c@gmail.com			Вынесен буфер пакета, не тестировно!
;14.06.2021	w5277c@gmail.com			Исправлены существенные ошибки, проведен успешный тест EXTRASCAN T
;04.07.2021	w5277c@gmail.com			Z->Y, необходимо тестирование
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер шины BUS5277
;-----------------------------------------------------------------------------------------------------------------------
;TODO все константы должны начинаться с DRV_
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_BUS5277
.else
.set DEF__C5_DRV_BUS5277 = 1
.message "Included driver Bus5277 v0.4"

;Шина на базе serial(19200-8N1+BREAK)
;  addr     size     code     data0        dataN     crc
; |eaaaaaaa|rrssssss|qucccccc|dddddddd|...|dddddddd|wwwwwwww|
; где:
; e - бит extrascan, если включен, то пакет состоит из 2 байт(addr+crc)(если а=0 - режим таймингов)
; a - адрес ведомого устройства (1-127, 0-новое или ведущее устройство)
; r - заразервировано
; s - размер всего пакета(40B максимум, длина данных = s-4)
; q - бит 0-запрос, 1-ответ
; с - код функции или ответа
; u - признак адресации по UID (значение a игнорируется)
; d - данные функции (при включенном u, первые 8 байт = UID)
; w - crc8(1wire)

	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./conv/crc8_maxim.inc"
	.include	"./conv/crc8_block.inc"
	.include	"./mem/ram_copy8.inc"
	.include	"./mem/ram_fill8.inc"
	.include	"./mem/rom_read_bytes.inc"

.ifdef LOGGING_PORT
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_char.inc"
	.include	"./core/log/log_bytes.inc"
.endif

;---CONSTANTS--------------------------------------------
_DRV_BUS5277_EXTRASCAN_T_REQ_DATA:								;2B - данные запроса EXTRASCAN_T
	.db	0x80,0x8c

	;---VAR-OFFSETS---
	;---DATA-OFFSETS---
;	.EQU	_DRV_BUS5277_PACKET_HEADER						= 0x00;3B - заголовок пакета
	.EQU		_DRV_BUS5277_PACKET_HEADER_ADDR		= 0x00;
	.EQU		_DRV_BUS5277_PACKET_HEADER_SIZE		= 0x01;
	.EQU		_DRV_BUS5277_PACKET_HEADER_CODE		= 0x02;

	.EQU	_DRV_BUS5277_EXTRASCAN_T_BUFFER_ADDR	= 0x00	;2B - адрес буфера EXTRASCAN_T
	.EQU	_DRV_BUS5277_BUS_ADDR						= 0x02	;1B - адрес устройства на шине
	.EQU	_DRV_BUS5277_UART_DRV_ID					= 0x03	;1B - ид драйвера UART
	.EQU	_DRV_BUS5277_EXTRASCAN_TIMER				= 0x04	;1B - ид таймера для EXTRASCAN_T
	.EQU	_DRV_BUS5277_EXTRASCAN_CNTR				= 0x05	;1B - счетчик для EXTRASCAN_T
	.EQU	_DRV_BUS5277_FLAGS							= 0x06	;1B - флаги драйвера

	;---FLAGS---
	.EQU	DRV_BUS5277_FLAG_HAVE_DATA					= 0x00	;Бит наличия данных
	.EQU	DRV_BUS5277_FLAG_LOGGING					= 0x01	;Бит логирования
	.EQU	DRV_BUS5277_FLAG_MASTER						= 0x02	;Режим 1-MASTER/0-SLAVE
	;---OPERATIONS---
	.EQU	DRV_BUS5277_OP_MASTER						= 0x00	;Режим ведущего (запрос-ожидание ответа)
	.EQU	DRV_BUS5277_OP_SLAVE_RECV					= 0x02	;Режим ведомого ожидание запроса
	.EQU	DRV_BUS5277_OP_SLAVE_SEND					= 0x04	;Режим ведомого ответ на запрос
	.EQU	DRV_BUS5277_OP_EXTRASCAN					= 0x06	;Режим EXTRASCAN - опрос адресов на наличие новых данных
	.EQU	DRV_BUS5277_OP_EXTRASCAN_T					= 0x08	;Режим TIMING EXTRASCAN
	.EQU	DRV_BUS5277_OP_HAVE_DATA					= 0x0a	;Управляем флагом HAVE_DATA
	;---REQUEST-CODES---
	.EQU	BUS5277_FUNC_INFO								= 0x00	;Чтение исномации об устройстве (0x5277 + тип, серийниый номер, версия PCB, версия FW, адрес на шине, uptime)
	.EQU	BUS5277_FUNC_ADDR_SET						= 0x01	;Установка адреса на шине(1-126)
	.EQU	BUS5277_FUNC_DATA_READ						= 0x02	;Чтение данных(показаний)
	.EQU	BUS5277_FUNC_SETTINGS_READ					= 0x03	;Чтение блока расширенных настроек
	.EQU	BUS5277_FUNC_SETTINGS_WRITE				= 0x04	;Запись блока расширенных настроек (с изменением в EEPROM)
	.EQU	BUS5277_FUNC_COMMAND_EXEC					= 0x05	;Выполнение команды (таких как вкл/выкл/инвертировать)
	.EQU	BUS5277_FUNC_HISTORY_PULL					= 0x06	;Чтение первой записи истории или последующей с удалением первой
	.EQU	BUS5277_FUNC_IFACE_OPEN						= 0x07	;Инициализация доп. интерфейса (открытие соединения)
	.EQU	BUS5277_FUNC_IFACE_COMM						= 0x08	;Выполнить процедуру передачи и приема данных
	.EQU	BUS5277_FUNC_IFACE_STATUS					= 0x09	;Сообщить состояние интерфейса
	.EQU	BUS5277_FUNC_IFACE_CLOSE					= 0x0a	;Закрытие соединения
	.EQU	BUS5277_FUNC_IFACE_BUFFER_READ			= 0x0b	;Чтение блока данных буфера интерфейса
	.EQU	BUS5277_FUNC_IFACE_BUFFER_WRITE			= 0x0c	;Запись блока данных в буфер интерфейса
	.EQU	BUS5277_FUNC_BTLDR_MODE						= 0x0d	;Перейти в режим бутлоадера
	.EQU	BUS5277_FUNC_BTLDR_WRITE					= 0x0e	;Записать блок данных во FLASH
	.EQU	BUS5277_FUNC_BTLDR_DONE						= 0x0f	;Выйти из режима бутлоадера
	.EQU	BUS5277_FUNC_COMM								= 0x10	;Быстрый аналог IFACE_OPEN/COMMUNICATE/CLOSE для BUS5277
	;---																	;Блок комманд перезапуска устройства
	.EQU	BUS5277_FUNC_SOFT_RESET						= 0x3c	;Перезагрузка устройства
	.EQU	BUS5277_FUNC_RESET							= 0x3d	;Сброс устройства к первоначальным настройкам (кроме параметров соединения)
	.EQU	BUS5277_FUNC_FULL_RESET						= 0x3e	;Сброс устройства к первоначальным настройкам
	;---																	;Блок комманд с адресацией по UID
	.EQU	BUS5277_FUNC_UID_SCAN						= 0x40	;Поиск устройства (определение UID устройств)
	.EQU	BUS5277_FUNC_INFO_BY_UID					= 0x41	;Чтение информации об устройстве (аналогично BUS5277_FUNC_INFO, но адресация по UID)
	.EQU	BUS5277_FUNC_ADDR_SET_BY_UID				= 0x42	;Установка адреса на шине (аналогично BUS5277_FUNC_ADDR_SET, но адресация по UID)
	;---RESPONSE-CODES---
	.EQU	BUS5277_RESP_OK								= 0x80	;Все в порядке
	.EQU	BUS5277_RESP_NO_ANSWER						= 0x81	;Нет ответа
	.EQU	BUS5277_RESP_INVALID_PACKET				= 0x82	;Битый пакет, несоответствие ADDR, бит запрос/ответ или CRC
	.EQU	BUS5277_RESP_ILLEGAL_FUNCTION				= 0x83	;Неизвестная функция
	.EQU	BUS5277_RESP_ILLEGAL_DATA_ADDR			= 0x84	;Не корректно указан начальный адрес либо запрошенный блок выходит за рамки блока данных
	.EQU	BUS5277_RESP_INVALIDATE						= 0x85	;Не корректный формат данных
	.EQU  BUS5277_RESP_REJECT							= 0x86	;Отклонено, например из-за SOURCE_MODE запрещающего управление по MODBUS
	.EQU	BUS5277_RESP_NO_DATA							= 0x87	;Нечего возвращать
	.EQU	BUS5277_RESP_RELAY_LOCK						= 0x88	;Реле заблокировано тригерром
	.EQU	BUS5277_RESP_ILLEGAL_CHANNEL				= 0x89	;Некорректный номер канала
	.EQU	BUS5277_RESP_ILLEGAL_COMMAND				= 0x8a	;Некорректная комманда
	.EQU	BUS5277_RESP_BLDR_MODE						= 0x8b	;Режим бутлоадера
	.EQU	BUS5277_EVENT_EXTRASCAN						= 0xc0	;Сообщение с данными EXTRASCAN
	.EQU	BUS5277_RESP_UNSUPPORTED					= 0x8c	;Функционал не поддерживается
	;---COMMAND-CODES---
	.EQU	BUS5277_CMD_CANCEL							= 0x00	;Отмена текущей команды (не используется)
	.EQU	BUS5277_CMD_TURN_ON							= 0x01	;Включить
	.EQU	BUS5277_CMD_TURN_OFF							= 0x02	;Выключить
	.EQU	BUS5277_CMD_SWITCH							= 0x03	;Переключить
	.EQU	BUS5277_CMD_INC_POWER						= 0x04	;Увеличить мощность
	.EQU	BUS5277_CMD_DEC_POWER						= 0x05	;Уменьшить мощность
	.EQU	BUS5277_CMD_SET_POWER						= 0x06	;Установить мощность в процентах
	.EQU	BUS5277_CMD_SET_MODE							= 0x07	;Установить режим управления
	.EQU	BUS5277_CMD_EXECUTE							= 0x08	;Выполнить
	.EQU	BUS5277_CMD_SET_VALUE						= 0x09	;Задать значение
	.EQU	BUS5277_CMD_SET_TIMER						= 0x0a	;Запустить таймер
	.EQU	BUS5277_CMD_SET_POWER_SLOW					= 0x0b	;Установить мощность в процентах с плавным переходом
	;---COMMAND-SOURCE-CODES---
	.EQU	BUS5277_CMD_SRC_MANUAL						= 0x01	;Вручную
	.EQU	BUS5277_CMD_SRC_BUS							= 0x02	;Шина
	.EQU	BUS5277_CMD_SRC_TIMER						= 0x04	;Таймер
	.EQU	BUS5277_CMD_SRC_TRIGGER						= 0x08	;Триггер
	.EQU	BUS5277_CMD_SRC_TRIGGER_ERR				= 0x88	;Триггер, ошибка датчика(старший бит)
	;---REQUEST/RESPONSE-INTERNAL-ERROR-CODE
	.EQU	BUS5277_FAULT									= 0xff	;Сбой в логике
	;---
	.EQU	DRV_BUS5277_MAX_PACKET_SIZE				= 0x28	;Максимальный размер пакета 40B
	.EQU	DRV_BUS5277_MIN_PACKET_SIZE				= 0x04	;Минимальный размер пакета 4B
	.EQU	_DRV_BUS5277_LISTEN_TIMEOUT				= 0x32	;100 мс - таймаут для приема ответа
	.EQU	_DRV_BUS5277_MASTER_ADDR					= 0x00	;Адрес мастера
	.EQU	_DRV_BUS5277_EXTRASCAN_T_PERIOD			= 0x01	;Период чтения сигнала 0,000 050с
	;---
	.EQU	_DRV_BUS5277_RAM_SIZE						= 0x07	;

;--------------------------------------------------------
DRV_BUS5277_INIT:
;--------------------------------------------------------
;Инициализация
;TEMP_H-адрес устройства на шине
;TEMP_L-ид таймера EXTRASCAN_T
;ACCUM-ид UART драйвера
;FLAGS-флаги
;--------------------------------------------------------
	;Выделяем память
	PUSH ACCUM
	LDI ACCUM,_DRV_BUS5277_RAM_SIZE
	MCALL C5_RAM_REALLOC
	;Записываем адрес на шине и флаги
	STD Y+_DRV_BUS5277_BUS_ADDR,TEMP_H
	STD Y+_DRV_BUS5277_FLAGS,FLAGS
	STD Y+_DRV_BUS5277_EXTRASCAN_TIMER,TEMP_L
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,C0x00
	;Записываем ид UART'а
	POP ACCUM
	STD Y+_DRV_BUS5277_UART_DRV_ID,ACCUM
	;Настраиваем таймер для EXTRASCAN T (период 0.000 1с)
	MOV TEMP,TEMP_L
	LDI TEMP_H,high(_DRV_BUS5277_EXTRASCAN_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_BUS5277_EXTRASCAN_TIMER_EVENT)
	LDI ACCUM,_DRV_BUS5277_EXTRASCAN_T_PERIOD/2+0x01
	MCALL C5_TIMER_SET

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация по 5277 шине
;IN: FLAGS-тип операции
;OUT: ACCUM-код функции/ответа(0xff - сбой)
;--------------------------------------------------------
	PUSH_Y

	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET_Y

	CPI FLAGS,DRV_BUS5277_OP_MASTER
	BRNE PC+0x04
	MCALL _DRV_BUS5277_PROC_MASTER
	RJMP _DRV_BUS5277__END

	CPI FLAGS,DRV_BUS5277_OP_SLAVE_RECV
	BRNE PC+0x04
	MCALL _DRV_BUS5277_PROC_SLAVE_RECV
	RJMP _DRV_BUS5277__END

	CPI FLAGS,DRV_BUS5277_OP_SLAVE_SEND
	BRNE PC+0x04
	MCALL _DRV_BUS5277_PROC_SLAVE_SEND
	RJMP _DRV_BUS5277__END

	CPI FLAGS,DRV_BUS5277_OP_EXTRASCAN
	BRNE PC+0x04
	MCALL _DRV_BUS5277_PROC_EXTRASCAN
	RJMP _DRV_BUS5277__END

	CPI FLAGS,DRV_BUS5277_OP_EXTRASCAN_T
	BRNE PC+0x04
	MCALL _DRV_BUS5277_PROC_EXTRASCAN_T
	RJMP _DRV_BUS5277__END

	CPI FLAGS,DRV_BUS5277_OP_HAVE_DATA
	BRNE PC+0x03
	MCALL _DRV_BUS5277_PROC_SET_HAVE_DATA

_DRV_BUS5277__END:
	POP_Y
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_MASTER:
;--------------------------------------------------------
;Режим MASTER, передача и прием пакета,
;пакет формируется и парсится отдельно процедурами
;PACKET_PACK и PACKET_UNPACK.
;IN: Y-выделенная память, Z-src адрес, X-dst адрес
;OUT: ACCUM-код ответа
;--------------------------------------------------------
	PUSH_X
	PUSH_FT

	;Режим ведущего
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	ORI TEMP,(1<<DRV_BUS5277_FLAG_MASTER)
	STD Y+_DRV_BUS5277_FLAGS,TEMP

.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_REQV
.endif
	;Запускаем драйвер UART
	PUSH YH
	LDD TEMP_EH,Z+_DRV_BUS5277_PACKET_HEADER_SIZE			;Длина данных для передачи
	LDD TEMP,Y+_DRV_BUS5277_UART_DRV_ID
	LDI TEMP_EL,DRV_BUS5277_MAX_PACKET_SIZE					;Максимальная длина данных для приема
	LDI TEMP_H,high(20/2)											;Максимальное время ожидания данных
	LDI TEMP_L,low(20/2)
	LDI YH,0xff
	MCALL C5_EXEC
	POP YH
	;Проверка ответа на ошибку
	LDI ACCUM,BUS5277_RESP_NO_ANSWER
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE _DRV_BUS5277_PROC_MASTER__END
	LDI ACCUM,BUS5277_RESP_NO_DATA
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277_PROC_MASTER__END
	CPI TEMP_L,0x28
	BRCC _DRV_BUS5277_PROC_MASTER__END
	LDI ACCUM,BUS5277_RESP_OK
.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_RESP
.endif
_DRV_BUS5277_PROC_MASTER__END:

	POP_FT
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_RECV:
;--------------------------------------------------------
;Режим SLAVE, прием
;IN: Y-выделенная память, X-dst адрес
;OUT: ACCUM-код ответа
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH FLAGS
	PUSH TRY_CNTR

	;Режим ведомого
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	ANDI TEMP,~(1<<DRV_BUS5277_FLAG_MASTER)
	STD Y+_DRV_BUS5277_FLAGS,TEMP

_DRV_BUS5277_PROC_SLAVE_RECV__LOOP:
	;Запускаем драйвер UART
	PUSH_Y
	LDD TEMP,Y+_DRV_BUS5277_UART_DRV_ID
	LDI TEMP_EH,0x00													;Длина данных для передачи
	LDI TEMP_EL,DRV_BUS5277_MAX_PACKET_SIZE					;Максимальная длина данных для приема
	LDI TEMP_H,0x00													;Максимальное время ожидания данных(0 - ждать постоянно)
	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x80												;Пара байт для определения EXTRASCAN T
	LDI FLAGS,0x8c
	MOV TRY_CNTR,PID
	LDI_Y _DRV_BUS5277_SLAVE_MAGIC_EVENT						;Адрес процедуры отклика на EXTRASCAN T
	MCALL C5_EXEC
	POP_Y

	;Проверка ответа на ошибку
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	CPI TEMP_L,0x28
	BRCC _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	LD TEMP,X
	SBRC TEMP,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP

.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_RESP
.endif

	LDI ACCUM,BUS5277_RESP_OK
_DRV_BUS5277_PROC_SLAVE_RECV__END:
	POP TRY_CNTR
	POP FLAGS
	POP LOOP_CNTR
	POP_FT
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_SEND:
;--------------------------------------------------------
;Режим SLAVE, передача
;IN: Y-выделенная память, Z-src адрес
;OUT: ACCUM-код ответа
;--------------------------------------------------------
	PUSH_FT

	;Режим ведомого
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	ANDI TEMP,~(1<<DRV_BUS5277_FLAG_MASTER)
	STD Y+_DRV_BUS5277_FLAGS,TEMP

.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_REQV
.endif
	;Запускаем драйвер UART
	PUSH YH
	LDD TEMP,Y+_DRV_BUS5277_UART_DRV_ID
	LDD TEMP_EH,Z+_DRV_BUS5277_PACKET_HEADER_SIZE			;Длина данных для передачи
	LDI TEMP_EL,0x00													;Максимальная длина данных для приема(только передача)
	LDI YH,0xff
	MCALL C5_EXEC
	POP YH
	LDI ACCUM,BUS5277_RESP_OK

	POP_FT
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_EXTRASCAN:
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_EXTRASCAN_T:
;Режим MASTER, экстрасканирования на базе таймингов
;128б ответа соответствуют номерам устройств с откликом
;IN: Y-выделенная память, X-dst адрес, ACCUM-код ответа
;--------------------------------------------------------
	PUSH_FT

	;Режим ведущего

	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	ORI TEMP,(1<<DRV_BUS5277_FLAG_MASTER)
	STD Y+_DRV_BUS5277_FLAGS,TEMP

	STD Y+_DRV_BUS5277_EXTRASCAN_T_BUFFER_ADDR+0x00,XH
	STD Y+_DRV_BUS5277_EXTRASCAN_T_BUFFER_ADDR+0x01,XL

	LDI TEMP,0x00
	LDI LOOP_CNTR,0x10
	MCALL RAM_FILL8

	;Обнуляем счетчик устройств
	LDI TEMP,0x01
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP

	LDI_Z _DRV_BUS5277_EXTRASCAN_T_REQ_DATA|0x8000
	MCALL C5_DISPATCHER_LOCK
	PUSH YH
	LDD TEMP,Y+_DRV_BUS5277_UART_DRV_ID
	LDI TEMP_EH,0x02
	LDI TEMP_EL,0x00
	LDI YH,0xff
	MCALL C5_EXEC														;Необходимо, чтобы UART отработал фиксированное время
	POP YH

	;Запускаем таймер отсчитывающий интервалы чтения сигналов от устройств
	LDS FLAGS,SREG
	CLI
	LDI TEMP,TIMERS_SPEED
	_CORRE5277_TIMERA_CORRECTOR TEMP
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_TIMER
	LDI ACCUM,_DRV_BUS5277_EXTRASCAN_T_PERIOD
	MCALL C5_TIMER_START_AT
	STS SREG,FLAGS
	MCALL C5_DISPATCHER_UNLOCK

_DRV_BUS5277_PROC_EXTRASCAN_T__LOOP:
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_CNTR
	SBRS TEMP,0x07
	RJMP _DRV_BUS5277_PROC_EXTRASCAN_T__LOOP

	LDI ACCUM,BUS5277_RESP_OK
	CPI TEMP,0xff
	BRNE PC+0x02
	LDI ACCUM,BUS5277_RESP_REJECT

	POP_FT
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SET_HAVE_DATA:
;--------------------------------------------------------
;Устанавливаем наличие данных
;IN: Y-выделенная память, ACCUM-(0x01-SET,0x00-RESET)
;--------------------------------------------------------
	PUSH TEMP
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	ANDI TEMP,~(1<<DRV_BUS5277_FLAG_HAVE_DATA)
	SBRC ACCUM,0x00
	ORI TEMP,(1<<DRV_BUS5277_FLAG_HAVE_DATA)
	STD Y+_DRV_BUS5277_FLAGS,TEMP
	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUS5277_EXTRASCAN_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик прерывания от таймера для четния сигналов от
;устройств
;IN: PID-PID драйвера
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP
	PUSH_THL
	PUSH ACCUM

	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET_Y

	SBRS _C5_COREFLAGS,_CFL_TIMER_UART_CORRECTION
	RJMP PC+0x04
	LDI TEMP,0xff
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP
	RJMP _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__END

_DRV_BUS5277_EXTRASCAN_TIMER_EVENT__GOOD_TIMER:
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	SBRC TEMP,DRV_BUS5277_FLAG_MASTER
	RJMP _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__MASTER
	;Режим ведомого
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_TIMER
	LDD TEMP_L,Y+_DRV_BUS5277_EXTRASCAN_CNTR
	LDI ACCUM,0x01
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,ACCUM

	PUSH PID
	LDD PID,Y+_DRV_BUS5277_UART_DRV_ID
	;Получаю адрес выделенной памяти драйвера UART
	MCALL C5_RAM_OFFSET_Y
	POP PID
	CPI TEMP_L,0x00
	BRNE _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__SLAVE_DONE
	;Выставляю низкий уровень TX (DR->HI)
	LDD ACCUM,Y+DRV_UART_TX_PORT
	MCALL PORT_SET_LO
	LDD ACCUM,Y+DRV_UART_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	LDI ACCUM,_DRV_BUS5277_EXTRASCAN_T_PERIOD
	MCALL C5_TIMER_START_AT
	RJMP _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__END
_DRV_BUS5277_EXTRASCAN_TIMER_EVENT__SLAVE_DONE:
	;Выставляю высокий уровень TX (DR->LO)
	LDD ACCUM,Y+DRV_UART_TX_PORT
	MCALL PORT_SET_HI
	LDD ACCUM,Y+DRV_UART_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
	MCALL C5_TIMER_STOP
	RJMP _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__END
_DRV_BUS5277_EXTRASCAN_TIMER_EVENT__MASTER:
	;Режим ведущего
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_CNTR
	PUSH_X
	PUSH TEMP
	LDD XH,Y+_DRV_BUS5277_EXTRASCAN_T_BUFFER_ADDR+0x00
	LDD XL,Y+_DRV_BUS5277_EXTRASCAN_T_BUFFER_ADDR+0x01
	LSR TEMP
	LSR TEMP
	LSR TEMP
	ADD XL,TEMP
	ADC XH,C0x00
	LD ACCUM,X
	LSL ACCUM
	SBIS PIND,RXD&0x0f
	ORI ACCUM,0x01

	ST X,ACCUM
	POP TEMP
	POP_X

	INC TEMP
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP
	CPI TEMP,0x80
	BRNE _DRV_BUS5277_EXTRASCAN_TIMER_EVENT__END
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_TIMER
	MCALL C5_TIMER_STOP

_DRV_BUS5277_EXTRASCAN_TIMER_EVENT__END:
	POP ACCUM
	POP_THL
	POP TEMP
	POP_Y
	RET

;--------------------------------------------------------
_DRV_BUS5277_SLAVE_MAGIC_EVENT:
;--------------------------------------------------------
;Обработчик прерывания от UART при получении MAGIC слова
;ACCUM,TEMP,FLAGS не нужно сохранять
;--------------------------------------------------------
	PUSH_Y
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET_Y

	;Проверяем на HAVE_DATA
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	SBRS TEMP,DRV_BUS5277_FLAG_HAVE_DATA
	RJMP _DRV_BUS5277_SLAVE_MAGIC_EVENT__END
	;При 0 адресе не отвечаем
	LDD ACCUM,Y+_DRV_BUS5277_BUS_ADDR
	CPI ACCUM,0x00
	BREQ _DRV_BUS5277_SLAVE_MAGIC_EVENT__END

	;Признак первой отработки таймера
	LDI TEMP,0x00
	STD Y+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP

	LDS FLAGS,SREG
	CLI
	LDI TEMP,TIMERS_SPEED
	_CORRE5277_TIMERA_CORRECTOR TEMP
	LDD TEMP,Y+_DRV_BUS5277_EXTRASCAN_TIMER
	MCALL C5_TIMER_START_AT
	STS SREG,FLAGS

_DRV_BUS5277_SLAVE_MAGIC_EVENT__END:
	POP_Y
	RET

;--------------------------------------------------------
DRV_BUS5277__PACKET_PACK:
;--------------------------------------------------------
;Сборка пакета
;IN: Z-адрес на данные (0-копировать не нужно),
;TEMP_H-код функции, TEMP_L-длина данных,
;LOOP_CNTR-адрес устройства
;X-DST адрес
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH LOOP_CNTR

	MOV ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x04)											;+0x04
	PUSH_X
	ST X+,LOOP_CNTR													;Адрес устройства на шине
	ST X+,ACCUM															;Длина данных всего пакета
	ST X+,TEMP_H														;Код функции или ответа
	MOV LOOP_CNTR,TEMP_L
	;Проверяю наличие данных и копирую их
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE
	SBRC ZH,0x07
	RJMP _DRV_BUS5277__PACKET_PACK__ROM_DATA_COPY
_DRV_BUS5277__PACKET_PACK__RAM_DATA_COPY:
	;Проверка на необходимость копирования
	CPI ZH,0x00
	BRNE PC+0x03
	CPI ZL,0x00
	BREQ _DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE
	PUSH_Z
	MOVW ZL,XL
	POP_X
	MCALL RAM_COPY8
	RJMP _DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE
_DRV_BUS5277__PACKET_PACK__ROM_DATA_COPY:
	ANDI ZH,0x7f
	LSL ZL
	ROL ZH
	MOVW YL,XL
	MCALL ROM_READ_BYTES
_DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE:
	POP_X
	SUBI LOOP_CNTR,(0x100-0x03)

	LDI_Z CRC8_MAXIM
	MCALL CRC8_BLOCK
	ADD XL,LOOP_CNTR
	CLR LOOP_CNTR
	ADC XH,LOOP_CNTR
	ST X,ACCUM

	POP LOOP_CNTR
	POP ACCUM
	POP_Z
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
DRV_BUS5277__PACKET_UNPACK:
;--------------------------------------------------------
;Распаковка пакета
;IN: Z-SRC адрес
;OUT: TEMP_H-код функции,
;TEMP_L-длина данных, LOOP_CNTR-адрес устройства
;Z-адрес на данные
;--------------------------------------------------------
	PUSH_X
	PUSH ACCUM

	MOVW XL,ZL
	LD LOOP_CNTR,Z+
	LD TEMP_L,Z+
	CPI TEMP_L,DRV_BUS5277_MAX_PACKET_SIZE+0x01
	BRCC DRV_BUS5277__PACKET_UNPACK__INVALID_PACKET
	CPI TEMP_L,DRV_BUS5277_MIN_PACKET_SIZE
	BRCS DRV_BUS5277__PACKET_UNPACK__INVALID_PACKET
	SUBI TEMP_L,0x04
	LD TEMP_H,Z+
	;Вычисляю CRC
	PUSH LOOP_CNTR
	MOV LOOP_CNTR,TEMP_L
	SUBI LOOP_CNTR,(0x100-0x03)
	PUSH_Z
	LDI_Z CRC8_MAXIM
	MCALL CRC8_BLOCK
	POP_Z
	ADD XL,LOOP_CNTR
	CLR LOOP_CNTR
	ADC XH,LOOP_CNTR
	POP LOOP_CNTR
	LD XH,X
	CP XH,ACCUM
	BREQ DRV_BUS5277__PACKET_UNPACK__CORRECT_CRC
DRV_BUS5277__PACKET_UNPACK__INVALID_PACKET:
	LDI TEMP_L,0x04
	LDI TEMP_H,BUS5277_RESP_INVALID_PACKET
DRV_BUS5277__PACKET_UNPACK__CORRECT_CRC:
	POP ACCUM
	POP_X
	RET

.ifdef LOGGING_PORT
;--------------------------------------------------------
_DRV_BUS5277__LOG_REQV:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	SBRS TEMP,DRV_BUS5277_FLAG_LOGGING
	RET
	LDI TEMP,'>'
	MCALL C5_LOG_CHAR
	LDD TEMP,Z+_DRV_BUS5277_PACKET_HEADER_SIZE
	PUSH_Y
	MOVW YL,ZL
	MCALL C5_LOG_BYTES
	POP_Y
	MCALL C5_LOG_CR
	RET
;--------------------------------------------------------
_DRV_BUS5277__LOG_RESP:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	LDD TEMP,Y+_DRV_BUS5277_FLAGS
	SBRS TEMP,DRV_BUS5277_FLAG_LOGGING
	RET
	LDI TEMP,'<'
	MCALL C5_LOG_CHAR
	PUSH_Y
	MOVW YL,XL
	LDD TEMP,Y+_DRV_BUS5277_PACKET_HEADER_SIZE
	MCALL C5_LOG_BYTES
	POP_Y
	MCALL C5_LOG_CR
	RET
.endif
.endif
