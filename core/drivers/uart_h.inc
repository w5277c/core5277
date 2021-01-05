;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.04.2020  w5277c@gmail.com			Начало
;19.04.2020  w5277c@gmail.com			Адаптировано под изменения ядра
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include "./core/drivers/_uart.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./core/wait.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/timer_stop.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_H_DR_PORT					= 0x00			;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_H_LED_PORT					= 0x01			;1B - Порт индикации
	.EQU	_DRV_UART_H_STATE						= 0x02			;1B - Статус драйвера
	.EQU	_DRV_UART_H_TIMER_ID					= 0x03			;1B - ИД программного таймера
	.EQU	_DRV_UART_H_OPTIONS					= 0x04			;1B - опции
	;---
	.EQU	_DRV_UART_H_SRC_ADDR					= 0x05			;2B - адрес блока данных для передачи
	.EQU	_DRV_UART_H_DST_ADDR					= 0x07			;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_UART_H_BUFFER_POS				= 0x09			;1B - Позиция в буфере при передаче
	.EQU	_DRV_UART_H_TX_LEN					= 0x0a			;1B - Длина данных для передачи
	.EQU	_DRV_UART_H_RX_LEN					= 0x0b			;1B - Максимальный размер данных для приема
	;---
	.EQU	_DRV_UART_H_FIRST_TIMEOUT			= 0x0c			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_UART_H_NEXT_TIMEOUT			= 0x0e			;1B - Таймаут ожидания последующего байта(0.050ms)

	.EQU	_DRV_UART_H_RAM_SIZE					= 0x0f			;15 байт необходимо выделить

	;---BAUDRATE_IDS---
	.EQU	DRV_UART_H_BAUDRATE_4800			= 0xcf
	.EQU	DRV_UART_H_BAUDRATE_9600			= 0x67
	.EQU	DRV_UART_H_BAUDRATE_14400			= 0x44
	.EQU	DRV_UART_H_BAUDRATE_19200			= 0x33
	.EQU	DRV_UART_H_BAUDRATE_28800			= 0x22
	.EQU	DRV_UART_H_BAUDRATE_38400			= 0x19
	.EQU	DRV_UART_H_BAUDRATE_57600			= 0x10
	.EQU	DRV_UART_H_BAUDRATE_76800			= 0x0c
	.EQU	DRV_UART_H_BAUDRATE_115200			= 0x08

;--------------------------------------------------------
DRV_UART_H_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H - порт направления передача/прием
;(0xff-не используется),
;TEMP_L - порт индикации передачи
;(0xff-не используется)
;ACCUM - ид таймера
;FLAGS - опции
;YH - скорость передачи данных
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH YH

	PUSH ACCUM
	LDI ACCUM,_DRV_UART_H_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;Задаем скорость передачи данных
	STS UBRR0L,YH
	CLR TEMP
	STS UBRR0H,TEMP
	;Инициализирую и записываю программный таймер
	PUSH TEMP_L
	PUSH TEMP_H
	LDI TEMP_H,high(_DRV_UART_H_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_UART_H_TIMER_EVENT)
	MOV TEMP,ACCUM
	MCALL C5_TIMER_SET
	STD Z+_DRV_UART_H_TIMER_ID,ACCUM
	;Записываю опции
	STD Z+_DRV_UART_H_OPTIONS,FLAGS
	;Инициализирую порт направления данных
	POP ACCUM
	CPI ACCUM,0xff
	BREQ _DRV_UART_H_INIT__NO_DR_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
_DRV_UART_H_INIT__NO_DR_PORT:
	STD Z+_DRV_UART_H_DR_PORT,ACCUM
	;Инициализирую порт индикации
	POP ACCUM
	CPI ACCUM,0xff
	BREQ _DRV_UART_H_INIT__NO_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
