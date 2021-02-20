;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;03.05.2020  w5277c@gmail.com			Начало
;12.02.2020  w5277c@gmail.com			CRC8_MAXIM_BLOCK->CRC8_BLOCK
;17.02.2021  w5277c@gmail.com			Исправлены ошибки
;19.02.2021  w5277c@gmail.com			Оптимизирован код, переделаны методы SLAVE, добавлена реализация SLAVE EXTRASCAN.
;												Код не оттестирован!
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_BUS5277
.else
.set DEF__C5_DRV_BUS5277 = 1
.message "Included driver Bus5277 v0.2"

;Шина на базе serial(19200-8N1+BREAK)
;  addr     size     code     data0        dataN     crc
; |eaaaaaaa|rrssssss|qccccccc|dddddddd|...|dddddddd|wwwwwwww|
; где:
; e - бит extrascan, если включен, то пакет состоит из 2 байт(addr+crc)(если а=0 - режим таймингов)
; a - адрес устройства (0-127)
; r - заразервировано
; s - размер всего пакета(40B максимум, длина данных = s-4)
; q - бит 0-запрос, 1-ответ
; с - код функции или ответа
; d - данные функции
; w - crc8(1wire)

	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./conv/crc8_maxim.inc"
	.include	"./conv/crc8_block.inc"
	.include	"./mem/ram_copy8.inc"
	.include	"./mem/rom_read_bytes.inc"

.ifdef LOGGING_PORT
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_char.inc"
	.include	"./core/log/log_bytes.inc"
.endif

