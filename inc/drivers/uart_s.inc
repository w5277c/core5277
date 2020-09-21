;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;03.04.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/core/wait.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SUART_DATA_RX_PORT				= 0x00			;1B - Порт, пин для приема
	.EQU	_DRV_SUART_DATA_TX_PORT				= 0x01			;1B - Порт, пин для передачи
	.EQU	_DRV_SUART_DATA_DR_PORT				= 0x02			;1B - Порт, пин для задания направления прием/передача
	.EQU	_DRV_SUART_DATA_LED_PORT			= 0x03			;1B - Порт, пин для индикации работы
	.EQU	_DRV_SUART_DATA_PCI_DRV				= 0x04			;1B - ид драйвера внешних прерываний
	.EQU	_DRV_SUART_DATA_TIMER_ID			= 0x05			;1B - ид таймера
	.EQU	_DRV_SUART_DATA_BAUDRATE			= 0x06			;1B - скорость UART
	.EQU	_DRV_SUART_DATA_FIRST_TIMEOUT		= 0x07			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_SUART_DATA_NEXT_TIMEOUT		= 0x09			;1B - Таймаут ожидания последующего байта(0.050ms)
	.EQU	_DRV_SUART_DATA_SRC_ADDR			= 0x0a			;2B - Адрес блока данных для передачи
	.EQU	_DRV_SUART_DATA_DST_ADDR			= 0x0c			;2B - Адрес блока данных для приема
	.EQU	_DRV_SUART_DATA_TX_LEN				= 0x0e			;1B - длина данных для передачи
	.EQU	_DRV_SUART_DATA_RX_LEN				= 0x0f			;1B - максимальныйх размер данных для приема
	.EQU	_DRV_SUART_DATA_ACCUM				= 0x10			;1B - текущая позиция в буфере
	.EQU	_DRV_SUART_DATA_BUFFER_POS			= 0x11			;1B - текущая позиция в буфере
	.EQU	_DRV_SUART_DATA_BIT_NUM				= 0x12			;1B - текущий номер бита
	.EQU	_DRV_SUART_DATA_STATE				= 0x13			;1B - текущий статус
	;---
	.EQU	_DRV_SUART_DATA_SIZE					= 0x14			;Размер выделяемого блока под данные(20 байт)
	;---
	.EQU	DRV_SUART_BAUDRATE_19200			= 0x01			;Скоростной режим 19200
	.EQU	DRV_SUART_BAUDRATE_9600				= 0x02			;Скоростной режим 9600

	;---STATES_IDS---
	.EQU	DRV_SUART_ST_READY					= 0x00;
	.EQU	DRV_SUART_ST_SENDING					= 0x01;
	.EQU	DRV_SUART_ST_BREAK					= 0x02;
	.EQU	DRV_SUART_ST_WAIT_DATA				= 0x03;
	.EQU	DRV_SUART_ST_RECEIVING				= 0x04;
	.EQU	DRV_SUART_ST_FAIL						= 0x05;


DRV_SUART_INIT:
;--------------------------------------------------------
;Инициализация
;IN: XH - RX порт, XL - TX порт,
;TEMP_H - порт направления передача/прием
;(0xff-не используется),
;TEMP_L - порт индикации передачи
;(0xff-не используется)
;ACCUM - ид таймера
;YH - скорость передачи данных
;YL - ид PCI драйвера
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH YH

	MOV TEMP,DRIVER_ID
	LDI ACCUM,_DRV_SUART_DATA_SIZE
	MCALL CORE5277_RAM_REALLOC

	STD Z+_DRV_SUART_DATA_RX_PORT,XH
	STD Z+_DRV_SUART_DATA_TX_PORT,XL
	STD Z+_DRV_SUART_DATA_DR_PORT,TEMP_H
	STD Z+_DRV_SUART_DATA_LED_PORT,TEMP_L
	STD Z+_DRV_SUART_DATA_TIMER_ID,ACCUM
	STD Z+_DRV_SUART_DATA_BAUDRATE,YH
	STD Z+_DRV_SUART_DATA_PCI_DRV,YL

	;Инициализирую программный таймер
	PUSH TEMP_L
	PUSH TEMP_H
	;Инициализирую порт направления данных
	POP ACCUM
	CPI ACCUM,0xff
	BREQ _DRV_SUART_INIT__NO_DR_PORT
	MCALL CORE5277_PORTS_MODE_OUT
	MCALL CORE5277_PORTS_SET_LO
