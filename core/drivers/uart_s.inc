;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021	w5277c@gmail.com			Начало
;25.01.2021	w5277c@gmail.com			Добавлена поддержка eint(не проверено)
;26.02.2021	w5277c@gmail.com			Исправляю баги(в процессе)
;26.02.2021	w5277c@gmail.com			Починил прием с использованием EINT
;18.03.2021	w5277c@gmail.com			Багфикс
;17.04.2021	w5277c@gmail.com			Поддержка таймера C
;14.06.2021	w5277c@gmail.com			Локализация C5_SOFT_UART_MODE_SET/RESET
;23.07.2021	w5277c@gmail.com			Z->Y, необходимо тестирование
;02.08.2021	w5277c@gmail.com			Поддержка BREAK
;18.08.2021	w5277c@gmail.com			Вынесена проверка на значение порта 0xff в процедуры порта, багфиксинг
;24.08.2021	w5277c@gmail.com			Октет таймаут-2 бита, поддержка TIMER_C, переход на /io/reg_. Не тестировано.
;-----------------------------------------------------------------------------------------------------------------------
;Программный UART, с поддержкой BREAK, TIMER_C, PCINT и C5_IR_INTn
;*некоторый BAUDRATE недоступен при использовании TIMER_C(в зависимости от CORE_FREQ)
;*BREAK не совместим с TIMER_C(переполняется 8 битный тамер)
;*рекомендуется использовать C5_IR_INTn, вместо драйверов PCINT(медленные)
;-----------------------------------------------------------------------------------------------------------------------
;TODO поддержка MAGIC?
;TODO привести PCINT_H и PCINT_S к одному виду
;TODO блокировать диспетчер при DRV_UART_ST_RECEIVING
;TODO учитывать доступность BAUDRATE для разных CORE_FREQ
;TODO стр.602, если данных принято RX_LEN, то включается ST_OVERFLOW(временно отключил), негде хранить признак
;переполнения.
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_UART_S
.else
.set DEF__C5_DRV_UART_S = 1
.message "Included driver software UART v0.7"

	.include "./core/drivers/_uart.inc"
	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./io/port_offsets.inc"
	.include	"./io/reg_bit_hi.inc"
	.include	"./io/reg_bit_lo.inc"
	.include	"./io/reg_bit_get.inc"
	.include	"./io/reg_bit_set.inc"
	.include	"./core/wait.inc"
	.include	"./core/timer_set.inc"
	.include	"./core/timer_start.inc"
	.include	"./core/timer_start_at.inc"
	.include	"./core/timer_stop.inc"
	.include	"./core/eint_enable.inc"
	.include	"./core/eint_disable.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_S_RX_PORT					= 0x00			;1B - сдвоенный порт для приема данных
	.EQU	_DRV_UART_S_RX_IA						= 0x01			;1B - DDR адрес для приема данных
	.EQU	_DRV_UART_S_RX_BN						= 0x02			;1B - число пина для приема данных
	.EQU	_DRV_UART_S_TX_PA						= 0x03			;1B - PORT адрес для передачи данных
	.EQU	_DRV_UART_S_TX_BN						= 0x04			;1B - число пина для передачи данных
	.EQU	_DRV_UART_S_DR_PA						= 0x05			;1B - PORT адрес для задания направления прием/передача
	.EQU	_DRV_UART_S_DR_BN						= 0x06			;1B - число пина для задания направления прием/передача
	.EQU	_DRV_UART_S_LED_PA					= 0x07			;1B - PORT адрес, для индикации
	.EQU	_DRV_UART_S_LED_BN					= 0x08			;1B - число пина для индикации
	.EQU	_DRV_UART_S_STATE						= 0x09			;1B - Статус драйвера
	.EQU	_DRV_UART_S_TIMER_ID					= 0x0a			;1B - ИД программного таймера(0x83-TIMER_C)
	.EQU	_DRV_UART_S_PCINT_DRV_ID			= 0x0b			;1B - ИД драйвера PCINT или прерывание C5_IR_INTn(с выключенным 0x07 битом,
																			;у ИД драйвера всегда старший бит C5_PROCID_OPT_DRV - включен)
	.EQU	_DRV_UART_S_OPTIONS					= 0x0c			;1B - опции
	;---
	.EQU	_DRV_UART_S_SRC_ADDR					= 0x0d			;2B - адрес блока данных для передачи
	.EQU	_DRV_UART_S_DST_ADDR					= 0x0f			;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_UART_S_BUFFER_POS				= 0x11			;1B - Позиция в буфере
	.EQU	_DRV_UART_S_TX_LEN					= 0x12			;1B - Длина данных для передачи
	.EQU	_DRV_UART_S_RX_LEN					= 0x13			;1B - Максимальный размер данных для приема
	.EQU	_DRV_UART_S_BIT_NUM					= 0x14			;1B - Номер текущего бита
	.EQU	_DRV_UART_S_ACCUM						= 0x15			;1B - аккумулятор (байт подготовленный для передачи/приема)
		;---
	.EQU	_DRV_UART_S_TIMER_PERIOD			= 0x16			;1B - период работы таймера для приема/передачи битов
	.EQU	_DRV_UART_S_FIRST_TIMEOUT			= 0x17			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_UART_S_NEXT_TIMEOUT			= 0x19			;1B - Таймаут ожидания последующего байта(0.050ms)

	.EQU	_DRV_UART_S_RAM_SIZE					= 0x1a			;Необходимо выделить

	;---BAUDRATE_IDS---