;---CONSTANTS--------------------------------------------
	;---VAR-OFFSETS---
	;---DATA-OFFSETS---
	.EQU	_DRV_BUS5277_PACKET_HEADER						= 0x00;3B - заголовок пакета
	.EQU		_DRV_BUS5277_PACKET_HEADER_ADDR			= 0x00;
	.EQU		_DRV_BUS5277_PACKET_HEADER_SIZE			= 0x01;
	.EQU		_DRV_BUS5277_PACKET_HEADER_CODE			= 0x02;

	.EQU	DRV_BUS5277_PACKET_DATA							= 0x03;37B - тело пакета
	.EQU	_DRV_BUS5277_BUS_ADDR							= 0x28;1B - адрес устройства на шине
	.EQU	_DRV_BUS5277_UART_DRV_ID						= 0x29;1B - ид драйвера UART
	.EQU	_DRV_BUS5277_EXTRASCAN_TIMER					= 0x2a;1B - ид таймера для EXTRASCAN_T
	.EQU	_DRV_BUS5277_EXTRASCAN_CNTR					= 0x2b;1B - счетчик для EXTRASCAN_T
	.EQU	_DRV_BUS5277_FLAGS								= 0x2c;1B - флаги драйвера

	;---FLAGS---
	.EQU	DRV_BUS5277_FLAG_HAVE_DATA						= 0x00;Бит наличия данных
	.EQU	DRV_BUS5277_FLAG_LOGGING						= 0x01;Бит логирования
	.EQU	DRV_BUS5277_FLAG_EXSTRASCAN_BUSY				= 0x02;Признак работы EXSTRASCAN
	;---OPERATIONS---
	.EQU	DRV_BUS5277_OP_MASTER							= 0x00;Режим ведущего (запрос-ожидание ответа)
	.EQU	DRV_BUS5277_OP_SLAVE_RECV						= 0x01;Режим ведомого ожидание запроса
	.EQU	DRV_BUS5277_OP_SLAVE_SEND						= 0x02;Режим ведомого ответ на запрос
	.EQU	DRV_BUS5277_OP_EXTRASCAN						= 0x03;Режим EXTRASCAN - опрос адресов на наличие новых данных
	.EQU	DRV_BUS5277_OP_EXTRASCAN_T						= 0x04;Режим TIMING EXTRASCAN
	.EQU	DRV_BUS5277_OP_HAVE_DATA						= 0x05;Устанавливаем флаг HAVE_DATA
	;---REQUEST-CODES---
	.EQU	BUS5277_FUNC_INFO									= 0x00;Чтение исномации об устройстве (тип, версия, uptime)
	.EQU	BUS5277_FUNC_DATA_READ							= 0x01;Чтение данных
	.EQU	BUS5277_FUNC_SETTINGS_READ						= 0x02;Чтение блока настроек
	.EQU	BUS5277_FUNC_SETTINGS_WRITE					= 0x03;Запись блока настроек
	.EQU	BUS5277_FUNC_COMMAND_EXEC						= 0x04;Выполнение команды (таких как вкл/выкл/инвертировать)
	.EQU	BUS5277_FUNC_HISTORY_PULL						= 0x05;Чтение первой записи истории или последующей с удалением первой
	.EQU	BUS5277_FUNC_IFACE_OPEN							= 0x06;Инициализация доп. интерфейса (открытие соединения)
	.EQU	BUS5277_FUNC_IFACE_COMM							= 0x07;Выполнить процедуру передачи и приема данных
	.EQU	BUS5277_FUNC_IFACE_STATUS						= 0x08;Сообщить состояние интерфейса
	.EQU	BUS5277_FUNC_IFACE_CLOSE						= 0x09;Закрытие соединения
	.EQU	BUS5277_FUNC_BUFFER_READ						= 0x0a;Чтение блока данных буфера
	.EQU	BUS5277_FUNC_BUFFER_WRITE						= 0x0b;Запись блока данных в буфер
	.EQU	BUS5277_FUNC_BTLDR_MODE							= 0x0c;Перейти в режим бутлоадера
	.EQU	BUS5277_FUNC_BTLDR_WRITE						= 0x0d;Записать блок данных во флеш
	.EQU	BUS5277_FUNC_BTLDR_DONE							= 0x0e;Выйти из режима бутлоадера
	;---
	.EQU	BUS5277_FUNC_SOFT_RESET							= 0x7c;Перезагрузка устройства
	.EQU	BUS5277_FUNC_RESET								= 0x7d;Сброс устройства к первоначальным настройкам (кроме параметров соединения)
	.EQU	BUS5277_FUNC_FULL_RESET							= 0x7e;Сброс устройства к первоначальным настройкам
	;---RESPONSE-CODES---
	.EQU	BUS5277_RESP_OK									= 0x80;Все в порядке
	.EQU	BUS5277_RESP_NO_ANSWER							= 0x81;Нет ответа
	.EQU	BUS5277_RESP_INVALID_PACKET					= 0x82;Битый пакет, несоответствие ADDR, бит запрос/ответ или CRC
	.EQU	BUS5277_RESP_ILLEGAL_FUNCTION					= 0x83;Неизвестная функция
	.EQU	BUS5277_RESP_ILLEGAL_DATA_ADDR				= 0x84;Не корректно указан начальный адрес либо запрошенный блок выходит за рамки блока данных
	.EQU	BUS5277_RESP_INVALIDATE							= 0x85;Не корректный формат данных
	.EQU  BUS5277_RESP_REJECT								= 0x86;Отклонено, например из-за SOURCE_MODE запрещающего управление по MODBUS
	.EQU	BUS5277_RESP_NO_DATA								= 0x87;Нечего возвращать
	.EQU	BUS5277_RESP_RELAY_LOCK							= 0x88;Реле заблокировано тригерром
	.EQU	BUS5277_RESP_ILLEGAL_CHANNEL					= 0x89;Некорректный номер канала
	.EQU	BUS5277_RESP_ILLEGAL_COMMAND					= 0x8a;Некорректная комманда
	.EQU	BUS5277_RESP_BTLDR_CRC							= 0x8b;Выозвращаем CRC полезных данных флеша
	;---COMMAND-CODES---
	.EQU	BUS5277_CMD_CANCEL								= 0x00;Отмена текущей команды (не используется)
	.EQU	BUS5277_CMD_TURN_ON								= 0x01;Включить
	.EQU	BUS5277_CMD_TURN_OFF								= 0x02;Выключить
	.EQU	BUS5277_CMD_SWITCH								= 0x03;Переключить
	.EQU	BUS5277_CMD_INC_POWER							= 0x04;Увеличить мощность
	.EQU	BUS5277_CMD_DEC_POWER							= 0x05;Уменьшить мощность
	.EQU	BUS5277_CMD_SET_POWER							= 0x06;Установить мощность в процентах
	.EQU	BUS5277_CMD_SET_MODE								= 0x07;Установить режим управления
	.EQU	BUS5277_CMD_EXECUTE								= 0x08;Выполнить
	.EQU	BUS5277_CMD_SET_VALUE							= 0x09;Задать значение
	.EQU	BUS5277_CMD_SET_TIMER							= 0x0a;Запустить таймер
	.EQU	BUS5277_CMD_CONNECT								= 0x0b;Подключиться
	.EQU	BUS5277_CMD_DISCONNECT							= 0x0c;Отключиться
	;---COMMAND-SOURCE-CODES---
	.EQU	BUS5277_CMD_SRC_MANUAL							= 0x01;Вручную
	.EQU	BUS5277_CMD_SRC_BUS								= 0x02;Шина
	.EQU	BUS5277_CMD_SRC_TIMER							= 0x04;Таймер
	.EQU	BUS5277_CMD_SRC_TRIGGER							= 0x08;Триггер
	.EQU	BUS5277_CMD_SRC_TRIGGER_ERR					= 0x88;Триггер, ошибка датчика(старший бит)
	;---REQUEST/RESPONSE-INTERNAL-ERROR-CODE
	.EQU	BUS5277_FAULT										= 0xff;Сбой в логике
	;---
	.EQU	DRV_BUS5277_MAX_PACKET_SIZE					= 0x28;Максимальный размер пакета 40B
	.EQU	DRV_BUS5277_MIN_PACKET_SIZE					= 0x04;Минимальный размер пакета 4B
	.EQU	_DRV_BUS5277_LISTEN_TIMEOUT					= 0x32;100 мс - таймаут для приема ответа
	.EQU	_DRV_BUS5277_MASTER_ADDR						= 0x00;Адрес мастера
	;---
	.EQU	_DRV_BUS5277_RAM_SIZE							= 0x2d;43B - буфер шины(40B) + переменные(5B)

