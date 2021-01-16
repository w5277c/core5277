;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;03.05.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
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
	.include	"./conv/crc8_maxim_block.inc"
	.include	"./mem/ram_copy8.inc"
	.include	"./mem/rom_read_bytes.inc"

.ifdef LOGGING_PORT
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_numx8.inc"
	.include	"./core/log/log_strn.inc"
	.include	"./str/strn_trim.inc"
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
	.EQU	_DRV_BUS5277_FLAGS								= 0x2a;1B - флаги драйвера
	;---FLAGS---
	.EQU	DRV_BUS5277_FLAG_LOGGING						= 0x00;Бит логирования
	;---OPERATIONS---
	.EQU	DRV_BUS5277_OP_MASTER							= 0x00;Режим ведущего (запрос-ожидание ответа)
	.EQU	DRV_BUS5277_OP_SLAVE_RECV						= 0x01;Режим ведомого ожидание запроса
	.EQU	DRV_BUS5277_OP_SLAVE_SEND						= 0x02;Режим ведомого ответ на запрос
	.EQU	DRV_BUS5277_OP_EXTRASCAN						= 0x03;Режим EXTRASCAN - опрос адресов на наличие новых данных
	.EQU	DRV_BUS5277_OP_EXTRASCAN_T						= 0x04;Режим TIMING EXTRASCAN
	;---REQUEST-CODES---
	.EQU	BUS5277_FUNC_INFO									= 0x00;Чтение исномации об устройстве (тип, версия, uptime)
	.EQU	BUS5277_FUNC_DATA									= 0x01;Чтение данных
	.EQU	BUS5277_FUNC_READ_SETTINGS						= 0x02;Чтение блока настроек
	.EQU	BUS5277_FUNC_WRITE_SETTINGS					= 0x03;Запись блока настроек
	.EQU	BUS5277_FUNC_EXECUTE_COMMAND					= 0x04;Выполнение команды (таких как вкл/выкл/инвертировать)
	.EQU	BUS5277_FUNC_HISTORY_GET						= 0x05;Чтение первой записи истории
	.EQU	BUS5277_FUNC_HISTORY_CLEAR						= 0x06;Удаление первой записи истории

	.EQU	BUS5277_FUNC_RESET								= 0x7f;Сброс устройства к первоначальным настройкам
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
	.EQU	BUS5277_CMD_SRC_TRIGGER_ERR					= 0x0C;Триггер(ошибка датчика)
	;---REQUEST/RESPONSE-INTERNAL-ERROR-CODE
	.EQU	BUS5277_FAULT										= 0xff;Сбой в логике
	;---
	.EQU	DRV_BUS5277_MAX_PACKET_SIZE					= 0x28;Максимальный размер пакета 40B
	.EQU	DRV_BUS5277_MIN_PACKET_SIZE					= 0x04;Минимальный размер пакета 4B
	.EQU	_DRV_BUS5277_LISTEN_TIMEOUT					= 0x32;100 мс - таймаут для приема ответа
	.EQU	_DRV_BUS5277_MASTER_ADDR						= 0x00;Адрес мастера
	;---
	.EQU	_DRV_BUS5277_RAM_SIZE							= 0x2b;43B - буфер шины(40B) + переменные(3B)

