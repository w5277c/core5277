;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.04.2020	w5277c@gmail.com			Начало
;19.04.2020	w5277c@gmail.com			Адаптировано под изменения ядра
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;16.03.2021	w5277c@gmail.com			Устранено несколько серьезных ошибок в логике приема данных
;18.03.2021	w5277c@gmail.com			C5_WAIT используется только в ожидании начала приема данных
;07.06.2021	w5277c@gmail.com			Ввод функционала MAGIC(незамедлительная реакция при получении определенных 2х байт)
;14.06.2021	w5277c@gmail.com			Корректная передача MAGIC PID
;04.07.2021	w5277c@gmail.com			Z->Y, необходимо тестирование
;18.08.2021	w5277c@gmail.com			Вынесена проверка на значение порта 0xff в процедуры порта
;26.02.2023	konstantin@5277.ru			Исправлена критическая ошибка
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_UART_H
.else
.set DEF__C5_DRV_UART_H = 1
.message "Included driver hardware UART v0.7"

.include "./core/drivers/_uart.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start_at.inc"
.include	"./core/timer_stop.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_H_RX_PORT					= 0x00			;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_H_TX_PORT					= 0x01			;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_H_DR_PORT					= 0x02			;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_H_LED_PORT					= 0x03			;1B - Порт индикации
	.EQU	_DRV_UART_H_STATE						= 0x04			;1B - Статус драйвера
	.EQU	_DRV_UART_H_TIMER_ID					= 0x05			;1B - ИД программного таймера
	.EQU	_DRV_UART_H_OPTIONS					= 0x06			;1B - опции
	;---
	.EQU	_DRV_UART_H_SRC_ADDR					= 0x07			;2B - адрес блока данных для передачи
	.EQU	_DRV_UART_H_DST_ADDR					= 0x09			;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_UART_H_BUFFER_POS				= 0x0b			;1B - Позиция в буфере при передаче
	.EQU	_DRV_UART_H_TX_LEN					= 0x0c			;1B - Длина данных для передачи
	.EQU	_DRV_UART_H_RX_LEN					= 0x0d			;1B - Максимальный размер данных для приема
	;---
	.EQU	_DRV_UART_H_FIRST_TIMEOUT			= 0x0e			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_UART_H_NEXT_TIMEOUT			= 0x10			;1B - Таймаут ожидания последующего байта(0.050ms)
	;---MAGIC---														;Используется только при включенной опции
	.EQU	_DRV_UART_H_MAGIC						= 0x11			;2B - MAGIC двухбайтное число
	.EQU	_DRV_UART_H_MAGIC_PID				= 0x13			;1B - PID процесса для процедуры MAGIC
	.EQU	_DRV_UART_H_MAGIC_ADDR				= 0x14			;2B - Адрес процедуры при обнаружении MAGIC

	.EQU	_DRV_UART_H_RAM_SIZE					= 0x16			;22 байт необходимо выделить

	;---BAUDRATE_IDS---
	.EQU	DRV_UART_H_BAUDRATE_4800			= 62500*CORE_FREQ/4800-1
	.EQU	DRV_UART_H_BAUDRATE_9600			= 62500*CORE_FREQ/9600-1
	.EQU	DRV_UART_H_BAUDRATE_14400			= 62500*CORE_FREQ/14400-1
	.EQU	DRV_UART_H_BAUDRATE_19200			= 62500*CORE_FREQ/19200-1
	.EQU	DRV_UART_H_BAUDRATE_28800			= 62500*CORE_FREQ/28800-1
	.EQU	DRV_UART_H_BAUDRATE_38400			= 62500*CORE_FREQ/38400-1
	.EQU	DRV_UART_H_BAUDRATE_57600			= 62500*CORE_FREQ/57600-1
	.EQU	DRV_UART_H_BAUDRATE_76800			= 62500*CORE_FREQ/76800-1
	.EQU	DRV_UART_H_BAUDRATE_115200			= 62500*CORE_FREQ/115200-1