;--------------------------------------------------------
DRV_BUS5277_INIT:
;--------------------------------------------------------
;Инициализация
;TEMP_H-адрес устройства на шине
;TEMP_L-ид таймера EXTRASCAN_T
;ACCUM-ид UART драйвера
;FLAGS-флаги
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	;Выделяем память
	PUSH ACCUM
	LDI ACCUM,_DRV_BUS5277_RAM_SIZE
	MCALL C5_RAM_REALLOC
	;Записываем адрес на шине и флаги
	STD Z+_DRV_BUS5277_BUS_ADDR,TEMP_H
	STD Z+_DRV_BUS5277_FLAGS,FLAGS
	STD Z+_DRV_BUS5277_EXTRASCAN_TIMER,TEMP_L
	CLR TEMP
	STD Z+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP
	;Записываем ид UART'а
	POP ACCUM
	STD Z+_DRV_BUS5277_UART_DRV_ID,ACCUM

	POP TEMP
	POP_Z

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация по 5277 шине
;IN: FLAGS - тип операции
;OUT: ACCUM - код функции/ответа(0xff - сбой)
;--------------------------------------------------------
	PUSH_Z

	LDI_Z _DRV_BUS5277_OP_END
	PUSH ZL
	PUSH ZH
	PUSH FLAGS
	LSL FLAGS
	LDI_Z _DRV_BUS5277_OP_JUMP_TABLE
	ADD ZL,FLAGS
	CLR FLAGS
	ADC ZH,FLAGS
	POP FLAGS
	PUSH ZL
	PUSH ZH
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	RET
_DRV_BUS5277_OP_END:
	POP_Z
	RET

_DRV_BUS5277_OP_JUMP_TABLE:
	MJMP _DRV_BUS5277_PROC_MASTER
	MJMP _DRV_BUS5277_PROC_SLAVE_RECV
	MJMP _DRV_BUS5277_PROC_SLAVE_SEND
	MJMP _DRV_BUS5277_PROC_EXTRASCAN
	MJMP _DRV_BUS5277_PROC_EXTRASCAN_T
	MJMP _DRV_BUS5277_PROC_SET_HAVE_DATA