;--------------------------------------------------------
DRV_BUS5277_INIT:
;--------------------------------------------------------
;Инициализация
;TEMP_H-адрес устройства на шине
;TEMP_L-номер флага EXTRASCAN_HAVE_DATA
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
;;;TODO	STD Z+DRV_BUS5277_FLAG_HAVE_DATA,TEMP_L
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
	CPI FLAGS,DRV_BUS5277_OP_MASTER
	BRNE PC+0x03
	MJMP  _DRV_BUS5277_PROC_MASTER
	CPI FLAGS,DRV_BUS5277_OP_SLAVE_RECV
	BRNE PC+0x03
	MJMP _DRV_BUS5277_PROC_SLAVE_RECV
	CPI FLAGS,DRV_BUS5277_OP_SLAVE_SEND
	BRNE PC+0x03
	MJMP _DRV_BUS5277_PROC_SLAVE_SEND
	CPI FLAGS,DRV_BUS5277_OP_EXTRASCAN
	BRNE PC+0x03
	MJMP _DRV_BUS5277_PROC_EXTRASCAN
	CPI FLAGS,DRV_BUS5277_OP_EXTRASCAN_T
	BRNE PC+0x03
	MJMP _DRV_BUS5277_PROC_EXTRASCAN_T
	LDI ACCUM,BUS5277_FAULT
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_MASTER:
;--------------------------------------------------------
;Режим MASTER
;IN: Y - адрес на данные, TEMP_H - код функции,
;TEMP_L - длина данных, LOOP_CNTR - адрес устройства
;OUT: ACCUM - код ответа,
;Z - адрес на блок данных, TEMP - длина данных
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	;Проверяю на максимальный размер пакета
	CPI TEMP_L,0x24+0x01
	BRCS PC+0x03
	LDI ACCUM,BUS5277_FAULT
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	;Формирую пакет
	PUSH_Z
	MOV ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x04)											;+0x04
	ST Z+,LOOP_CNTR													;Адрес устройства на шине
	ST Z+,ACCUM															;Длина данных всего пакета
	ST Z+,TEMP_H														;Код функции или ответа
	MOV LOOP_CNTR,TEMP_L
	;Проверяю наличие данных и копирую их
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277_PROC_MASTER__DATA_COPY_DONE
	SBRC YH,0x07
	RJMP _DRV_BUS5277_PROC_MASTER__ROM_DATA_COPY
_DRV_BUS5277_PROC_MASTER__RAM_DATA_COPY:
	MOV XH,YH
	MOV XL,YL
	MCALL RAM_COPY8
	RJMP _DRV_BUS5277_PROC_MASTER__DATA_COPY_DONE
_DRV_BUS5277_PROC_MASTER__ROM_DATA_COPY:
	ANDI YH,0b01111111
	LSL YL
	ROL YH
	PUSH_Z
	MOV XH,YH
	MOV XL,YL
	MOV YH,ZH
	MOV YL,ZL
	MOV ZH,XH
	MOV ZL,XL
	MCALL ROM_READ_BYTES
	POP_Z
_DRV_BUS5277_PROC_MASTER__DATA_COPY_DONE:
	ADD ZL,LOOP_CNTR
	LDI TEMP,0x00
	ADC ZH,TEMP
	POP_X
	PUSH LOOP_CNTR
	SUBI LOOP_CNTR,(0x100-0x03)
	MCALL CRC8_MAXIM_BLOCK
	POP LOOP_CNTR
	ST Z,ACCUM
	;Формирование пакета завершено
	;Восстанавливаем начальный адрес выделенной памяти
	MOV ZH,XH
	MOV ZL,XL


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
	;Проверка на наличие данных
	;MOV LOOP_CNTR,TEMP_L ???
	;CPI LOOP_CNTR,0x00 ???
	CPI TEMP_L,0x00
	LDI ACCUM,BUS5277_RESP_NO_DATA
	BREQ _DRV_BUS5277_PROC_MASTER__ERROR
.ifdef LOGGING_PORT
	MCALL _DRV_BUS5277__LOG_RESP