.IF TIMERS_SPEED == TIMERS_SPEED_50US
	.EQU	DRV_UART_S_BAUDRATE_4800			= 0x04
	.EQU	DRV_UART_S_BAUDRATE_9600			= 0x02
	.EQU	DRV_UART_S_BAUDRATE_19200			= 0x01			;Только совместно с C5_IR_INTn
.ENDIF
.IF TIMERS_SPEED == TIMERS_SPEED_25US
	.EQU	DRV_UART_S_BAUDRATE_4800			= 0x08
	.EQU	DRV_UART_S_BAUDRATE_9600			= 0x04
	.EQU	DRV_UART_S_BAUDRATE_19200			= 0x02			;Только совместно с C5_IR_INTn
.ENDIF

;При использовании таймера C доступый BAUDRATE зависит от частоты CPU, минимальное кол-во тактов-256(TODO нужно проверить, достаточно ли?)
.IF TIMER_C_ENABLE == 1
	.IF CORE_FREQ < 5
		.EQU	DRV_UART_S_BAUDRATE_C9600		= CORE_FREQ*125000/4800-1
	.ENDIF
	.IF CORE_FREQ > 2 & CORE_FREQ < 10
		.EQU	DRV_UART_S_BAUDRATE_C9600		= CORE_FREQ*125000/9600-1
	.ENDIF
	.IF CORE_FREQ > 3 & CORE_FREQ < 15
		.EQU	DRV_UART_S_BAUDRATE_C14400		= CORE_FREQ*125000/14400-1
	.ENDIF
	.IF CORE_FREQ > 4 & CORE_FREQ < 20
		.EQU	DRV_UART_S_BAUDRATE_C19200		= CORE_FREQ*125000/19200-1
	.ENDIF
	.IF CORE_FREQ > 7
		.EQU	DRV_UART_S_BAUDRATE_C28800		= CORE_FREQ*125000/28800-1
	.ENDIF
	.IF CORE_FREQ > 9
		.EQU	DRV_UART_S_BAUDRATE_C38400		= CORE_FREQ*125000/38400-1
	.ENDIF
	.IF CORE_FREQ > 14
		.EQU	DRV_UART_S_BAUDRATE_C57600		= CORE_FREQ*125000/57600-1
	.ENDIF
	.IF CORE_FREQ > 19
		.EQU	DRV_UART_S_BAUDRATE_C76800		= CORE_FREQ*125000/76800-1
	.ENDIF