;--------------------------------------------------------
_DRV_BUS5277_PROC_MASTER:
;--------------------------------------------------------
;Режим MASTER
;IN: Z-выделенная память, Y-адрес на данные, TEMP_H-код
;функции, TEMP_L-длина данных, LOOP_CNTR-адрес устройства
;OUT: ACCUM-код ответа,
;Y - адрес на блок данных, TEMP - длина данных
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP
	PUSH ACCUM

	;Проверяю на максимальный размер пакета
	CPI TEMP_L,0x24+0x01
	BRCS PC+0x03
	LDI ACCUM,BUS5277_FAULT
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR

	;Формирую пакет
	MOV XH,ZH
	MOV XL,ZL
	MCALL DRV_BUS5277__PACKET_PACK

	;Запускаем драйвер UART
	LDI TEMP_EH,0x04													;Длина данных для передачи
	ADD TEMP_EH,LOOP_CNTR
	MOV YH,ZH															;Адрес блока данных для передачи
	MOV YL,ZL
.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_REQV
.endif
	LDD TEMP,Z+_DRV_BUS5277_UART_DRV_ID
	MOV XH,ZH															;Адрес блока данных для приема
	MOV XL,ZL
	LDI TEMP_EL,DRV_BUS5277_MAX_PACKET_SIZE					;Максимальная длина данных для приема
	LDI TEMP_H,high(100/2)											;Максимальное время ожидания данных
	LDI TEMP_L,low(100/2)
	MCALL C5_EXEC
	;Проверка ответа на ошибку
	CPI TEMP_H,DRV_UART_ST_READY
	LDI ACCUM,BUS5277_RESP_NO_ANSWER
	BRNE _DRV_BUS5277_PROC_MASTER__ERROR
	CPI TEMP_L,0x00
	LDI ACCUM,BUS5277_RESP_NO_DATA
	BREQ _DRV_BUS5277_PROC_MASTER__ERROR
.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_RESP
.endif

	;Распаковка ответа
	MCALL DRV_BUS5277__PACKET_UNPACK
	;Проверяю адрес на совпадение
	MOV ACCUM,LOOP_CNTR
	ANDI ACCUM,0x7f
	LDD TEMP,Z+_DRV_BUS5277_BUS_ADDR
	CP ACCUM,TEMP
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	BRNE _DRV_BUS5277_PROC_MASTER__ERROR
	;Проверяю бит extrascan
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	SBRC LOOP_CNTR,0x07
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR
	;Проверяем бит ответа
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	SBRS TEMP_H,0x07
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR
	RJMP _DRV_BUS5277_PROC_MASTER__END
_DRV_BUS5277_PROC_MASTER__ERROR:
	LDI TEMP_H,BUS5277_RESP_INVALID_PACKET
	LDI TEMP_L,0x00
_DRV_BUS5277_PROC_MASTER__END:

	MOV YH,ZH
	MOV YL,ZL

	POP ACCUM
	POP TEMP
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_RECV:
;--------------------------------------------------------
;Режим SLAVE, прием
;IN: Z-выделенная память
;OUT: TEMP_H-код ответа,TEMP_L-длина данных, Y-адрес на
;блок данных
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH LOOP_CNTR

_DRV_BUS5277_PROC_SLAVE_RECV__LOOP:
	;Запускаем драйвер UART
	LDD TEMP,Z+_DRV_BUS5277_UART_DRV_ID
	MOV YH,ZH															;Адрес блока данных для передачи
	MOV YL,ZL
	MOV XH,ZH															;Адрес блока данных для приема
	MOV XL,ZL
	LDI TEMP_EH,0x00													;Длина данных для передачи
	LDI TEMP_EL,DRV_BUS5277_MAX_PACKET_SIZE					;Максимальная длина данных для приема
	LDI TEMP_H,0x00													;Максимальное время ожидания данных(0 - ждать постоянно)
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277_PROC_SLAVE_RECV__LOOP

	MCALL DRV_BUS5277__PACKET_UNPACK
	;Проверяю бит extrascan
	SBRC LOOP_CNTR,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN
	;Проверяю адрес на совпадение
	LDD TEMP,Z+_DRV_BUS5277_BUS_ADDR
	ANDI LOOP_CNTR,0x7f
	CP LOOP_CNTR,TEMP
	BRNE _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	;Блок обычного режима(не EXTRASCAN)
	;Проверяем бит ответа
	SBRC TEMP_H,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__END
