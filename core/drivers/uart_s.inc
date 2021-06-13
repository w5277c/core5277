;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021  w5277c@gmail.com			Начало
;25.01.2021  w5277c@gmail.com			Добавлена поддержка eint(не проверено)
;26.02.2021  w5277c@gmail.com			Исправляю баги(в процессе)
;26.02.2021  w5277c@gmail.com			Починил прием с использованием EINT
;18.03.2021  w5277c@gmail.com			Багфикс
;17.04.2021  w5277c@gmail.com			Поддержка таймера C
;-----------------------------------------------------------------------------------------------------------------------
;TODO поддержка MAGIC?
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_UART_S
.else
.set DEF__C5_DRV_UART_S = 1
.message "Included driver software UART v0.4"

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
.include	"./core/timer_start.inc"
.include	"./core/timer_start_at.inc"
.include	"./core/timer_stop.inc"
.include	"./core/eint_enable.inc"
.include	"./core/eint_disable.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_S_RX_PORT					= DRV_UART_RX_PORT	;1B - Порт для приема данных
	.EQU	_DRV_UART_S_TX_PORT					= DRV_UART_TX_PORT	;1B - Порт для передачи данных
	.EQU	_DRV_UART_S_DR_PORT					= DRV_UART_DR_PORT	;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_S_LED_PORT					= 0x03			;1B - Порт индикации
	.EQU	_DRV_UART_S_STATE						= 0x04			;1B - Статус драйвера
	.EQU	_DRV_UART_S_TIMER_ID					= 0x05			;1B - ИД программного таймера
	.EQU	_DRV_UART_S_PCINT_DRV_ID			= 0x06			;1B - ИД драйвера PCINT или прерывание C5_IR_INTn(с выключенным 0x07 битом,
																			;у ИД драйвера всегда старший бит C5_PROCID_OPT_DRV - включен)
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

	;---BAUDRATE_IDS---
.IF TIMERS_SPEED == TIMERS_SPEED_50NS
	.EQU	DRV_UART_S_BAUDRATE_4800			= 0x04
	.EQU	DRV_UART_S_BAUDRATE_9600			= 0x02
	.EQU	DRV_UART_S_BAUDRATE_19200			= 0x01			;Только совместно с C5_IR_INTn
.ENDIF
.IF TIMERS_SPEED == TIMERS_SPEED_25NS
	.EQU	DRV_UART_S_BAUDRATE_4800			= 0x08
	.EQU	DRV_UART_S_BAUDRATE_9600			= 0x04
	.EQU	DRV_UART_S_BAUDRATE_19200			= 0x02			;Только совместно с C5_IR_INTn
.ENDIF