.ENDIF
;--------------------------------------------------------
DRV_UART_S_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт RX, TEMP_L-порт TX
;YH-порт направления передача/прием
;YL-порт индикации передачи
;TEMP_EH-ид таймера(0x83 - используем таймер C)
;FLAGS-опции
;TEMP_EL-скорость передачи данных
;ACCUM-ид драйвера PCINT или
;прерывание C5_IR_INTn(с выключенным 0x07 битом)
;Для всех портов 0xff-порт не используется.
;--------------------------------------------------------
	PUSH ACCUM
	MOVW ZL,YL
	LDI ACCUM,_DRV_UART_S_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;Записываю ID таймера
	STD Y+_DRV_UART_S_TIMER_ID,TEMP_EH

	;Записываю ID PCINT драйвера
	STD Y+_DRV_UART_S_PCINT_DRV_ID,ACCUM
	;Устанавливаю состояние READY
	LDI ACCUM,DRV_UART_ST_READY
	STD Y+_DRV_UART_S_STATE,ACCUM
	;Записываю опции
	STD Y+_DRV_UART_S_OPTIONS,FLAGS

	;Инициализирую порт RX
	PUSH TEMP_L
	STD Y+_DRV_UART_S_RX_PORT,TEMP_H
	MOV ACCUM,TEMP_H
	MCALL PORT_OFFSETS
	STD Y+_DRV_UART_S_RX_IA,TEMP
	MOV TEMP,TEMP_L
	MCALL REG_BIT_LO
	MOV TEMP,TEMP_H
	MCALL REG_BIT_HI
	STD Y+_DRV_UART_S_RX_BN,ACCUM
	POP TEMP_L
	;Инициализирую порт TX
	MOV ACCUM,TEMP_L
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	MCALL REG_BIT_HI
	STD Y+_DRV_UART_S_TX_PA,TEMP_H
	STD Y+_DRV_UART_S_TX_BN,ACCUM
	;Инициализирую порт направления данных
	MOV ACCUM,ZH
	CPI ACCUM,0xff
	BREQ PC+0x0a
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_UART_S_DR_PA,TEMP_H
	STD Y+_DRV_UART_S_DR_BN,ACCUM
	;Инициализирую порт индикации
	MOV ACCUM,ZL
	CPI ACCUM,0xff
	BREQ PC+0x0a
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_UART_S_LED_PA,TEMP_H
	STD Y+_DRV_UART_S_LED_BN,ACCUM

	;Инициализирую таймер
.IF TIMER_C_ENABLE == 1
	CPI TEMP_EH,0x83
	BREQ PC+0x08
.ENDIF
	MOV TEMP,TEMP_EH
	LDI TEMP_H,high(_DRV_UART_S_EVENT)
	LDI TEMP_L,low(_DRV_UART_S_EVENT)
	MOV ACCUM,TEMP_EL
	MCALL C5_TIMER_SET
.IF TIMER_C_ENABLE == 1
	RJMP PC+0x08
	LDI_X _DRV_UART_S_EVENT
	MCALL C5_TIMER_C_SET_VECTOR
	MOV TEMP,TEMP_EL
	;TODO преобразовать в период для TIMER_C
	MCALL C5_TIMER_C_SET_PERIOD