_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN:
	;Проверяем тип EXTRASCAN
	CPI LOOP_CNTR,0x80
	BREQ _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T
	;Обычный EXTRASCAN
	LDI TEMP_H,BUS5277_RESP_NO_DATA
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP,Z+_DRV_BUS5277_FLAGS
	ANDI TEMP,~(1<<DRV_BUS5277_FLAG_HAVE_DATA)
	STD Z+_DRV_BUS5277_FLAGS,TEMP
	MCALL C5_DISPATCHER_UNLOCK
	CPI TEMP,0x00
	BREQ PC+0x02
	LDI TEMP_H,BUS5277_RESP_OK
	LDI TEMP_L,0x00
	MCALL _DRV_BUS5277_PROC_SLAVE_SEND
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T:
	;EXTRASCAN на базе таймингов
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP,Z+_DRV_BUS5277_EXTRASCAN_TIMER
	LDI TEMP_H,high(_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC)
	LDI TEMP_L,low(_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC)
	LDI ACCUM,0x01
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START
	LDD ACCUM,Z+_DRV_BUS5277_FLAGS
	ORI ACCUM,(1<<DRV_BUS5277_FLAG_EXSTRASCAN_BUSY)
	STD Z+_DRV_BUS5277_FLAGS,ACCUM
_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_LOOP:
	SBRC ACCUM,DRV_BUS5277_FLAG_EXSTRASCAN_BUSY
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_LOOP
	MCALL C5_DISPATCHER_UNLOCK
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC:
		PUSH_Z
		PUSH TEMP
		PUSH ACCUM

		MCALL C5_RAM_OFFSET

		LDD ACCUM,Z+_DRV_BUS5277_BUS_ADDR
		LSL ACCUM
		LDD TEMP,Z+_DRV_BUS5277_EXTRASCAN_CNTR
		INC TEMP
		STD Z+_DRV_BUS5277_EXTRASCAN_CNTR,TEMP
		CPI TEMP,0x00
		BRNE _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_PART1
		LDD ACCUM,Z+_DRV_BUS5277_FLAGS
		ANDI ACCUM,~(1<<DRV_BUS5277_FLAG_EXSTRASCAN_BUSY)
		STD Z+_DRV_BUS5277_FLAGS,ACCUM
		LDD TEMP,Z+_DRV_BUS5277_EXTRASCAN_TIMER
		MCALL C5_TIMER_STOP
		RJMP _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_END
	_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_PART1:
		CP ACCUM,TEMP
		BRNE _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_PART2
		;TODO Устанавливаем высокий уровень на BUS5277 TX и DR
		RJMP _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_END
	_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_PART2:
		DEC TEMP
		CP ACCUM,TEMP
		BRNE _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_END
		;TODO устанавливаем низкие уровни BUS5277 TX и DR

	_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN_T_PROC_END:
		POP ACCUM
		POP TEMP
		POP_Z
		RET

_DRV_BUS5277_PROC_SLAVE_RECV__END:
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_SEND:
;--------------------------------------------------------
;Режим SLAVE, передача
;IN: Z-выделенная память, Y-адрес на данные, TEMP_H-код
;функции, TEMP_L-длина данных
;OUT: TEMP_H-код ответа
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH LOOP_CNTR

	;Проверяю на максимальный размер пакета
	CPI TEMP_L,0x24+0x01
	BRCS PC+0x03
	LDI TEMP_H,BUS5277_FAULT
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__END

	LDI LOOP_CNTR,_DRV_BUS5277_MASTER_ADDR
	MCALL DRV_BUS5277__PACKET_PACK
	;Запускаем драйвер UART
	LDD TEMP,Z+_DRV_BUS5277_UART_DRV_ID
	MOV YH,ZH															;Адрес блока данных для передачи
	MOV YL,ZL
	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP_EH,0x04													;Длина данных для передачи
	ADD TEMP_EH,TEMP_L
	LDI TEMP_EL,0x00													;Максимальная длина данных для приема(только передача)
	MCALL C5_EXEC
	;Проверка ответа на ошибку
	CPI TEMP_H,DRV_UART_ST_READY
	LDI TEMP_H,BUS5277_RESP_NO_ANSWER
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__END
	LDI TEMP_H,BUS5277_RESP_OK