_DRV_SUART_INIT__NO_DR_PORT:
	;Инициализирую порт индикации
	POP ACCUM
	CPI ACCUM,0xff
	BREQ _DRV_SUART_INIT__NO_LED_PORT
	MCALL CORE5277_PORTS_MODE_OUT
	MCALL CORE5277_PORTS_SET_LO
_DRV_SUART_INIT__NO_LED_PORT:
	;Инициализирую порт RX
	MOV ACCUM,XH
	MCALL CORE5277_PORTS_MODE_IN
	MCALL CORE5277_PORTS_SET_HI
	;Инициализирую порт TX
	MOV ACCUM,XL
	MCALL CORE5277_PORTS_MODE_OUT
	MCALL CORE5277_PORTS_SET_HI
	;Расчитываю NEXT TIMEOUT										;Для 9600 таймаут составит 1.3ms(~13бит), для 19200 составит 0.65ms(~13бит)
	LDI TEMP,0x0d														;Таймаут будет базироваться на _CORE5277_MAIN_TIMER_CNTR(отсчет каждые 0.050ms)
	CPI YH,0x01
	BRNE PC+0x02
	LDI TEMP,0x1a
	STD Z+_DRV_SUART_DATA_NEXT_TIMEOUT,YH

	POP YH
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z

	LDI ACCUM,(1<<CORE5277_MODEBITS_DRV_BSEM)
	MCALL CORE5277_DRIVER_READY
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

	;Получаю адрес выделенной памяти
	MOV TEMP,DRIVER_ID
	MCALL CORE5277_RAM_GET_OFFSET
	;Запоминаем таймаут ожидания первого байта
	STD Z+_DRV_SUART_DATA_FIRST_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_SUART_DATA_FIRST_TIMEOUT+0x01,TEMP_L
	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес рабоыт с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_SUART_DATA_SRC_ADDR+0x00,YH
	STD Z+_DRV_SUART_DATA_SRC_ADDR+0x01,YL
	;Запоминаем DST
	STD Z+_DRV_SUART_DATA_DST_ADDR+0x00,XH
	STD Z+_DRV_SUART_DATA_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Z+_DRV_SUART_DATA_TX_LEN,TEMP_EH
	STD Z+_DRV_SUART_DATA_RX_LEN,TEMP_EL

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_SUART_PROC__TX_SKIP
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_SUART_DATA_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL CORE5277_PORTS_SET_HI
	;Сброс позиции в буфере
	CLR TEMP
	STD Z+_DRV_SUART_DATA_BUFFER_POS,TEMP
	;Запуск передачи
	LDI TEMP,DRV_SUART_ST_SENDING
	STD Z+_DRV_SUART_DATA_STATE,TEMP
	;Порт направления в HI
	LDD ACCUM,Z+_DRV_SUART_DATA_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL CORE5277_PORTS_SET_HI
	;Инициализирую и включаю таймер
	LDI TEMP_H,high(_DRV_SUART_TX_EVENT)
	LDI TEMP_L,low(_DRV_SUART_TX_EVENT)
	LDD TEMP,Z+_DRV_SUART_DATA_TIMER_ID
	LDD ACCUM,Z+_DRV_SUART_DATA_BAUDRATE
	MCALL CORE5277_TIMER_SET
	MCALL CORE5277_TIMER_START
	;Жду окончания передачи