.ENDIF
	;Записываю период таймера
	STD Y+_DRV_UART_S_TIMER_PERIOD,TEMP_EL
	;Расчитываю NEXT TIMEOUT
	LSL TEMP_EL
	STD Y+_DRV_UART_S_NEXT_TIMEOUT,TEMP_EL

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
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
	STD Y+_DRV_UART_S_FIRST_TIMEOUT+0x00,TEMP_H
	STD Y+_DRV_UART_S_FIRST_TIMEOUT+0x01,TEMP_L
	;Запминаем длину TX и RX
	STD Y+_DRV_UART_S_TX_LEN,TEMP_EH
	STD Y+_DRV_UART_S_RX_LEN,TEMP_EL

	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_S_STATE,TEMP

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_S_PROC__TX_SKIP
	;Запоминаем SRC
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_UART_S_SRC_ADDR+0x00,ZH
	STD Y+_DRV_UART_S_SRC_ADDR+0x01,ZL

	;Сброс позиции в буфере
	STD Y+_DRV_UART_S_BUFFER_POS,C0x00
	STD Y+_DRV_UART_S_BIT_NUM,C0x00
	;Включаю индикатор передачи
	LDD ACCUM,Y+_DRV_UART_S_LED_BN
	CPI ACCUM,0x0ff
	BREQ PC+0x04
	LDD TEMP,Y+_DRV_UART_S_LED_PA
	MCALL REG_BIT_HI
	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Y+_DRV_UART_S_STATE,TEMP
	;Порт направления в HI
	LDD ACCUM,Y+_DRV_UART_S_DR_BN
	CPI ACCUM,0x0ff
	BREQ PC+0x04
	LDD TEMP,Y+_DRV_UART_S_DR_PA
	MCALL REG_BIT_HI
	;Запускаем таймер
	LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 1
	CPI TEMP,0x83
	BREQ PC+0x07
.ENDIF
	MCALL C5_SOFT_UART_MODE_SET
	LDD ACCUM,Y+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_START_AT
.IF TIMER_C_ENABLE == 1
	RJMP PC+0x04
	LDD TEMP,Y+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_C_START_AT
.ENDIF
	;Жду окончания передачи
_DRV_UART_S_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_S_PROC__SENDING_WAIT

	MCALL C5_SOFT_UART_MODE_RESET
	;Порт направления в LO
	LDD ACCUM,Y+_DRV_UART_S_DR_BN
	CPI ACCUM,0x0ff
	BRNE PC+0x04
	LDD TEMP,Y+_DRV_UART_S_DR_PA
	MCALL REG_BIT_LO
	;Выключаю индикатор
	LDD ACCUM,Y+_DRV_UART_S_LED_BN
	CPI ACCUM,0x0ff
	BREQ PC+0x04
	LDD TEMP,Y+_DRV_UART_S_LED_PA
	MCALL REG_BIT_LO
_DRV_UART_S_PROC__TX_SKIP:

	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_UART_S_BUFFER_POS,C0x00
	STD Y+_DRV_UART_S_BIT_NUM,C0x00

	;Блок приема
	CPI TEMP_EL,0x00
	BRNE PC+0x02
	RJMP _DRV_UART_S_PROC__RX_SKIP

	;Запоминаем DST
	STD Y+_DRV_UART_S_DST_ADDR+0x00,XH
	STD Y+_DRV_UART_S_DST_ADDR+0x01,XL

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Y+_DRV_UART_S_STATE,TEMP
	;Включаю прерывание для RX порта
	LDD TEMP,Y+_DRV_UART_S_PCINT_DRV_ID
	SBRC TEMP,0x07
	RJMP _DRV_UART_S_PROC__RX_PCINT_ENABLE
	MOV TEMP_H,TEMP
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	PUSH_Y
	LDI_Y _DRV_UART_S_EINT_EVENT
	MCALL C5_EINT_ENABLE
	POP_Y
	RJMP _DRV_UART_S_PROC__RX_PCINT_INT_ENABLE_DONE
_DRV_UART_S_PROC__RX_PCINT_ENABLE:
	LDD ACCUM,Y+_DRV_UART_S_RX_PORT
	PUSH_X
	LDI_X _DRV_UART_S_PCINT_EVENT
	MOV FLAGS,PID
	MCALL C5_EXEC
	POP_X
_DRV_UART_S_PROC__RX_PCINT_INT_ENABLE_DONE:

	;Запоминаем метку времени для отсчета таймаута ожидания данных
	CLI
	LDS ZH,_C5_UPTIME+0x03
	LDS ZL,_C5_UPTIME+0x04
	SEI