_DRV_BUS5277_PROC_SLAVE_SEND__END:

	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_EXTRASCAN:
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_EXTRASCAN_T:
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SET_HAVE_DATA:
;--------------------------------------------------------
;Режим SLAVE, прием
;IN: Z-выделенная память
;OUT: TEMP_H-код ответа,TEMP_L-длина данных, Y-адрес на
;блок данных
;--------------------------------------------------------
	PUSH TEMP
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP,Z+_DRV_BUS5277_FLAGS
	ORI TEMP,(1<<DRV_BUS5277_FLAG_HAVE_DATA)
	STD Z+_DRV_BUS5277_FLAGS,TEMP
	MCALL C5_DISPATCHER_UNLOCK
	POP TEMP
	RET
;--------------------------------------------------------
DRV_BUS5277__PACKET_PACK:
;--------------------------------------------------------
;Сборка пакета
;IN: Y-адрес на данные (0-копировать не нужно),
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
	SBRC YH,0x07
	RJMP _DRV_BUS5277__PACKET_PACK__ROM_DATA_COPY
_DRV_BUS5277__PACKET_PACK__RAM_DATA_COPY:
	;Проверка на необходимость копирования
	CPI YH,0x00
	BRNE PC+0x03
	CPI YL,0x00
	BREQ _DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE
	MOV ZH,XH
	MOV ZL,XL
	MOV XH,YH
	MOV XL,YL
	MCALL RAM_COPY8
	RJMP _DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE
_DRV_BUS5277__PACKET_PACK__ROM_DATA_COPY:
	ANDI YH,0x7f
	LSL YL
	ROL YH
	MOV ZH,YH
	MOV ZL,YL
	MOV YH,XH
	MOV YL,XL
	MCALL ROM_READ_BYTES
_DRV_BUS5277__PACKET_PACK__DATA_COPY_DONE:
	POP_X
	SUBI LOOP_CNTR,(0x100-0x03)

	LDI_Y CRC8_MAXIM
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
;IN: Y-SRC адрес
;OUT: TEMP_H-код функции,
;TEMP_L-длина данных, LOOP_CNTR-адрес устройства
;Y-адрес на данные
;--------------------------------------------------------
	PUSH_X
	PUSH ACCUM

	MOV XH,YH
	MOV XL,YL
	LD LOOP_CNTR,Y+
	LD TEMP_L,Y+
	CPI TEMP_L,DRV_BUS5277_MAX_PACKET_SIZE+0x01
	BRCC DRV_BUS5277__PACKET_UNPACK__INVALID_PACKET
	CPI TEMP_L,DRV_BUS5277_MIN_PACKET_SIZE
	BRCS DRV_BUS5277__PACKET_UNPACK__INVALID_PACKET
	SUBI TEMP_L,0x04
	LD TEMP_H,Y+
	;Вычисляю CRC
	PUSH LOOP_CNTR
	MOV LOOP_CNTR,TEMP_L
	SUBI LOOP_CNTR,(0x100-0x03)
	PUSH_Y
	LDI_Y CRC8_MAXIM
	MCALL CRC8_BLOCK
	POP_Y
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
	LDD TEMP,Z+_DRV_BUS5277_FLAGS
	SBRS TEMP,DRV_BUS5277_FLAG_LOGGING
	RET
	LDI TEMP,'>'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_EH
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
	RET
;--------------------------------------------------------
_DRV_BUS5277__LOG_RESP:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	LDD TEMP,Z+_DRV_BUS5277_FLAGS
	SBRS TEMP,DRV_BUS5277_FLAG_LOGGING
	RET
	LDI TEMP,'<'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_L
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
	RET
.endif
.endif
