;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include "./core/drivers/_uart.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"
.include	"./core/wait.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_set_period.inc"
.include	"./core/timer_start.inc"
.include	"./core/timer_stop.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_S_RX_PORT					= 0x00			;1B - Порт для приема данных
	.EQU	_DRV_UART_S_TX_PORT					= 0x01			;1B - Порт для передачи данных
	.EQU	_DRV_UART_S_DR_PORT					= 0x02			;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_S_LED_PORT					= 0x03			;1B - Порт индикации
	.EQU	_DRV_UART_S_STATE						= 0x04			;1B - Статус драйвера
	.EQU	_DRV_UART_S_TIMER_ID					= 0x05			;1B - ИД программного таймера
	.EQU	_DRV_UART_S_PCINT_DRV_ID			= 0x06			;1B - ИД драйвера PCINT
	.EQU	_DRV_UART_S_OPTIONS					= 0x07			;1B - опции
	;---
	.EQU	_DRV_UART_S_SRC_ADDR					= 0x08			;2B - адрес блока данных для передачи
	.EQU	_DRV_UART_S_DST_ADDR					= 0x0a			;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_UART_S_BUFFER_POS				= 0x0c			;1B - Позиция в буфере
	.EQU	_DRV_UART_S_TX_LEN					= 0x0d			;1B - Длина данных для передачи
	.EQU	_DRV_UART_S_RX_LEN					= 0x0e			;1B - Максимальный размер данных для приема
	.EQU	_DRV_UART_S_BIT_NUM					= 0x0f			;1B - Номер текущего бита
	.EQU	_DRV_UART_S_ACCUM						= 0x10			;1B - аккумулятор (байт подготовленный для передачи/приема)
		;---
	.EQU	_DRV_UART_S_TIMER_PERIOD			= 0x11			;1B - период работы таймера для приема/передачи битов
	.EQU	_DRV_UART_S_FIRST_TIMEOUT			= 0x12			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_UART_S_NEXT_TIMEOUT			= 0x14			;1B - Таймаут ожидания последующего байта(0.050ms)

	.EQU	_DRV_UART_S_RAM_SIZE					= 0x15			;21 байт необходимо выделить


;--------------------------------------------------------
DRV_UART_S_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H - порт RX, TEMP_L - порт TX
;TEMP_EH - порт направления передача/прием
;(0xff-не используется),
;TEMP_EL - порт индикации передачи
;(0xff-не используется)
;ACCUM - ид таймера
;FLAGS - опции
;YH - скорость передачи данных
;YL - ид драйвера PCINT
;--------------------------------------------------------
	PUSH_Z
;	PUSH TEMP_H
;	PUSH TEMP_L
;	PUSH TEMP
	PUSH ACCUM
	PUSH YH

	PUSH ACCUM
	LDI ACCUM,_DRV_UART_S_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;TODO расчитать период работы таймера и проинициализировать его

	;Записываю ID таймера
	STD Z+_DRV_UART_S_TIMER_ID,ACCUM
	;Инициализирую таймер
	PUSH TEMP_H
	PUSH TEMP_L
	MOV TEMP,ACCUM
	LDI TEMP_H,high(_DRV_UART_S_EVENT)
	LDI TEMP_L,low(_DRV_UART_S_EVENT)
	MOV ACCUM,YH
	MCALL C5_TIMER_SET
	POP TEMP_L
	POP TEMP_H

	;Записываю ID PCINT драйвера
	STD Z+_DRV_UART_S_PCINT_DRV_ID,YL
	;Устанавливаю состояние READY
	LDI ACCUM,DRV_UART_ST_READY
	STD Z+_DRV_UART_S_STATE,ACCUM
	;Записываю опции
	STD Z+_DRV_UART_S_OPTIONS,FLAGS
	;Инициализирую порт направления данных
	MOV ACCUM,TEMP_EH
	CPI ACCUM,0xff
	BREQ _DRV_UART_S_INIT__NO_DR_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
_DRV_UART_S_INIT__NO_DR_PORT:
	STD Z+_DRV_UART_S_DR_PORT,ACCUM
	;Инициализирую порт индикации
	MOV ACCUM,TEMP_EL
	CPI ACCUM,0xff
	BREQ _DRV_UART_S_INIT__NO_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