_DRV_UART_S_PROC__RECEIVING_WAIT:
	LDD TEMP,Y+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_S_PROC__RECEIVING_WAIT
	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE _DRV_UART_S_PROC__DONE
		;Проверка FIRST TIMEOUT
		LDD TEMP,Y+_DRV_UART_S_FIRST_TIMEOUT+0x00
		LDD ACCUM,Y+_DRV_UART_S_FIRST_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE PC+0x03
		CPI ACCUM,0x00
		BREQ _DRV_UART_S_PROC__RECEIVING_WAIT
		CLI
		LDS TEMP_H,_C5_UPTIME+0x03
		LDS TEMP_L,_C5_UPTIME+0x04
		SEI
		SUB TEMP_L,ZL
		SBC TEMP_H,ZH
		CP TEMP_H,TEMP
		BRCS _DRV_UART_S_PROC__RECEIVING_WAIT
		BRNE PC+0x03
		CP TEMP_L,ACCUM
		BRCS _DRV_UART_S_PROC__RECEIVING_WAIT
		;Достигли таймаута
		LDI TEMP,DRV_UART_ST_FAIL
		STD Y+_DRV_UART_S_STATE,TEMP
		;Останавливаем таймер
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 0x01
		CPI TEMP,0x83
		BREQ PC+0x06
.ENDIF
		MCALL C5_SOFT_UART_MODE_RESET
		MCALL C5_TIMER_STOP
.IF TIMER_C_ENABLE == 0x01
		RJMP PC+0x03
		MCALL C5_TIMER_C_STOP
.ENDIF
		MCALL _DRV_UART_S_INT_DISABLE
_DRV_UART_S_PROC__RX_SKIP:
_DRV_UART_S_PROC__DONE:

	;Результат
	LDD TEMP_H,Y+_DRV_UART_S_STATE
	LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS

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
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	LDD TEMP,Y+_DRV_UART_S_STATE
	CPI TEMP,DRV_UART_ST_SENDING
	BRNE PC+0x04
	MCALL _DRV_UART_S_SEND_EVENT
	RJMP __DRV_UART_S_EVENT__END
	CPI TEMP,DRV_UART_ST_RECEIVING
	BRNE PC+0x04
	MCALL _DRV_UART_S_RECV_EVENT
	RJMP __DRV_UART_S_EVENT__END
	CPI TEMP,DRV_UART_ST_BREAK
	BRNE __DRV_UART_S_EVENT__END
		;Останавливаем таймер
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 1
		CPI TEMP,0x83
		BREQ PC+0x04
.ENDIF
		MCALL C5_TIMER_STOP
.IF TIMER_C_ENABLE == 1
		RJMP PC+0x03
		MCALL C5_TIMER_C_STOP
.ENDIF
		;Порт TX в HI
		LDD TEMP,Y+_DRV_UART_S_TX_PA
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		MCALL REG_BIT_HI
		;Порт направления в LO
		LDD ACCUM,Y+_DRV_UART_S_DR_BN
		CPI ACCUM,0xff
		BREQ PC+0x04
		LDD TEMP,Y+_DRV_UART_S_DR_PA
		MCALL REG_BIT_LO
		;Устанавливаю состояние READY
		LDI TEMP,DRV_UART_ST_READY
		STD Y+_DRV_UART_S_STATE,TEMP
__DRV_UART_S_EVENT__END:
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_UART_S_SEND_EVENT:
;--------------------------------------------------------
;Оптравка 1 бита
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	LDD TEMP_H,Y+_DRV_UART_S_BIT_NUM
	CPI TEMP_H,0x00
	BRNE __DRV_UART_S_SEND_EVENT__NO_START
		;START (TX=LO)
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		LDD TEMP,Y+_DRV_UART_S_TX_PA
		MCALL REG_BIT_LO
		;Вычисляю смещение
		LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS

		LDD ZH,Y+_DRV_UART_S_SRC_ADDR+0x00
		LDD ZL,Y+_DRV_UART_S_SRC_ADDR+0x01
		MOV ACCUM,ZH
		ANDI ZH,0x7f
		ADD ZL,TEMP_L
		ADC ZH,C0x00
		;Считываем с буфера байт и передаем
		SBRS ACCUM,0x07
		LD TEMP,Z
		SBRC ACCUM,0x07
		LPM TEMP,Z
		STD Y+_DRV_UART_S_ACCUM,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__NEXT