.endif
	;Считываю адрес
	LDD TEMP,Z+_DRV_BUS5277_PACKET_HEADER_ADDR
	MOV TEMP_H,TEMP
	ANDI TEMP_H,0x7f
	LDD TEMP_L,Z+_DRV_BUS5277_BUS_ADDR
	;Проверяю адрес на совпадение
	CP TEMP_H,TEMP_L
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	BRNE _DRV_BUS5277_PROC_MASTER__ERROR
	;Проверяю бит extrascan
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	SBRC TEMP,0x07
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR
	;Блок обычного режима(не EXTRASCAN)
	;Проверяем бит ответа
	LDD TEMP_H,Z+_DRV_BUS5277_PACKET_HEADER_CODE
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	SBRS TEMP_H,0x07
	RJMP _DRV_BUS5277_PROC_MASTER__ERROR
	;Получаю длину пакета
	LDD LOOP_CNTR,Z+_DRV_BUS5277_PACKET_HEADER_SIZE
	ANDI LOOP_CNTR,0x3f
	;Вычисляю CRC
	MOV XH,ZH
	MOV XL,ZH
	MCALL CRC8_MAXIM_BLOCK
	DEC LOOP_CNTR
	ADD XL,LOOP_CNTR
	CLR TEMP
	ADC XH,TEMP
	LD TEMP,X
	CP TEMP,ACCUM
	LDI ACCUM,BUS5277_RESP_INVALID_PACKET
	BREQ _DRV_BUS5277_PROC_MASTER__ERROR
	;Помещаю в ACCUM код ответа
	MOV ACCUM,TEMP_H
	ANDI ACCUM,0x7f
	;Помещаю в Z адрес на блок данных
	LDI TEMP_L,DRV_BUS5277_PACKET_DATA
	ADD ZL,TEMP_L
	LDI TEMP_L,0x00
	ADC ZH,TEMP_L
	;Вычисляю длину данных
	SUBI LOOP_CNTR,0x03
	MOV TEMP,LOOP_CNTR
	RJMP _DRV_BUS5277_PROC_MASTER__END
_DRV_BUS5277_PROC_MASTER__ERROR:
_DRV_BUS5277_PROC_MASTER__END:
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_RECV:
;--------------------------------------------------------
;Режим SLAVE, прием
;OUT: ACCUM - код ответа(0xff - сбой),
;Z - адрес на блок данных, TEMP - длина данных
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

_DRV_BUS5277_PROC_SLAVE_RECV__LOOP:
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET

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
	LDI ACCUM,BUS5277_FAULT
	BRNE _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	CPI TEMP_L,0x00
	LDI ACCUM,BUS5277_FAULT
	BREQ _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	;Считываю адрес
	LDD TEMP_H,Z+_DRV_BUS5277_PACKET_HEADER_ADDR
	LDD TEMP,Z+_DRV_BUS5277_BUS_ADDR
	MOV TEMP_L,TEMP_H
	ANDI TEMP_H,0x7f
	;Проверяю адрес на совпадение
	CP TEMP_H,TEMP
	BRNE _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	;Проверяю бит extrascan
	SBRC TEMP,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN
	;Блок обычного режима(не EXTRASCAN)
	;Проверяем бит ответа
	LDD TEMP_H,Z+_DRV_BUS5277_PACKET_HEADER_CODE
	SBRC TEMP_H,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	;Получаю длину пакета
	LDD LOOP_CNTR,Z+_DRV_BUS5277_PACKET_HEADER_SIZE
	ANDI LOOP_CNTR,0x3f
	DEC LOOP_CNTR
	;Вычисляю CRC
	MOV XH,ZH
	MOV XL,ZL
	MCALL CRC8_MAXIM_BLOCK
	ADD XL,LOOP_CNTR
	CLR TEMP
	ADC XH,TEMP
	LD TEMP,X
	CP TEMP,ACCUM
	BREQ PC+0x02
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__LOOP
	;Помещаю в ACCUM код запроса
	MOV ACCUM,TEMP_H
	ANDI ACCUM,0x7f

	;Помещаю в Z адрес на блок данных
	LDI TEMP_L,DRV_BUS5277_PACKET_DATA
	ADD ZL,TEMP_L
	LDI TEMP_L,0x00
	ADC ZH,TEMP_L
	;Вычисляю длину данных
	SUBI LOOP_CNTR,0x03
	MOV TEMP,LOOP_CNTR
	RJMP _DRV_BUS5277_PROC_SLAVE_RECV__END