_DRV_UART_S_INIT__NO_LED_PORT:
	STD Z+_DRV_UART_S_LED_PORT,ACCUM
	;Инициализирую порт RX
	MOV ACCUM,TEMP_H
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	STD Z+_DRV_UART_S_RX_PORT,ACCUM
	;Инициализирую порт TX
	MOV ACCUM,TEMP_L
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO	;TODO в UART_H используется SET_HI
	STD Z+_DRV_UART_S_TX_PORT,ACCUM
	;Записываю период таймера
	STD Z+_DRV_UART_S_TIMER_PERIOD,YH
	;Расчитываю NEXT TIMEOUT										;Для 4800 таймаут составит 2.6ms(~13бит), для 115200 составит 0.150ms(~17бит)
	LSR YH																;Таймаут будет базироваться на _C5_MAIN_TIMER_CNTR(отсчет каждые 0.050ms)
	LSR YH
	INC YH
	STD Z+_DRV_UART_S_NEXT_TIMEOUT,YH

	POP YH
	POP ACCUM
;	POP TEMP
;	POP TEMP_L
;	POP TEMP_H
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
	STD Z+_DRV_UART_S_FIRST_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_UART_S_FIRST_TIMEOUT+0x01,TEMP_L
	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес рабоыт с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_UART_S_SRC_ADDR+0x00,YH
	STD Z+_DRV_UART_S_SRC_ADDR+0x01,YL
	;Запоминаем DST
	STD Z+_DRV_UART_S_DST_ADDR+0x00,XH
	STD Z+_DRV_UART_S_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Z+_DRV_UART_S_TX_LEN,TEMP_EH
	STD Z+_DRV_UART_S_RX_LEN,TEMP_EL

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_S_PROC__TX_SKIP
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_UART_S_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_UART_S_BUFFER_POS,TEMP
	STD Z+_DRV_UART_S_BIT_NUM,TEMP
	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Z+_DRV_UART_S_STATE,TEMP
	;Запускаю таймер
	;TODO
	;Порт направления в HI
	LDD ACCUM,Z+_DRV_UART_S_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	;Отправляю первый бит
	MCALL _DRV_UART_S_EVENT
	;Жду окончания передачи
_DRV_UART_S_PROC__SENDING_WAIT:
	MCALL C5_WAIT
	LDD TEMP,Z+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_S_PROC__SENDING_WAIT
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_UART_S_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
_DRV_UART_S_PROC__TX_SKIP:

	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_UART_S_BUFFER_POS,TEMP

	;Блок приема
	CPI TEMP_EL,0x00
	BREQ _DRV_UART_S_PROC__RX_SKIP

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Z+_DRV_UART_S_STATE,TEMP
	;Включаю PCINT для RX порта
	LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	ORI ACCUM,0x80
	LDI YH,high(_DRV_UART_S_PCINT_EVENT)
	LDI YL,low(_DRV_UART_S_PCINT_EVENT)
	MOV FLAGS,PID
	MCALL C5_EXEC
	;Запоминаем метку времени для отсчета таймаута ожидания данных
	LDS YH,_C5_UPTIME+0x03
	LDS YL,_C5_UPTIME+0x04
_DRV_UART_S_PROC__RECEIVING_WAIT:
	MCALL C5_WAIT
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_S_PROC__RECEIVING_WAIT
	RJMP _DRV_UART_S_PROC__RECEIVING_DONE
_DRV_UART_S_PROC__RECEIVING_TIMEOUT:
	LDI TEMP,DRV_UART_ST_FAIL
	STD Z+_DRV_UART_S_STATE,TEMP
_DRV_UART_S_PROC__RECEIVING_DONE:
_DRV_UART_S_PROC__RX_SKIP:
	;Выключаю PCINT для RX порта
	LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	LDI FLAGS,0xff
	MCALL C5_EXEC
	;Останавливаю таймер
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	MCALL C5_TIMER_STOP

	;Результат
	LDD TEMP_H,Z+_DRV_UART_S_STATE
	LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_UART_S_EVENT:
;--------------------------------------------------------
;Событие от таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	LDD TEMP,Z+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_SENDING
	BREQ _DRV_UART_S_SEND_EVENT
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE PC+0x02
	RJMP _DRV_UART_S_RECV_EVENT

	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE __DRV_UART_S_EVENT__END
		;Проверка FIRST TIMEOUT
		LDS TEMP_L,_C5_UPTIME+0x04
		SUB TEMP_L,YL
		LDS TEMP_H,_C5_UPTIME+0x03
		SBC TEMP_H,YH
		LDD TEMP,Z+_DRV_UART_S_FIRST_TIMEOUT+0x00
		LDD ACCUM,Z+_DRV_UART_S_FIRST_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE __DRV_UART_S_EVENT__NOT_ZERO_TIMEOUT
		CPI ACCUM,0x00
		BREQ __DRV_UART_S_EVENT__END
__DRV_UART_S_EVENT__NOT_ZERO_TIMEOUT:
	CP TEMP,TEMP_H
	BRCS __DRV_UART_S_EVENT__TIMEOUT
	BRNE __DRV_UART_S_EVENT__END
	CP ACCUM,TEMP_L
	BRCS __DRV_UART_S_EVENT__END
__DRV_UART_S_EVENT__TIMEOUT:
	LDI TEMP,DRV_UART_ST_FAIL
	STD Z+_DRV_UART_S_STATE,TEMP
__DRV_UART_S_EVENT__END:

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_UART_S_SEND_EVENT:
;--------------------------------------------------------
;Оптравка 1 бита
;--------------------------------------------------------
	LDD TEMP_H,Z+_DRV_UART_S_BIT_NUM
	CPI TEMP_H,0x00
	BRNE __DRV_UART_S_SEND_EVENT__NO_START
		;START (TX=LO)
		LDD ACCUM,Z+_DRV_UART_S_TX_PORT
		MCALL PORT_SET_LO
		;Вычисляю смещение
		LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
		PUSH_Z
		LDD TEMP,Z+_DRV_UART_S_SRC_ADDR+0x00
		LDD ZL,Z+_DRV_UART_S_SRC_ADDR+0x01
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
		POP_Z
		STD Z+_DRV_UART_S_ACCUM,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__NEXT
__DRV_UART_S_SEND_EVENT__NO_START:
	CPI TEMP_H,0x08
	BREQ __DRV_UART_S_SEND_EVENT__STOP
__DRV_UART_S_SEND_EVENT__DATA:
		LDD ACCUM,Z+_DRV_UART_S_RX_PORT
		LDD TEMP,Z+_DRV_UART_S_ACCUM
		LSL TEMP
		STD Z+_DRV_UART_S_ACCUM,TEMP
		MCALL PORT_SET
		RJMP __DRV_UART_S_SEND_EVENT__NEXT
__DRV_UART_S_SEND_EVENT__STOP:
		;STOP (TX=HI)
		LDD ACCUM,Z+_DRV_UART_S_TX_PORT
		MCALL PORT_SET_HI
		LDI TEMP,0x00
		STD Z+_DRV_UART_S_BIT_NUM,TEMP
		LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
		INC TEMP_L
		STD Z+_DRV_UART_S_BUFFER_POS,TEMP
		LDD TEMP,Z+_DRV_UART_S_TX_LEN
		CP TEMP_L,TEMP
		BRNE __DRV_UART_S_SEND_EVENT__END
		;Останавливаю таймер
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		;Порт направления в LO
		LDD ACCUM,Z+_DRV_UART_S_DR_PORT
		CPI ACCUM,0xff
		BREQ PC+0x03
		MCALL PORT_SET_LO
		LDI TEMP,DRV_UART_ST_READY
		STD Z+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__END
__DRV_UART_S_SEND_EVENT__NEXT:
	INC TEMP_H
	STD Z+_DRV_UART_S_BIT_NUM,TEMP_H