__DRV_UART_S_SEND_EVENT__NO_START:
	CPI TEMP_H,0x0ff
	BRNE __DRV_UART_S_SEND_EVENT__NOT_FINISH
		;Проверяем на опцию BREAK
		LDD TEMP,Y+_DRV_UART_S_OPTIONS
		SBRS TEMP,DRV_UART_OPT_BREAK
		RJMP __DRV_UART_S_SEND_EVENT__NO_BREAK
		;Выставляем TX в низкий уровень
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		LDD ACCUM,Y+_DRV_UART_S_TX_PA
		MCALL REG_BIT_LO

		LDI TEMP,DRV_UART_ST_BREAK
		STD Y+_DRV_UART_S_STATE,TEMP
		;Обновляю таймер на отсчет таймаута для посылки BREAK
		LDD ACCUM,Y+_DRV_UART_S_NEXT_TIMEOUT
		LSL ACCUM
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
		;TODO не получится с TIMER_C, большая вероятность переполнения ACCUM
.IF TIMER_C_ENABLE == 0x01
		CPI TEMP,0x83
		BREQ PC+0x03
.ENDIF
		MCALL C5_TIMER_START_AT	;TODO оптимизировать
		RJMP __DRV_UART_S_SEND_EVENT__END
__DRV_UART_S_SEND_EVENT__NO_BREAK:
		;Останавливаем таймер
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 1
		CPI TEMP,0x83
		BREQ PC+0x04
.ENDIF
		MCALL C5_TIMER_STOP
.IF TIMER_C_ENABLE == 0x01
		RJMP PC+0x03
		MCALL C5_TIMER_C_STOP
.ENDIF
		;Порт TX в HI
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		LDD TEMP,Y+_DRV_UART_S_TX_PA
		MCALL REG_BIT_HI
		LDI TEMP,DRV_UART_ST_READY
		STD Y+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_SEND_EVENT__END
__DRV_UART_S_SEND_EVENT__NOT_FINISH:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_S_SEND_EVENT__STOP
__DRV_UART_S_SEND_EVENT__DATA:
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		LDD TEMP,Y+_DRV_UART_S_ACCUM
		LSR TEMP
		STD Y+_DRV_UART_S_ACCUM,TEMP
		LDD TEMP,Y+_DRV_UART_S_TX_PA
		MCALL REG_BIT_SET
		RJMP __DRV_UART_S_SEND_EVENT__NEXT
__DRV_UART_S_SEND_EVENT__STOP:
		;STOP (TX=HI)
		LDD ACCUM,Y+_DRV_UART_S_TX_BN
		LDD TEMP,Y+_DRV_UART_S_TX_PA
		MCALL REG_BIT_HI
		STD Y+_DRV_UART_S_BIT_NUM,C0x00
		LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS
		INC TEMP_L
		STD Y+_DRV_UART_S_BUFFER_POS,TEMP_L
		LDD TEMP,Y+_DRV_UART_S_TX_LEN
		CP TEMP_L,TEMP
		BRNE __DRV_UART_S_SEND_EVENT__END
		;Делаю метку для выхода
		LDI TEMP_H,0xfe
__DRV_UART_S_SEND_EVENT__NEXT:
	INC TEMP_H
	STD Y+_DRV_UART_S_BIT_NUM,TEMP_H