_DRV_BUS5277_PROC_SLAVE_RECV__EXTRASCAN:
	;TODO проверяем тип EXTRASCAN
	;TODO считываем флаг HAVE_DATA, сбрасываем его, передаем ответ и начинаем процедуру заново

_DRV_BUS5277_PROC_SLAVE_RECV__END:
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_BUS5277_PROC_SLAVE_SEND:
;--------------------------------------------------------
;Режим SLAVE, передача
;IN: Y - адрес на данные, TEMP_H - код функции,
;TEMP_L - длина данных
;OUT: ACCUM - код ответа
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	;Проверяю на максимальный размер пакета
	CPI TEMP_L,0x24+0x01
	BRCS PC+0x03
	LDI ACCUM,BUS5277_FAULT
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__ERROR
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	;Формирую пакет
	PUSH_Z
	MOV ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x04)											;+0x04
	LDI TEMP,_DRV_BUS5277_MASTER_ADDR
	ST Z+,TEMP															;Адрес ведущего на шине
	ST Z+,ACCUM															;Длина данных всего пакета
	ST Z+,TEMP_H														;Код функции или ответа
	MOV LOOP_CNTR,TEMP_L
	;Проверяю наличие данных и копирую их
	CPI TEMP_L,0x00
	BREQ _DRV_BUS5277_PROC_SLAVE_SEND__DATA_COPY_DONE
	SBRC YH,0x07
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__ROM_DATA_COPY
_DRV_BUS5277_PROC_SLAVE_SEND__RAM_DATA_COPY:
	;Проверяю на адрес буфера
	CP YH,ZH
	BRNE PC+0x03
	CP YL,ZL
	BREQ _DRV_BUS5277_PROC_SLAVE_SEND__DATA_COPY_DONE
	;Копирую данные в буфер
	MOV XH,YH
	MOV XL,YL
	MCALL RAM_COPY8
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__DATA_COPY_DONE
_DRV_BUS5277_PROC_SLAVE_SEND__ROM_DATA_COPY:
	ANDI YH,0b01111111
	LSL YL
	ROL YH
	PUSH_Z
	MOV XH,YH
	MOV XL,YL
	MOV YH,ZH
	MOV YL,ZL
	MOV ZH,XH
	MOV ZL,XL
	MCALL ROM_READ_BYTES
	POP_Z
_DRV_BUS5277_PROC_SLAVE_SEND__DATA_COPY_DONE:
	ADD ZL,LOOP_CNTR
	LDI TEMP,0x00
	ADC ZH,TEMP
	POP_X
	PUSH LOOP_CNTR
	SUBI LOOP_CNTR,(0x100-0x03)
	MCALL CRC8_MAXIM_BLOCK
	POP LOOP_CNTR
	ST Z,ACCUM
	;Формирование пакета завершено
	;Восстанавливаем начальный адрес выделенной памяти
	MOV ZH,XH
	MOV ZL,XL
	;Запускаем драйвер UART
	LDD TEMP,Z+_DRV_BUS5277_UART_DRV_ID
	MOV YH,ZH															;Адрес блока данных для передачи
	MOV YL,ZL
	LDI TEMP_EH,0x04													;Длина данных для передачи
	ADD TEMP_EH,LOOP_CNTR
	LDI TEMP_EL,0x00													;Максимальная длина данных для приема(только передача)
	MCALL C5_EXEC
	;Проверка ответа на ошибку
	CPI TEMP_H,DRV_UART_ST_READY
	LDI ACCUM,BUS5277_RESP_NO_ANSWER
	BRNE _DRV_BUS5277_PROC_SLAVE_SEND__ERROR
	LDI ACCUM,BUS5277_RESP_OK
	RJMP _DRV_BUS5277_PROC_SLAVE_SEND__END
_DRV_BUS5277_PROC_SLAVE_SEND__ERROR:
_DRV_BUS5277_PROC_SLAVE_SEND__END:

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
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