__DRV_UART_S_SEND_EVENT__END:

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_UART_S_RECV_EVENT:
;--------------------------------------------------------
;Прием 1 бита
;--------------------------------------------------------
	;Читаем состояние порта и пишем его в флаг T
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	MCALL PORT_GET
	LDS TEMP,SREG
	BLD TEMP,0x00
	;Читаем номер бита
	LDD TEMP_H,Z+_DRV_UART_S_BIT_NUM
	CPI TEMP_H,0x09
	BRNE __DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT
		;Отработал таймер после STOP, т.е. прошло времени на несколько бит и START не был обнаружен
		;Выключаю PCINT для RX порта
		LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
		LDI FLAGS,0xff
		MCALL C5_EXEC
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		;Прием данных завершен
		LDI TEMP,DRV_UART_ST_READY
		STD Z+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT:
	CPI TEMP_H,0x00
	BRNE __DRV_UART_S_RECV_EVENT__NO_START
		;START (TX=LO)
		;Выключаю PCINT для RX порта
		LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
		LDI FLAGS,0xff
		MCALL C5_EXEC
		;Ощибка, если не START
		BRTS __DRV_UART_S_RECV_EVENT__ERROR
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
__DRV_UART_S_RECV_EVENT__NO_START:
	CPI TEMP_H,0x08
	BREQ __DRV_UART_S_RECV_EVENT__STOP
__DRV_UART_S_RECV_EVENT__DATA:
		LDD TEMP,Z+_DRV_UART_S_ACCUM
		BST TEMP_L,0x00
		STS SREG,TEMP_L
		LSR TEMP
		STD Z+_DRV_UART_S_ACCUM,TEMP
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
__DRV_UART_S_RECV_EVENT__STOP:
		;STOP (TX=HI)
		BRTC __DRV_UART_S_RECV_EVENT__ERROR
		LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
		LDD ACCUM,Z+_DRV_UART_S_ACCUM
		PUSH_Z
		LDD TEMP,Z+_DRV_UART_S_DST_ADDR+0x00
		LDD ZL,Z+_DRV_UART_S_DST_ADDR+0x01
		MOV ZH,TEMP
		ADD ZL,TEMP_L
		CLR TEMP
		ADC ZH,TEMP
		ST Z,ACCUM
		POP_Z
		INC TEMP_L
		STD Z+_DRV_UART_S_BUFFER_POS,TEMP_L
		LDD TEMP,Z+_DRV_UART_S_RX_LEN
		CP TEMP_L,TEMP
		BRNE __DRV_UART_S_RECV_EVENT__NEXT_BYTE
		;Останавливаю таймер
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		LDI TEMP,DRV_UART_ST_READY
		STD Z+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NEXT_BYTE:
		;Включаю PCINT для RX порта
		LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
		LDD ACCUM,Z+_DRV_UART_S_RX_PORT
		ORI ACCUM,0x80
		LDI YH,high(_DRV_UART_S_PCINT_EVENT)
		LDI YL,low(_DRV_UART_S_PCINT_EVENT)
		MOV FLAGS,PID
		MCALL C5_EXEC
		;Задаю новый период таймеру для отсчета таймаута
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		LDD ACCUM,Z+_DRV_UART_S_NEXT_TIMEOUT
		MCALL C5_TIMER_SET_PERIOD
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
__DRV_UART_S_RECV_EVENT__ERROR:
	;Останавливаю таймер
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	MCALL C5_TIMER_STOP
	LDI TEMP,DRV_UART_ST_FAIL
	STD Z+_DRV_UART_S_STATE,TEMP
	RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NEXT_BIT:
	INC TEMP_H
	STD Z+_DRV_UART_S_BIT_NUM,TEMP_H
__DRV_UART_S_RECV_EVENT__END:
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_UART_S_PCINT_EVENT:
;--------------------------------------------------------
;Обработчик смены состояния пина
;Первый бит FLAGS содержит состояние пина
;--------------------------------------------------------
	SBRC FLAGS,0x00
	RET

	PUSH TEMP
	PUSH ACCUM

	CLR TEMP
	STD Z+_DRV_UART_S_BIT_NUM,TEMP
	;Задаю новый период таймеру для отсчета половины бита
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	LDD ACCUM,Z+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_SET_PERIOD

	POP ACCUM
	POP TEMP
	RET