__DRV_UART_S_SEND_EVENT__END:

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
	PUSH FLAGS

	;Читаем состояние порта и пишем его в FLAG
	LDD ACCUM,Y+_DRV_UART_S_RX_BN
	LDD TEMP,Y+_DRV_UART_S_RX_IA
	MCALL REG_BIT_GET
	ROR FLAGS

	;Читаем номер бита
	LDD TEMP_H,Y+_DRV_UART_S_BIT_NUM

	CPI TEMP_H,0x00
	BRNE __DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT
		;Отработал таймер после STOP, т.е. прошло времени на несколько бит и START не был обнаружен
		;Останавливаем таймер
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 0x01
		CPI TEMP,0x83
		BREQ PC+0x04
.ENDIF
		MCALL C5_TIMER_STOP
.IF TIMER_C_ENABLE == 0x01
		RJMP PC+0x03
		MCALL C5_TIMER_C_STOP
.ENDIF
		MCALL C5_SOFT_UART_MODE_RESET
		MCALL _DRV_UART_S_INT_DISABLE

		;Прием данных завершен
		LDI TEMP,DRV_UART_ST_READY
;		LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS
;		LDD ACCUM,Y+_DRV_UART_S_RX_LEN
;		CP TEMP_L,ACCUM
;		BRNE PC+0x02
;		LDI TEMP,DRV_UART_ST_OVERFLOW
		STD Y+_DRV_UART_S_STATE,TEMP
		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NO_NEXT_TIMEOUT:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_S_RECV_EVENT__STOP
__DRV_UART_S_RECV_EVENT__DATA:
		LDD TEMP,Y+_DRV_UART_S_ACCUM
		LSL FLAGS
		ROR TEMP
		STD Y+_DRV_UART_S_ACCUM,TEMP
		CPI TEMP_H,0x08
		BREQ PC+0x02
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT

		;На бит раньше включаю прерывание для RX порта (так как позже могу не успеть)
		LDD TEMP,Y+_DRV_UART_S_PCINT_DRV_ID
		SBRC TEMP,0x07
		RJMP _DRV_UART_S_RECV_EVENT__PCINT_ENABLE
		PUSH TEMP_H
		MOV TEMP_H,TEMP
		LDI TEMP_L,C5_ISC_FALLING_EDGE
		PUSH_Y
		LDI_Y _DRV_UART_S_EINT_EVENT
		MCALL C5_EINT_ENABLE
		POP_Y
		POP TEMP_H
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
_DRV_UART_S_RECV_EVENT__PCINT_ENABLE:
		LDD ACCUM,Y+_DRV_UART_S_RX_PORT
		PUSH_X
		LDI_X _DRV_UART_S_PCINT_EVENT
		MOV FLAGS,PID
		MCALL C5_EXEC
		POP_X
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BIT
__DRV_UART_S_RECV_EVENT__STOP:
		;STOP (TX=HI)
		SBRS FLAGS,0x07
		RJMP __DRV_UART_S_RECV_EVENT__ERROR
		LDD TEMP_L,Y+_DRV_UART_S_BUFFER_POS
		LDD ACCUM,Y+_DRV_UART_S_RX_LEN
		CP TEMP_L,ACCUM
		BRNE PC+0x02			;BREQ __DRV_UART_S_RECV_EVENT__NEXT_BYTE
		RJMP __DRV_UART_S_RECV_EVENT__NEXT_BYTE
		LDD ACCUM,Y+_DRV_UART_S_ACCUM
		LDD ZH,Y+_DRV_UART_S_DST_ADDR+0x00
		LDD ZL,Y+_DRV_UART_S_DST_ADDR+0x01
		ADD ZL,TEMP_L
		ADC ZH,C0x00
		ST Z,ACCUM
		INC TEMP_L
		STD Y+_DRV_UART_S_BUFFER_POS,TEMP_L
__DRV_UART_S_RECV_EVENT__NEXT_BYTE:
		;Сбрасываю номер бита
		STD Y+_DRV_UART_S_BIT_NUM,C0x00
		;Задаю новый период таймеру для отсчета таймаута
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 0x01
		CPI TEMP,0x83
		BREQ PC+0x05
.ENDIF
		LDD ACCUM,Y+_DRV_UART_S_NEXT_TIMEOUT
		MCALL C5_TIMER_START_AT