_DRV_SUART_PROC__SENDING_WAIT:
	MCALL CORE5277_WAIT
	LDD TEMP,Z+_DRV_SUART_DATA_STATE
	CPI TEMP,DRV_SUART_ST_READY
	BRNE _DRV_SUART_PROC__SENDING_WAIT
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_SUART_DATA_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL CORE5277_PORTS_SET_LO
_DRV_SUART_PROC__TX_SKIP:

;	;Сброс позиции в буфере
;	CLR TEMP
;	STD Z+_DRV_SUART_DATA_BUFFER_POS,TEMP
;
;	;Блок приема
;	CPI TEMP_EL,0x00
;	BREQ _DRV_SUART_PROC__RX_SKIP
;	;Запуск приема
;	LDI TEMP,DRV_SUART_ST_WAIT_DATA
;	STD Z+_DRV_SUART_DATA_STATE,TEMP
;
;;!!!TODO включаю PCINT и таймер?
;
;	;Запоминаем метку времени для отсчета таймаута ожидания данных
;	LDS YH,_CORE5277_UPTIME+0x03
;	LDS YL,_CORE5277_UPTIME+0x04
;_DRV_SUART_PROC__RECEIVING_WAIT:
;	MCALL CORE5277_WAIT
;	LDD TEMP,Z+_DRV_SUART_DATA_STATE
;	CPI TEMP,DRV_SUART_ST_WAIT_DATA
;	BRNE _DRV_SUART_PROC__RECEIVING_NOT_WAIT_DATA
;	LDS TEMP_L,_CORE5277_UPTIME+0x04
;	SUB TEMP_L,YL
;	LDS TEMP_H,_CORE5277_UPTIME+0x03
;	SBC TEMP_H,YH
;	LDD TEMP,Z+_DRV_SUART_DATA_FIRST_TIMEOUT+0x00
;	LDD ACCUM,Z+_DRV_SUART_DATA_FIRST_TIMEOUT+0x01
;	CPI TEMP,0x00
;	BRNE _DRV_SUART_PROC__RECEIVING_NOT_ZERO_TIMEOUT
;	CPI ACCUM,0x00
;	BREQ _DRV_SUART_PROC__RECEIVING_WAIT
;_DRV_SUART_PROC__RECEIVING_NOT_ZERO_TIMEOUT:
;	CP TEMP,TEMP_H
;	BRCS _DRV_SUART_PROC__RECEIVING_TIMEOUT
;	BRNE _DRV_SUART_PROC__RECEIVING_WAIT
;	CP ACCUM,TEMP_L
;	BRCS _DRV_SUART_PROC__RECEIVING_WAIT
;	RJMP _DRV_SUART_PROC__RECEIVING_TIMEOUT
;_DRV_SUART_PROC__RECEIVING_NOT_WAIT_DATA:
;	CPI TEMP,DRV_SUART_ST_RECEIVING
;	BREQ _DRV_SUART_PROC__RECEIVING_WAIT
;	RJMP _DRV_SUART_PROC__RECEIVING_DONE
;_DRV_SUART_PROC__RECEIVING_TIMEOUT:
;	LDI TEMP,DRV_SUART_ST_FAIL
;	STD Z+_DRV_SUART_DATA_STATE,TEMP
;_DRV_SUART_PROC__RECEIVING_DONE:

;!!! TODO запрещаю PCINT и таймер?

_DRV_SUART_PROC__RX_SKIP:
	;Результат
	LDD TEMP_H,Z+_DRV_SUART_DATA_STATE
	LDD TEMP_L,Z+_DRV_SUART_DATA_BUFFER_POS

	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_SUART_TX_EVENT:
;--------------------------------------------------------
;Обработчик таймера для передачи данных
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	;Получаю адрес выделенной памяти
	MOV TEMP,DRIVER_ID
	MCALL CORE5277_RAM_GET_OFFSET

	;Считываю и инкрементирую номер текущего бита
	LDD TEMP,Z+_DRV_SUART_DATA_BIT_NUM
	INC TEMP
	STD Z+_DRV_SUART_DATA_BIT_NUM,TEMP
	CPI ACCUM,0x01
	BRNE _DRV_SUART_TX_EVENT__NOT_START
;START
	;Считываю позицию в буфере
	LDD ACCUM,Z+_DRV_SUART_DATA_BUFFER_POS
	;Считываем размер передаваемых данных
	LDD TEMP,Z+_DRV_SUART_DATA_TX_LEN
	;Если все данные переданы то завершаем работу TX
	CP TEMP,ACCUM
	BREQ _DRV_SUART_TX_EVENT__TX_DISABLE
	;Вычисляю смещение на нужный байт
	PUSH_Z
	LDD TEMP,Z+_DRV_SUART_DATA_SRC_ADDR+0x00
	LDD ZL,Z+_DRV_SUART_DATA_SRC_ADDR+0x01
	MOV ZH,TEMP
	PUSH ZH
	ANDI ZH,0x7f
	ADD ZL,ACCUM
	CLR TEMP
	ADC ZH,TEMP
	;Считываем с буфера байт и заносим в аккумулятор
	POP ACCUM
	SBRC ACCUM,0x07
	RJMP PC+0x03
	LD TEMP,Z
	RJMP PC+02
	LPM TEMP,Z+
	POP_Z
	STD Z+_DRV_SUART_DATA_ACCUM,TEMP
	;Низкий уровень порта - start
	LDD ACCUM,Z+_DRV_SUART_DATA_TX_PORT
	MCALL CORE5277_PORTS_SET_LO
	RJMP _DRV_SUART_TX_EVENT__END
_DRV_SUART_TX_EVENT__NOT_START:
	CPI ACCUM,0x0a
	BRCC _DRV_SUART_TX_EVENT__STOP
;DATABITS
	;Считываю передаваемый байт
	LDD ACCUM,Z+_DRV_SUART_DATA_ACCUM
	ROL ACCUM
	STD Z+_DRV_SUART_DATA_ACCUM,ACCUM
	LDD ACCUM,Z+_DRV_SUART_DATA_TX_PORT
	MCALL CORE5277_PORTS_SET
	RJMP _DRV_SUART_TX_EVENT__END
_DRV_SUART_TX_EVENT__STOP:
	BRNE _DRV_SUART_TX_EVENT__TX_DISABLE
;STOP
	;Инкрементирую позицию в буфере
	LDD TEMP,Z+_DRV_SUART_DATA_BUFFER_POS
	INC TEMP
	STD Z+_DRV_SUART_DATA_BUFFER_POS,TEMP
	;Сбрасываю счетчик бит
	LDI TEMP,0x00
	STD Z+_DRV_SUART_DATA_BIT_NUM,TEMP
	;Высокий уровень порта - stop
	LDD ACCUM,Z+_DRV_SUART_DATA_TX_PORT
	MCALL CORE5277_PORTS_SET_HI
	RJMP _DRV_SUART_TX_EVENT__END
_DRV_SUART_TX_EVENT__TX_DISABLE:
	;Перевожу драйвер в режим приема
	LDD ACCUM,Z+_DRV_SUART_DATA_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL CORE5277_PORTS_SET_LO
	;Останавливаю таймер
	LDD TEMP,Z+_DRV_SUART_DATA_TIMER_ID
	MCALL CORE5277_TIMER_STOP
	;Выставляю состояние завершенной передачи
	LDI TEMP,DRV_SUART_ST_READY
	STD Z+_DRV_SUART_DATA_STATE,TEMP
	RJMP _DRV_SUART_TX_EVENT__END

_DRV_SUART_TX_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_SUART_RX_EVENT:
;--------------------------------------------------------
;Обработчик таймера для получения данных
;--------------------------------------------------------