_DRV_UART_H_INIT__NO_LED_PORT:
	STD Z+_DRV_UART_H_LED_PORT,ACCUM
	;Инициализирую порт RX
	LDI ACCUM,RXD
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	;Инициализирую порт TX
	LDI ACCUM,TXD
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	;Расчитываю NEXT TIMEOUT										;Для 4800 таймаут составит 2.6ms(~13бит), для 115200 составит 0.150ms(~17бит)
	LSR YH																;Таймаут будет базироваться на _C5_MAIN_TIMER_CNTR(отсчет каждые 0.050ms)
	LSR YH
	INC YH
	STD Z+_DRV_UART_H_NEXT_TIMEOUT,YH
	;Отключаю прием и передачу
	LDI TEMP,0x00
	STS UCSRB,TEMP
	;Сбрасываю флаги
	LDI TEMP,(1<<RXC)|(1<<TXC)
	STS UCSRA,TEMP
	;Async, no parity, 1 stop bits, 8 data bits
	LDI TEMP,low((1<<URSEL)|(1<<UCSZ01)|(1<<UCSZ00))		;URSEL необходим для ATmega16, в остальных должен быть равен 0
	STS UCSRC,TEMP
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

	POP YH
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y - src адрес, X - dst адрес,
;TEMP_EH - TX длина, TEMP_EL - RX максимальная длина
;TEMP_H,TEMP_L - таймаут ожидания данных(2ms)
;OUT: TEMP_H - результат(состояние UART'а),
;TEMP_L - длина полученных данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	;Запоминаем таймаут ожидания первого байта
	STD Z+_DRV_UART_H_FIRST_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_UART_H_FIRST_TIMEOUT+0x01,TEMP_L
	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_UART_H_SRC_ADDR+0x00,YH
	STD Z+_DRV_UART_H_SRC_ADDR+0x01,YL
	;Запоминаем DST
	STD Z+_DRV_UART_H_DST_ADDR+0x00,XH
	STD Z+_DRV_UART_H_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Z+_DRV_UART_H_TX_LEN,TEMP_EH
	STD Z+_DRV_UART_H_RX_LEN,TEMP_EL
	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_H_PROC__TX_SKIP
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_UART_H_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_UART_H_BUFFER_POS,TEMP
	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Z+_DRV_UART_H_STATE,TEMP
	;Разрешаю UART(передача)
	LDS TEMP,UCSRB
	ANDI TEMP,low(~((1<<RXCIE0)|(1<<RXEN0)))
	ORI TEMP,(1<<TXCIE0)|(1<<TXEN0)
	STS UCSRB,TEMP
	;Отправляю первый байт
	MCALL _DRV_UART_H_TX_EVENT
	;Жду окончания передачи
_DRV_UART_H_PROC__SENDING_WAIT:
	MCALL C5_WAIT
	LDD TEMP,Z+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_H_PROC__SENDING_WAIT
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_UART_H_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
_DRV_UART_H_PROC__TX_SKIP:

	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_UART_H_BUFFER_POS,TEMP

	;Блок приема
	CPI TEMP_EL,0x00
	BREQ _DRV_UART_H_PROC__RX_SKIP

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Z+_DRV_UART_H_STATE,TEMP
	;Разрешаю UART(прием)
	LDS TEMP,UCSRB
	ANDI TEMP,~((1<<TXCIE0)|(1<<TXEN0))
	ORI TEMP,(1<<RXCIE)|(1<<RXEN)
	STS UCSRB,TEMP
	;Запоминаем метку времени для отсчета таймаута ожидания данных
	LDS YH,_C5_UPTIME+0x03
	LDS YL,_C5_UPTIME+0x04
_DRV_UART_H_PROC__RECEIVING_WAIT:
	MCALL C5_WAIT
	LDD TEMP,Z+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE _DRV_UART_H_PROC__RECEIVING_NOT_WAIT_DATA
	LDS TEMP_L,_C5_UPTIME+0x04
	SUB TEMP_L,YL
	LDS TEMP_H,_C5_UPTIME+0x03
	SBC TEMP_H,YH
	LDD TEMP,Z+_DRV_UART_H_FIRST_TIMEOUT+0x00
	LDD ACCUM,Z+_DRV_UART_H_FIRST_TIMEOUT+0x01
	CPI TEMP,0x00
	BRNE _DRV_UART_H_PROC__RECEIVING_NOT_ZERO_TIMEOUT
	CPI ACCUM,0x00
	BREQ _DRV_UART_H_PROC__RECEIVING_WAIT
_DRV_UART_H_PROC__RECEIVING_NOT_ZERO_TIMEOUT:
	CP TEMP,TEMP_H
	BRCS _DRV_UART_H_PROC__RECEIVING_TIMEOUT
	BRNE _DRV_UART_H_PROC__RECEIVING_WAIT
	CP ACCUM,TEMP_L
	BRCS _DRV_UART_H_PROC__RECEIVING_WAIT
	RJMP _DRV_UART_H_PROC__RECEIVING_TIMEOUT
_DRV_UART_H_PROC__RECEIVING_NOT_WAIT_DATA:
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_H_PROC__RECEIVING_WAIT
	RJMP _DRV_UART_H_PROC__RECEIVING_DONE
_DRV_UART_H_PROC__RECEIVING_TIMEOUT:
	LDI TEMP,DRV_UART_ST_FAIL
	STD Z+_DRV_UART_H_STATE,TEMP
_DRV_UART_H_PROC__RECEIVING_DONE:
_DRV_UART_H_PROC__RX_SKIP:
	;Запрещаю UART
	LDS TEMP,UCSRB
	ANDI TEMP,low(~((1<<TXCIE0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<RXEN0)))
	STS UCSRB,TEMP

	;Результат
	LDD TEMP_H,Z+_DRV_UART_H_STATE
	LDD TEMP_L,Z+_DRV_UART_H_BUFFER_POS
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
	LDD TEMP_L,Z+_DRV_UART_H_BUFFER_POS
	;Считываем размер передаваемых данных
	LDD TEMP_H,Z+_DRV_UART_H_TX_LEN
	;Проверяем на первый байт
	CPI TEMP_L,0x00
	BRNE _DRV_UART_H_TX_EVENT__SKIP_DR
	;Порт направления в HI
	LDD ACCUM,Z+_DRV_UART_H_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
_DRV_UART_H_TX_EVENT__SKIP_DR:
	;Если все данные переданы то завершаем работу TX
	CP TEMP_H,TEMP_L
	BREQ _DRV_UART_H_TX_EVENT__DONE
	;Инкрементируем и записываепм позицию в буфере
	MOV TEMP,TEMP_L
	INC TEMP
	STD Z+_DRV_UART_H_BUFFER_POS,TEMP
	;Вычисляю смещение на нужный байт
	PUSH_Z
	LDD TEMP,Z+_DRV_UART_H_SRC_ADDR+0x00
	LDD ZL,Z+_DRV_UART_H_SRC_ADDR+0x01
	MOV ZH,TEMP
	MOV ACCUM,ZH
	ANDI ZH,0x7f
	ADD ZL,TEMP_L
	CLR TEMP
	ADC ZH,TEMP
	;Считываем с буфера байт и передаем
	SBRC ACCUM,0x07
	RJMP PC+0x03
	LD TEMP,Z
	RJMP PC+02
	LPM TEMP,Z+
	STS UDR,TEMP
	POP_Z
	RJMP _DRV_UART_H_TX_EVENT__END
_DRV_UART_H_TX_EVENT__DONE:
	;Отключаем передатчик
	LDS TEMP,UCSRB
	ANDI TEMP,~((1<<TXCIE0)|(1<<TXEN0))
	STS UCSRB,TEMP
	;Выставляем TX в низкий уровень
	LDI ACCUM,TXD
	MCALL PORT_SET_LO
	;Проверяем на опцию BREAK
	LDD TEMP,Z+_DRV_UART_H_OPTIONS
	SBRS TEMP,DRV_UART_OPT_BREAK
	RJMP _DRV_UART_H_TX_EVENT__NO_BREAK
	LDI TEMP,DRV_UART_ST_BREAK
	STD Z+_DRV_UART_H_STATE,TEMP
	;Обновляю таймер на отсчет таймаута для посылки BREAK
	LDD ACCUM,Z+_DRV_UART_H_NEXT_TIMEOUT
	MOV TEMP,ACCUM
	LSR TEMP
	ADD ACCUM,TEMP
	LDD TEMP,Z+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_SET_PERIOD	;TODO оптимизировать
	MCALL C5_TIMER_START
	RJMP _DRV_UART_H_TX_EVENT__END
_DRV_UART_H_TX_EVENT__NO_BREAK:
	;Порт направления в LO
	LDD ACCUM,Z+_DRV_UART_H_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
	;Останавливаю таймер
	LDD TEMP,Z+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_STOP
	;Устанавливаю состояние READY
	LDI TEMP,DRV_UART_ST_READY
	STD Z+_DRV_UART_H_STATE,TEMP

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

	MCALL C5_RAM_OFFSET
	;Считываю позицию в буфере
	LDD ACCUM,Z+_DRV_UART_H_BUFFER_POS
	;Проверяю на первый байт
	CPI ACCUM,0x00
	BRNE _DRV_UART_H_RX_EVENT__NOT_FIRST_BYTE
	LDI TEMP,DRV_UART_ST_RECEIVING
	STD Z+_DRV_UART_H_STATE,TEMP
_DRV_UART_H_RX_EVENT__NOT_FIRST_BYTE:
	;Проверяю на корректное состояние
	LDD TEMP,Z+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE _DRV_UART_H_RX_EVENT__END
	;Проверяю на максимальный размер
	LDD TEMP,Z+_DRV_UART_H_RX_LEN
	CP ACCUM,TEMP
	BRCS _DRV_UART_H_RX_EVENT__NO_OVERSIZE
	LDI TEMP,DRV_UART_ST_FAIL
	STD Z+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_RX_EVENT__TIMER_UPDATE
_DRV_UART_H_RX_EVENT__NO_OVERSIZE:
	LDS TEMP,UCSR0A
	SBRS TEMP,FE0
	RJMP _DRV_UART_H_RX_EVENT__NO_BREAK
	LDI TEMP,DRV_UART_ST_READY
	STD Z+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_RX_EVENT__END
_DRV_UART_H_RX_EVENT__NO_BREAK:
	;Записываю байт
	LDD YH,Z+_DRV_UART_H_DST_ADDR+0x00
	LDD YL,Z+_DRV_UART_H_DST_ADDR+0x01
	ADD YL,ACCUM
	CLR TEMP
	ADC YH,TEMP
	LDS TEMP,UDR
	ST Y,TEMP
	;Инкрементирую и записываю позицию в буфере
	INC ACCUM
	STD Z+_DRV_UART_H_BUFFER_POS,ACCUM
_DRV_UART_H_RX_EVENT__TIMER_UPDATE:
	;Обновляю таймер на отсчет таймаута ожидания следоующего байта
	LDD TEMP,Z+_DRV_UART_H_TIMER_ID
	LDD ACCUM,Z+_DRV_UART_H_NEXT_TIMEOUT
	MCALL C5_TIMER_SET_PERIOD		;TODO оптимизировать
	MCALL C5_TIMER_START
_DRV_UART_H_RX_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_UART_H_TIMER_EVENT:
;--------------------------------------------------------
;Событие от таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	LDD TEMP,Z+_DRV_UART_H_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE _DRV_UART_H_TIMER_EVENT__NOT_RECEIVING
	LDD TEMP,Z+_DRV_UART_H_BUFFER_POS
	LDI ACCUM,DRV_UART_ST_FAIL
	CPI TEMP,0x00
	BREQ PC+0x02
	LDI TEMP,DRV_UART_ST_READY
	STD Z+_DRV_UART_H_STATE,TEMP
	RJMP _DRV_UART_H_TIMER_EVENT__END

_DRV_UART_H_TIMER_EVENT__NOT_RECEIVING:
	CPI TEMP,DRV_UART_ST_BREAK
	BRNE _DRV_UART_H_TIMER_EVENT__END
	;Останавливаю таймер
	LDD TEMP,Z+_DRV_UART_H_TIMER_ID
	MCALL C5_TIMER_STOP
	;Порт направления в LO
	LDD ACCUM,Z+_DRV_UART_H_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
	;Устанавливаю состояние READY
	LDI TEMP,DRV_UART_ST_READY
	STD Z+_DRV_UART_H_STATE,TEMP

_DRV_UART_H_TIMER_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Z
	RET