;--------------------------------------------------------
DRV_UART_H_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт направления передача/прием
;(0xff-не используется),
;TEMP_L-порт индикации передачи
;(0xff-не используется)
;TEMP_EH-ид таймера, FLAGS - опции
;TEMP_EL-скорость передачи данных
;--------------------------------------------------------
	LDI ACCUM,_DRV_UART_H_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Задаем скорость передачи данных
	STS UBRR0L,TEMP_EL
	STS UBRR0H,C0x00
	;Инициализирую и записываю программный таймер
	PUSH TEMP_L
	PUSH TEMP_H
	LDI TEMP_H,high(_DRV_UART_H_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_UART_H_TIMER_EVENT)
	MOV TEMP,TEMP_EH
	MCALL C5_TIMER_SET
	STD Y+_DRV_UART_H_TIMER_ID,TEMP_EH
	;Записываю опции
	STD Y+_DRV_UART_H_OPTIONS,FLAGS
	;Записываю основные порты для совместимости с другими реализациями UART
	LDI TEMP,TXD
	STD Y+_DRV_UART_H_TX_PORT,TEMP
	LDI TEMP,RXD
	STD Y+_DRV_UART_H_RX_PORT,TEMP
	;Инициализирую порт направления данных
	POP ACCUM
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	STD Y+_DRV_UART_H_DR_PORT,ACCUM
	;Инициализирую порт индикации
	POP ACCUM
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	STD Y+_DRV_UART_H_LED_PORT,ACCUM
	;Инициализирую порт RX
	LDI ACCUM,RXD
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	;Инициализирую порт TX
	LDI ACCUM,TXD
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	;Расчитываю NEXT TIMEOUT										;Для 4800 таймаут составит 2.6ms(~13бит), для 115200 составит 0.150ms(~17бит)
	LSR TEMP_EL															;Таймаут будет базироваться на _C5_MAIN_TIMER_CNTR(отсчет каждые 0.050ms)
	STD Y+_DRV_UART_H_NEXT_TIMEOUT,TEMP_EL
	;Отключаю прием и передачу
	STS UCSR0B,C0x00
	;Сбрасываю флаги
	LDI TEMP,(1<<RXC0)|(1<<TXC0)
	STS UCSR0A,TEMP
	;Async, no parity, 1 stop bits, 8 data bits
	LDI TEMP,low((1<<URSEL0)|(1<<UCSZ01)|(1<<UCSZ00))		;URSEL необходим для ATmega16, в остальных должен быть равен 0
	STS UCSR0C,TEMP
	;Устанавливаю векторы прерывания TX и RX
	LDI ACCUM,C5_IR_UART_RX
	LDI TEMP_H,high(_DRV_UART_H_RX_EVENT)
	LDI TEMP_L,low(_DRV_UART_H_RX_EVENT)
	MCALL C5_IR_VECTOR_SET
	LDI ACCUM,C5_IR_UART_TX
	LDI TEMP_H,high(_DRV_UART_H_TX_EVENT)
	LDI TEMP_L,low(_DRV_UART_H_TX_EVENT)
	MCALL C5_IR_VECTOR_SET
	;Подпитываю UART
	_C5_POWER_ON PRUSART0

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
;TEMP_EH-TX длина, TEMP_EL-RX максимальная длина
;TEMP_H,TEMP_L-таймаут ожидания данных(2ms)
;LOOP_CNTR,FLAGS-MAGIC слово, Y-адрес процедуры MAGIC
;TRY_CNTR-PID вызывающего процесса
;(YH=0xff не использовать MAGIC)
;OUT: TEMP_H-результат(состояние UART'а),
;TEMP_L-длина полученных данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	PUSH_Y
	MCALL C5_RAM_OFFSET

	;Запоминаем таймаут ожидания первого байта
	STD Y+_DRV_UART_H_FIRST_TIMEOUT+0x00,TEMP_H
	STD Y+_DRV_UART_H_FIRST_TIMEOUT+0x01,TEMP_L
	;Запоминаем SRC
	;Умножаем на 2 адрес работы с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_UART_H_SRC_ADDR+0x00,ZH
	STD Y+_DRV_UART_H_SRC_ADDR+0x01,ZL
	;Запоминаем DST
	STD Y+_DRV_UART_H_DST_ADDR+0x00,XH
	STD Y+_DRV_UART_H_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Y+_DRV_UART_H_TX_LEN,TEMP_EH
	STD Y+_DRV_UART_H_RX_LEN,TEMP_EL

	POP TEMP
	STD Y+_DRV_UART_H_MAGIC_ADDR+0x01,TEMP
	POP TEMP
	STD Y+_DRV_UART_H_MAGIC_ADDR+0x00,TEMP
	STD Y+_DRV_UART_H_MAGIC+0x00,LOOP_CNTR
	STD Y+_DRV_UART_H_MAGIC+0x01,FLAGS
	STD Y+_DRV_UART_H_MAGIC_PID,TRY_CNTR

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_H_PROC__TX_SKIP
	;Включаю индикатор передачи
	LDD ACCUM,Y+_DRV_UART_H_LED_PORT
	MCALL PORT_SET_HI
	;Сброс позиции в буфере
	CLR TEMP
	STD Y+_DRV_UART_H_BUFFER_POS,TEMP
	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Y+_DRV_UART_H_STATE,TEMP
	;Разрешаю UART(передача)
	LDS TEMP,UCSR0B
	ANDI TEMP,low(~((1<<RXCIE0)|(1<<RXEN0)))
	ORI TEMP,(1<<TXCIE0)|(1<<TXEN0)
	STS UCSR0B,TEMP
	;Отправляю первый байт
	MCALL _DRV_UART_H_TX_EVENT
	;Жду окончания передачи
_DRV_UART_H_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_H_PROC__SENDING_WAIT
	;Выключаю индикатор
	LDD ACCUM,Y+_DRV_UART_H_LED_PORT
	MCALL PORT_SET_LO
_DRV_UART_H_PROC__TX_SKIP:

	;Сброс позиции в буфере
	CLR TEMP
	STD Y+_DRV_UART_H_BUFFER_POS,TEMP

	;Блок приема
	CPI TEMP_EL,0x00
	BREQ _DRV_UART_H_PROC__RX_SKIP

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Y+_DRV_UART_H_STATE,TEMP
	;Разрешаю UART(прием)
	LDS TEMP,UCSR0B
	ANDI TEMP,~((1<<TXCIE0)|(1<<TXEN0))
	ORI TEMP,(1<<RXCIE0)|(1<<RXEN0)
	STS UCSR0B,TEMP
	;Запоминаем метку времени для отсчета таймаута ожидания данных
	CLI
	LDS ZH,_C5_UPTIME+0x03
	LDS ZL,_C5_UPTIME+0x04
	SEI
_DRV_UART_H_PROC__RECEIVING_WAIT:
	LDD TEMP,Y+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE _DRV_UART_H_PROC__RECEIVING_NOT_WAIT_DATA
	CLI
	LDS TEMP_H,_C5_UPTIME+0x03
	LDS TEMP_L,_C5_UPTIME+0x04
	SEI
	SUB TEMP_L,ZL
	SBC TEMP_H,ZH
	LDD TEMP,Y+_DRV_UART_H_FIRST_TIMEOUT+0x00
	LDD ACCUM,Y+_DRV_UART_H_FIRST_TIMEOUT+0x01
	CPI TEMP,0x00
	BRNE _DRV_UART_H_PROC__RECEIVING_NOT_ZERO_TIMEOUT
	CPI ACCUM,0x00
	BREQ _DRV_UART_H_PROC__RECEIVING_WAIT
_DRV_UART_H_PROC__RECEIVING_NOT_ZERO_TIMEOUT:
	CP TEMP,TEMP_H
	BRCS _DRV_UART_H_PROC__RECEIVING_TIMEOUT
	BRNE _DRV_UART_H_PROC__RECEIVING_WAIT
	CP ACCUM,TEMP_L
	BRCS _DRV_UART_H_PROC__RECEIVING_TIMEOUT
	RJMP _DRV_UART_H_PROC__RECEIVING_WAIT
_DRV_UART_H_PROC__RECEIVING_NOT_WAIT_DATA:
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_H_PROC__RECEIVING_WAIT
	RJMP _DRV_UART_H_PROC__RECEIVING_DONE
_DRV_UART_H_PROC__RECEIVING_TIMEOUT:
	LDI TEMP,DRV_UART_ST_FAIL
	STD Y+_DRV_UART_H_STATE,TEMP
_DRV_UART_H_PROC__RECEIVING_DONE:
_DRV_UART_H_PROC__RX_SKIP:
	;Запрещаю UART
	LDS TEMP,UCSR0B
	ANDI TEMP,low(~((1<<TXCIE0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<RXEN0)))
	STS UCSR0B,TEMP

	;Результат
	LDD TEMP_H,Y+_DRV_UART_H_STATE
	LDD TEMP_L,Y+_DRV_UART_H_BUFFER_POS
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_UART_H_TX_EVENT:
;--------------------------------------------------------
;Обработчик прерывания TX
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	;Считываю позицию в буфере
	LDD TEMP_L,Y+_DRV_UART_H_BUFFER_POS
	;Считываем размер передаваемых данных
	LDD TEMP_H,Y+_DRV_UART_H_TX_LEN
	;Проверяем на первый байт
	CPI TEMP_L,0x00
	BRNE _DRV_UART_H_TX_EVENT__SKIP_DR
	;Порт направления в HI
	LDD ACCUM,Y+_DRV_UART_H_DR_PORT
	MCALL PORT_SET_HI
_DRV_UART_H_TX_EVENT__SKIP_DR:
	;Если все данные переданы то завершаем работу TX
	CP TEMP_H,TEMP_L
	BREQ _DRV_UART_H_TX_EVENT__DONE
	;Инкрементируем и записываепм позицию в буфере
	MOV TEMP,TEMP_L
	INC TEMP
	STD Y+_DRV_UART_H_BUFFER_POS,TEMP
	;Вычисляю смещение на нужный байт
	LDD ZH,Y+_DRV_UART_H_SRC_ADDR+0x00
	LDD ZL,Y+_DRV_UART_H_SRC_ADDR+0x01
	MOV ACCUM,ZH
	ANDI ZH,0x7f
	ADD ZL,TEMP_L
	ADC ZH,C0x00
	;Считываем с буфера байт и передаем
	SBRC ACCUM,0x07
	RJMP PC+0x03
	LD TEMP,Z
	RJMP PC+02
	LPM TEMP,Z+
	STS UDR0,TEMP
	RJMP _DRV_UART_H_TX_EVENT__END
_DRV_UART_H_TX_EVENT__DONE:
	;Отключаем передатчик
	LDS TEMP,UCSR0B
	ANDI TEMP,~((1<<TXCIE0)|(1<<TXEN0))
	STS UCSR0B,TEMP
	;Проверяем на опцию BREAK
	LDD TEMP,Y+_DRV_UART_H_OPTIONS
	SBRS TEMP,DRV_UART_OPT_BREAK
	RJMP _DRV_UART_H_TX_EVENT__NO_BREAK
	;Выставляем TX в низкий уровень
	LDI ACCUM,TXD
	MCALL PORT_SET_LO
	LDI TEMP,DRV_UART_ST_BREAK
	STD Y+_DRV_UART_H_STATE,TEMP
	;Обновляю таймер на отсчет таймаута для посылки BREAK
	LDD ACCUM,Y+_DRV_UART_H_NEXT_TIMEOUT
	MOV TEMP,ACCUM
	LSR TEMP
	ADD ACCUM,TEMP
	LDD TEMP,Y+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_START_AT	;TODO оптимизировать
	RJMP _DRV_UART_H_TX_EVENT__END
_DRV_UART_H_TX_EVENT__NO_BREAK:
	;Выставляем TX в высокий уровень
	LDI ACCUM,TXD
	MCALL PORT_SET_HI
	;Порт направления в LO
	LDD ACCUM,Y+_DRV_UART_H_DR_PORT
	MCALL PORT_SET_LO
	;Останавливаю таймер
	LDD TEMP,Y+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_STOP
	;Устанавливаю состояние READY
	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_H_STATE,TEMP

_DRV_UART_H_TX_EVENT__END:
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_UART_H_RX_EVENT:
;--------------------------------------------------------
;Обработчик прерывания RX
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	LDS FLAGS,UDR0

	MCALL C5_RAM_OFFSET

	;Считываю позицию в буфере
	LDD ACCUM,Y+_DRV_UART_H_BUFFER_POS
	;Проверяю на первый байт
	CPI ACCUM,0x00
	BRNE _DRV_UART_H_RX_EVENT__NOT_FIRST_BYTE
	LDI TEMP,DRV_UART_ST_RECEIVING
	STD Y+_DRV_UART_H_STATE,TEMP
_DRV_UART_H_RX_EVENT__NOT_FIRST_BYTE:
	;Проверяю на корректное состояние
	LDD TEMP,Y+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE _DRV_UART_H_RX_EVENT__END
	;Проверяю на максимальный размер
	LDD TEMP,Y+_DRV_UART_H_RX_LEN
	CP ACCUM,TEMP
	BRCS _DRV_UART_H_RX_EVENT__NO_OVERSIZE
	LDI TEMP,DRV_UART_ST_FAIL
	STD Y+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_RX_EVENT__TIMER_UPDATE
_DRV_UART_H_RX_EVENT__NO_OVERSIZE:
	LDS TEMP,UCSR0A
	SBRS TEMP,FE0
	RJMP _DRV_UART_H_RX_EVENT__NO_BREAK
	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_RX_EVENT__END
_DRV_UART_H_RX_EVENT__NO_BREAK:
	;Записываю байт
	LDD ZH,Y+_DRV_UART_H_DST_ADDR+0x00
	LDD ZL,Y+_DRV_UART_H_DST_ADDR+0x01
	ADD ZL,ACCUM
	ADC ZH,C0x00
	ST Z,FLAGS
	;Инкрементирую и записываю позицию в буфере
	INC ACCUM
	STD Y+_DRV_UART_H_BUFFER_POS,ACCUM
_DRV_UART_H_RX_EVENT__TIMER_UPDATE:
	;Обновляю таймер на отсчет таймаута ожидания следующего байта
	PUSH ACCUM
	LDD TEMP,Y+_DRV_UART_H_TIMER_ID
	LDD ACCUM,Y+_DRV_UART_H_NEXT_TIMEOUT
	MCALL C5_TIMER_START_AT		;TODO оптимизировать
	POP ACCUM
	;Проверка MAGIC
	CPI ACCUM,0x02														;Получили 2 байта?
	BRNE _DRV_UART_H_RX_EVENT__END
	LDD ACCUM,Y+_DRV_UART_H_OPTIONS
	SBRS ACCUM,DRV_UART_OPT_MAGIC									;Опция MAGIC включена?
	RJMP _DRV_UART_H_RX_EVENT__END

	LDD ZH,Y+_DRV_UART_H_MAGIC_ADDR+0x00
	LDD ZL,Y+_DRV_UART_H_MAGIC_ADDR+0x01
	LDD TEMP,Y+_DRV_UART_H_MAGIC_ADDR+0x00
	CPI ZH,0xff															;Адрес обработчика MAGIC указан?
	BREQ _DRV_UART_H_RX_EVENT__END
	LDD ACCUM,Y+_DRV_UART_H_MAGIC+0x01
	CP ACCUM,FLAGS														;2-й принятый байт равен второму байту MAGIC?
	BRNE _DRV_UART_H_RX_EVENT__END
	LDD ACCUM,Y+_DRV_UART_H_MAGIC+0x00
	LDD ZH,Y+_DRV_UART_H_DST_ADDR+0x00
	LDD ZL,Y+_DRV_UART_H_DST_ADDR+0x01
	LD FLAGS,Z
	CP ACCUM,FLAGS														;1-й принятый байт равен первому байту MAGIC?
	BRNE _DRV_UART_H_RX_EVENT__END
	PUSH PID
	LDD PID,Y+_DRV_UART_H_MAGIC_PID
	LDD ZL,Y+_DRV_UART_H_MAGIC_ADDR+0x01
	MOV ZH,TEMP
	ICALL																	;Не нужно сохранять FLAGS,ACCUM,TEMP,Z,Y
	POP PID

	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_H_STATE,TEMP
_DRV_UART_H_RX_EVENT__END:

	POP FLAGS
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_UART_H_TIMER_EVENT:
;--------------------------------------------------------
;Событие от таймера
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	LDD TEMP,Y+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE _DRV_UART_H_TIMER_EVENT__NOT_RECEIVING
	LDD TEMP,Y+_DRV_UART_H_BUFFER_POS
	LDI ACCUM,DRV_UART_ST_FAIL
	CPI TEMP,0x00
	BREQ PC+0x02
	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_TIMER_EVENT__END

_DRV_UART_H_TIMER_EVENT__NOT_RECEIVING:
	;Останавливаю таймер
	LDD TEMP,Y+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_STOP
	CPI TEMP,DRV_UART_ST_BREAK
	BRNE _DRV_UART_H_TIMER_EVENT__END
	;Порт направления в LO
	LDD ACCUM,Y+_DRV_UART_H_DR_PORT
	MCALL PORT_SET_LO
	;Устанавливаю состояние READY
	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_H_STATE,TEMP

_DRV_UART_H_TIMER_EVENT__END:
	POP ACCUM
	POP TEMP
	RET
.endif