.IF TIMER_C_ENABLE == 0x01
		RJMP PC+0x04
		LDD TEMP,Y+_DRV_UART_S_NEXT_TIMEOUT
		MCALL C5_TIMER_C_START_AT
.ENDIF
		RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__ERROR:
		;Останавливаем таймер
		LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 0x01
		CPI TEMP,0x83
		BREQ PC+0x04
.ENDIF
		MCALL C5_TIMER_STOP
.IF TIMER_C_ENABLE == 0x01
		RJMP PC+0x03
		MCALL C5_TIMER_C_STOP
.ENDIF
	LDI TEMP,DRV_UART_ST_FAIL
	STD Y+_DRV_UART_S_STATE,TEMP
	RJMP __DRV_UART_S_RECV_EVENT__END
__DRV_UART_S_RECV_EVENT__NEXT_BIT:
	INC TEMP_H
	STD Y+_DRV_UART_S_BIT_NUM,TEMP_H
__DRV_UART_S_RECV_EVENT__END:

	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_UART_S_PCINT_EVENT:
;--------------------------------------------------------
;Обработчик смены состояния пина(PCINT/EINT)
;Первый бит FLAGS содержит состояние пина
;--------------------------------------------------------
	SBRC FLAGS,0x00
	RET

_DRV_UART_S_EINT_EVENT:
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_UART_S_BIT_NUM
	CPI TEMP,0x00
	BRNE __DRV_UART_S_PCINT_EVENT__END

	MCALL C5_SOFT_UART_MODE_SET

	MCALL _DRV_UART_S_INT_DISABLE
	LDI TEMP,(TIMERS_SPEED/4)*3
	LDD ACCUM,Y+_DRV_UART_S_PCINT_DRV_ID
	SBRC ACCUM,0x07
	LDI TEMP,(TIMERS_SPEED/3)*1
	LDS FLAGS,SREG
	CLI
	_CORRE5277_TIMERA_CORRECTOR TEMP
	;Задаю новый период таймеру для отсчета битов
	LDD TEMP,Y+_DRV_UART_S_TIMER_ID
.IF TIMER_C_ENABLE == 0x01
	CPI TEMP,0x83
	BREQ PC+0x05
.ENDIF
	LDD ACCUM,Y+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_START_AT
.IF TIMER_C_ENABLE == 0x01
	RJMP PC+0x04
	LDD TEMP,Y+_DRV_UART_S_TIMER_PERIOD
	MCALL C5_TIMER_C_START_AT
.ENDIF
	LDI TEMP,0x01
	STD Y+_DRV_UART_S_BIT_NUM,TEMP
	LDI TEMP,DRV_UART_ST_RECEIVING
	STD Y+_DRV_UART_S_STATE,TEMP
	STS SREG,FLAGS

__DRV_UART_S_PCINT_EVENT__END:
	POP FLAGS
	POP ACCUM
	POP TEMP
	POP_Y
	RET


;--------------------------------------------------------
_DRV_UART_S_INT_DISABLE:
;--------------------------------------------------------
;Отключаю обработку PCINT/EINT
;IN: Y-адрес на выделенную память
;--------------------------------------------------------
	PUSH TEMP
	LDD TEMP,Y+_DRV_UART_S_PCINT_DRV_ID
	SBRC TEMP,0x07
	RJMP _DRV_UART_S_INT_DISABLE__PCINT
	PUSH TEMP_H
	MOV TEMP_H,TEMP
	MCALL C5_EINT_DISABLE
	POP TEMP_H
	RJMP _DRV_UART_S_INT_DISABLE__END
_DRV_UART_S_INT_DISABLE__PCINT:
	PUSH ACCUM
	PUSH FLAGS
	LDD ACCUM,Y+_DRV_UART_S_RX_PORT
	LDI FLAGS,0xff
	MCALL C5_EXEC
	POP FLAGS
	POP ACCUM
_DRV_UART_S_INT_DISABLE__END:
	POP TEMP
	RET
.endif