;--------------------------------------------------------
DRV_UART_S_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт RX, TEMP_L-порт TX
;TEMP_EH-порт направления передача/прием
;(0xff-не используется),
;TEMP_EL-порт индикации передачи
;(0xff-не используется)
;ACCUM-ид таймера(0x83 - используем таймер C)
;FLAGS-опции
;YH-скорость передачи данных
;YL-ид драйвера PCINT или
;прерывание C5_IR_INTn(с выключенным 0x07 битом)
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH YH
	PUSH TEMP

	PUSH ACCUM
	LDI ACCUM,_DRV_UART_S_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;Записываю ID таймера
	STD Z+_DRV_UART_S_TIMER_ID,ACCUM

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
	MCALL PORT_SET_HI
	STD Z+_DRV_UART_S_TX_PORT,ACCUM
	;Записываю период таймера
	STD Z+_DRV_UART_S_TIMER_PERIOD,YH
	;Расчитываю NEXT TIMEOUT
	LSL YH
	LSL YH
	LSL YH
	STD Z+_DRV_UART_S_NEXT_TIMEOUT,YH

	POP TEMP
	POP YH
	POP ACCUM
	POP_Z
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y-src адрес, X-dst адрес,
;TEMP_EH-TX длина, TEMP_EL-RX максимальная длина
;TEMP_H,TEMP_L-таймаут ожидания данных(2ms)
;OUT: TEMP_H-результат(состояние UART'а),
;TEMP_L-длина полученных данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL

	MCALL C5_RAM_OFFSET

	;Запоминаем таймаут ожидания первого байта
	STD Z+_DRV_UART_S_FIRST_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_UART_S_FIRST_TIMEOUT+0x01,TEMP_L
	;Запминаем длину TX и RX
	STD Z+_DRV_UART_S_TX_LEN,TEMP_EH
	STD Z+_DRV_UART_S_RX_LEN,TEMP_EL

	LDI TEMP,DRV_UART_ST_READY
	STD Z+_DRV_UART_S_STATE,TEMP

	;Инициализирую таймер
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	LDI TEMP_H,high(_DRV_UART_S_EVENT)
	LDI TEMP_L,low(_DRV_UART_S_EVENT)
	LDD ACCUM,Z+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_SET

	MCALL C5_SOFT_UART_MODE_SET
;;;	MCALL C5_DISPATCHER_LOCK

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_S_PROC__TX_SKIP
	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_UART_S_SRC_ADDR+0x00,YH
	STD Z+_DRV_UART_S_SRC_ADDR+0x01,YL

	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_UART_S_BUFFER_POS,TEMP
	STD Z+_DRV_UART_S_BIT_NUM,TEMP
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_UART_S_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI

	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Z+_DRV_UART_S_STATE,TEMP
	;Порт направления в HI
	LDD ACCUM,Z+_DRV_UART_S_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	;Запускаю таймер
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	MCALL C5_TIMER_START
	;Жду окончания передачи
_DRV_UART_S_PROC__SENDING_WAIT:
	LDD TEMP,Z+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_S_PROC__SENDING_WAIT

	;Порт направления в LO
	LDD ACCUM,Z+_DRV_UART_S_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_UART_S_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
_DRV_UART_S_PROC__TX_SKIP:

	;Сброс позиции в буфере и номера бита
	CLR TEMP
	STD Z+_DRV_UART_S_BUFFER_POS,TEMP
	STD Z+_DRV_UART_S_BIT_NUM,TEMP

	;Блок приема
	CPI TEMP_EL,0x00
	BRNE PC+0x02
	RJMP _DRV_UART_S_PROC__RX_SKIP

	;Запоминаем DST
	STD Z+_DRV_UART_S_DST_ADDR+0x00,XH
	STD Z+_DRV_UART_S_DST_ADDR+0x01,XL

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Z+_DRV_UART_S_STATE,TEMP
	;Включаю прерывание для RX порта
	LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
	SBRC TEMP,0x07
	RJMP PC+0x08
	MOV TEMP_H,TEMP
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	LDI YH,high(_DRV_UART_S_EINT_EVENT)
	LDI YL,low(_DRV_UART_S_EINT_EVENT)
	MCALL C5_EINT_ENABLE
	RJMP PC+0x07
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	LDI YH,high(_DRV_UART_S_PCINT_EVENT)
	LDI YL,low(_DRV_UART_S_PCINT_EVENT)
	MOV FLAGS,PID
	MCALL C5_EXEC

;;;	MCALL C5_DISPATCHER_UNLOCK

	;TODO проверить на возможность использования C5_WAIT при ожидании начала данных
	;Запоминаем текущий UPTIME
	LDS YH,_C5_UPTIME+0x03
	LDS YL,_C5_UPTIME+0x04
_DRV_UART_S_PROC__RECEIVING_WAIT:
	LDD TEMP,Z+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_S_PROC__RECEIVING_WAIT
	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE _DRV_UART_S_PROC__DONE
		;Проверка FIRST TIMEOUT
		LDD TEMP,Z+_DRV_UART_S_FIRST_TIMEOUT+0x00
		LDD ACCUM,Z+_DRV_UART_S_FIRST_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE PC+0x03
		CPI ACCUM,0x00
		BREQ _DRV_UART_S_PROC__RECEIVING_WAIT
		LDS TEMP_H,_C5_UPTIME+0x03
		LDS TEMP_L,_C5_UPTIME+0x04
		SUB TEMP_L,YL
		SBC TEMP_H,YH
		CP TEMP_H,TEMP
		BRCS _DRV_UART_S_PROC__RECEIVING_WAIT
		BRNE PC+0x03
		CP TEMP_L,ACCUM
		BRCS _DRV_UART_S_PROC__RECEIVING_WAIT
		;Достигли таймаута
		LDI TEMP,DRV_UART_ST_FAIL
		STD Z+_DRV_UART_S_STATE,TEMP
		;Останавливаю таймер
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		;Выключаю прерывание для RX порта
		LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
		SBRC TEMP,0x07
		RJMP PC+0x05
		MOV TEMP_H,TEMP
		MCALL C5_EINT_DISABLE
		RJMP PC+0x05
		LDD ACCUM,Z+_DRV_UART_S_RX_PORT
		LDI FLAGS,0xff
		MCALL C5_EXEC
_DRV_UART_S_PROC__RX_SKIP:
;;;	MCALL C5_DISPATCHER_UNLOCK
_DRV_UART_S_PROC__DONE:
	MCALL C5_SOFT_UART_MODE_RESET

	;Результат
	LDD TEMP_H,Z+_DRV_UART_S_STATE
	LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS

	POP TEMP_EL
	POP TEMP_EH
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

	MCALL C5_RAM_OFFSET
	LDD TEMP,Z+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_SENDING
	BRNE PC+0x04
	MCALL _DRV_UART_S_SEND_EVENT
	RJMP __DRV_UART_S_EVENT__END
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE PC+0x03
	MCALL _DRV_UART_S_RECV_EVENT

__DRV_UART_S_EVENT__END:
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_UART_S_SEND_EVENT:
;--------------------------------------------------------
;Оптравка 1 бита
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

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
		SBRS ACCUM,0x07
		LD TEMP,Z
		SBRC ACCUM,0x07
		LPM TEMP,Z
		POP_Z
		STD Z+_DRV_UART_S_ACCUM,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__NEXT
__DRV_UART_S_SEND_EVENT__NO_START:
	CPI TEMP_H,0x0ff
	BRNE __DRV_UART_S_SEND_EVENT__NOT_FINISH
		;Останавливаю таймер
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		LDI TEMP,DRV_UART_ST_READY
		STD Z+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__END
__DRV_UART_S_SEND_EVENT__NOT_FINISH:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_S_SEND_EVENT__STOP
__DRV_UART_S_SEND_EVENT__DATA:
		LDD ACCUM,Z+_DRV_UART_S_TX_PORT
		LDD TEMP,Z+_DRV_UART_S_ACCUM
		LSR TEMP
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
		STD Z+_DRV_UART_S_BUFFER_POS,TEMP_L
		LDD TEMP,Z+_DRV_UART_S_TX_LEN
		CP TEMP_L,TEMP
		BRNE __DRV_UART_S_SEND_EVENT__END
		;Делаю метку для выхода
		LDI TEMP,0xff
		STD Z+_DRV_UART_S_BIT_NUM,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__END
__DRV_UART_S_SEND_EVENT__NEXT:
	INC TEMP_H
	STD Z+_DRV_UART_S_BIT_NUM,TEMP_H
__DRV_UART_S_SEND_EVENT__END:

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_UART_S_RECV_EVENT:
;--------------------------------------------------------
;Прием 1 бита
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM
	PUSH_Y
	PUSH FLAGS

	;Читаем состояние порта и пишем его в флаг T
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	MCALL PORT_GET
	ROR FLAGS

	;Читаем номер бита
	LDD TEMP_H,Z+_DRV_UART_S_BIT_NUM

	CPI TEMP_H,0x00
	BRNE __DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT
		;Отработал таймер после STOP, т.е. прошло времени на несколько бит и START не был обнаружен
		;Останавливаю таймер
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		MCALL C5_TIMER_STOP
		;Прием данных завершен
		LDI TEMP,DRV_UART_ST_READY
		LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
		LDD ACCUM,Z+_DRV_UART_S_RX_LEN
		CP TEMP_L,ACCUM
		BRNE PC+0x02
		LDI TEMP,DRV_UART_ST_OVERFLOW
		STD Z+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_S_RECV_EVENT__STOP
__DRV_UART_S_RECV_EVENT__DATA:
		LDD TEMP,Z+_DRV_UART_S_ACCUM
		LSL FLAGS
		ROR TEMP
		STD Z+_DRV_UART_S_ACCUM,TEMP
		CPI TEMP_H,0x08
		BRNE __DRV_UART_S_RECV_EVENT__NEXT_BIT

		;На бит раньше включаю прерывание для RX порта (так как позже могу не успеть)
		LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
		SBRC TEMP,0x07
		RJMP PC+0x0a
		PUSH TEMP_H
		MOV TEMP_H,TEMP
		LDI TEMP_L,C5_ISC_FALLING_EDGE
		LDI YH,high(_DRV_UART_S_EINT_EVENT)
		LDI YL,low(_DRV_UART_S_EINT_EVENT)
		MCALL C5_EINT_ENABLE
		POP TEMP_H
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
		LDD ACCUM,Z+_DRV_UART_S_RX_PORT
		LDI YH,high(_DRV_UART_S_PCINT_EVENT)
		LDI YL,low(_DRV_UART_S_PCINT_EVENT)
		MOV FLAGS,PID
		MCALL C5_EXEC
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
__DRV_UART_S_RECV_EVENT__STOP:
		;STOP (TX=HI)
		SBRS FLAGS,0x07
		RJMP __DRV_UART_S_RECV_EVENT__ERROR
		LDD TEMP_L,Z+_DRV_UART_S_BUFFER_POS
		LDD ACCUM,Z+_DRV_UART_S_RX_LEN
		CP TEMP_L,ACCUM
		BREQ __DRV_UART_S_RECV_EVENT__NEXT_BYTE
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
;		LDD TEMP,Z+_DRV_UART_S_RX_LEN
;		CP TEMP_L,TEMP
;		BRNE __DRV_UART_S_RECV_EVENT__NEXT_BYTE
;		;Останавливаю таймер
;		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
;		MCALL C5_TIMER_STOP
;		LDI TEMP,DRV_UART_ST_READY
;		STD Z+_DRV_UART_S_STATE,TEMP
;		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NEXT_BYTE:
		;Сбрасываю номер бита
		CLR TEMP_H
		STD Z+_DRV_UART_S_BIT_NUM,TEMP_H
		;Задаю новый период таймеру для отсчета таймаута
		LDD TEMP,Z+_DRV_UART_S_TIMER_ID
		LDD ACCUM,Z+_DRV_UART_S_NEXT_TIMEOUT
		MCALL C5_TIMER_START_AT
		RJMP __DRV_UART_S_RECV_EVENT__END
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

	POP FLAGS
	POP_Y
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_UART_S_PCINT_EVENT:
;--------------------------------------------------------
;Обработчик смены состояния пина
;Первый бит FLAGS содержит состояние пина
;--------------------------------------------------------
	SBRC FLAGS,0x00
	RET

_DRV_UART_S_EINT_EVENT:
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	MCALL C5_RAM_OFFSET
	LDD TEMP,Z+_DRV_UART_S_BIT_NUM
	CPI TEMP,0x00
	BRNE __DRV_UART_S_PCINT_EVENT__END

	;Выключаю прерывание для RX порта
	LDD TEMP,Z+_DRV_UART_S_PCINT_DRV_ID
	SBRC TEMP,0x07
	RJMP __DRV_UART_S_PCINT_EVENT__PCINT
	PUSH TEMP_H
	MOV TEMP_H,TEMP
	MCALL C5_EINT_DISABLE
	POP TEMP_H
	LDI TEMP,(TIMERS_SPEED/4)*3
	MCALL __DRV_UART_S_PCINT_EVENT__CORRECTOR
	RJMP __DRV_UART_S_PCINT_EVENT__END
__DRV_UART_S_PCINT_EVENT__PCINT:
;	PUSH TEMP
	LDI TEMP,(TIMERS_SPEED/3)*1
	MCALL __DRV_UART_S_PCINT_EVENT__CORRECTOR
;	POP TEMP
	LDD ACCUM,Z+_DRV_UART_S_RX_PORT
	LDI FLAGS,0xff
	MCALL C5_EXEC

__DRV_UART_S_PCINT_EVENT__END:
	POP FLAGS
	POP ACCUM
	POP TEMP
	POP_Z
	RET
__DRV_UART_S_PCINT_EVENT__CORRECTOR:
	LDS FLAGS,SREG
	CLI
	_CORRE5277_TIMERA_CORRECTOR TEMP
	;Задаю новый период таймеру для отсчета битов
	LDD TEMP,Z+_DRV_UART_S_TIMER_ID
	LDD ACCUM,Z+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_START_AT
	LDI TEMP,0x01
	STD Z+_DRV_UART_S_BIT_NUM,TEMP
	LDI TEMP,DRV_UART_ST_RECEIVING
	STD Z+_DRV_UART_S_STATE,TEMP
	STS SREG,FLAGS
	RET
.endif