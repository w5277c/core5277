;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.06.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер инфракрасного приемника и передачтика, стандарт NEC(возможно адаптирую для SIRC)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_IR
.else
.set DEF__C5_DRV_IR = 1
.message "Included driver IR v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_IR_RX_PORT								= 0x00	;1B - Порт для приема данных
	.EQU	_DRV_IR_TX_PORT								= 0x01	;1B - Порт для передачи данных
	.EQU	_DRV_IR_LED_PORT								= 0x02	;1B - Порт индикации
	.EQU	_DRV_IR_TIMER_ID								= 0x03	;1B - ИД программного таймера
	.EQU	_DRV_IR_INT_ID									= 0x04	;1B - ИД прерывания C5_IR_INTn
	.EQU	_DRV_IR_STATE									= 0x05	;1B - Состояние драйвера
	.EQU	_DRV_IR_ACCUM									= 0x06	;1B - Аккумулятор
	;---
	.EQU	_DRV_IR_TIMEOUT								= 0x07	;2B - Таймаут ожидания данных
	.EQU	_DRV_IR_SRC_ADDR								= 0x09	;2B - Адрес блока данных для передачи
	.EQU	_DRV_IR_DST_ADDR								= 0x0b	;2B - Адрес блока данных для приема
	;---
	.EQU	_DRV_IR_BIT_POS								= 0x0d	;1B - Позиция текущего бита для передачи/приема
	.EQU	_DRV_IR_TX_LEN									= 0x0e	;1B - Количество бит для передачи
	.EQU	_DRV_IR_RX_LEN									= 0x0f	;1B - Максимальное количество бит для приема
	;---
	
	.EQU	_DRV_IR_PERIOD_PREAMBLE						= 0x78	;120 = 6мс, половина преамбулы
	.EQU	_DRV_IR_PERIOD_LO								= 0x0a	;10 = 0.5мс, низкий уровень
	.EQU	_DRV_IR_PERIOD_HI0							= 0x0a	;10 = 0.5мс, высокйи уровень = 0
	.EQU	_DRV_IR_PERIOD_HI1							= 0x1e	;30 = 1.5мс, высокйи уровень = 1

	;---STATES_IDS---
	.EQU	_DRV_IR_READY									= 0x00;
	.EQU	_DRV_IR_TX_PREAMBLE_LO						= 0x02;
	.EQU	_DRV_IR_TX_PREAMBLE_HI						= 0x03;
	.EQU	_DRV_IR_TX_DATA_LO							= 0x04;
	.EQU	_DRV_IR_TX_DATA_HI							= 0x05;
	.EQU	_DRV_IR_TX_DONE								= 0x06;
	.EQU	_DRV_IR_WAIT_DATA								= 0x07;
	.EQU	_DRV_IR_RX_PREAMBLE							= 0x08;
	.EQU	_DRV_IR_RX_DATA								= 0x09;
	.EQU	_DRV_IR_RX_FAIL								= 0x0a;
	.EQU	_DRV_IR_RX_OVERFLOW							= 0x0b;
	
	;---RESULT_IDS---
	.EQU	DRV_IR_RESULT_OK								= 0x00;
	.EQU	DRV_IR_RESULT_REPEAT							= 0x01;
	.EQU	DRV_IR_RESULT_ERROR							= 0x80;

	.EQU	_DRV_IR_RAM_SIZE								= 0x10			;16B - необходимо выделить

;--------------------------------------------------------
DRV_IR_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт RX, TEMP_L-порт TX
;TEMP_EH-ид прерывания C5_IR_INTn
;TEMP_EL-порт индикации передачи (0xff-не используется)
;ACCUM-ид таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM

	PUSH ACCUM
	LDI ACCUM,_DRV_IR_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;RX
	STD Z+_DRV_IR_RX_PORT,TEMP_H
	MOV ACCUM,TEMP_H
	CPI ACCUM,0xff
	BREQ PC+0x05
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	;TX
	STD Z+_DRV_IR_TX_PORT,TEMP_L
	MOV ACCUM,TEMP_L
	CPI ACCUM,0xff
	BREQ PC+0x05
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	;LED
	STD Z+_DRV_IR_LED_PORT,TEMP_EL
	MOV ACCUM,TEMP_EL
	CPI ACCUM,0xff
	BREQ PC+0x05
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	POP TEMP
	;Записываю ID таймера
	STD Z+_DRV_IR_TIMER_ID,TEMP
	;Инициализирую таймер
	LDI TEMP_H,high(_DRV_IR_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_IR_TIMER_EVENT)
	MCALL C5_TIMER_SET

	;Устанавливаю состояние READY
	LDI ACCUM,_DRV_IR_READY
	STD Z+_DRV_IR_STATE,ACCUM

	POP ACCUM
	POP_Z
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y-src адрес, X-dst адрес,
;TEMP_EH-TX длина в битах, TEMP_EL-RX максимальная длина
;в битах, TEMP_H,TEMP_L-таймаут ожидания данных(2ms)
;OUT: TEMP_H-DRV_IR_RESULT_x, TEMP_L-длина полученных
;данных
;--------------------------------------------------------
	PUSH_Z

	MCALL C5_RAM_OFFSET

	;Запоминаем таймаут ожидания первого байта
	STD Z+_DRV_IR_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_IR_TIMEOUT+0x01,TEMP_L
	;Запминаем длину TX и RX
	STD Z+_DRV_IR_TX_LEN,TEMP_EH
	STD Z+_DRV_IR_RX_LEN,TEMP_EL

	CPI TEMP_EH,0x00
	BREQ _DRV_IR_PROC__TX_SKIP
	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_IR_SRC_ADDR+0x00,YH
	STD Z+_DRV_IR_SRC_ADDR+0x01,YL

	;Запуск передачи
	LDI TEMP,_DRV_IR_TX_PREAMBLE_LO
	STD Z+_DRV_IR_STATE,TEMP
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI

	LDD ACCUM,Z+_DRV_IR_TX_PORT
	MCALL PORT_SET_LO
	;Запуск передачи
	LDD TEMP,Z+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_PERIOD_PREAMBLE
	MCALL C5_TIMER_START_AT
	;Жду окончания передачи
_DRV_IR_PROC__SENDING_WAIT:
	LDD TEMP,Z+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_TX_DONE
	BRNE _DRV_IR_PROC__SENDING_WAIT
	
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
_DRV_IR_PROC__TX_SKIP:




	;TODO

;--------------------------------------------------------
_DRV_IR_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик прерывания от таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	LDD TEMP,Z+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_TX_PREAMBLE_LO
	BRNE _DRV_IR_TIMER_EVENT__NO_TX_PREAMBLE_LO
	;Перамбула, низкий уровень, включаем выслкий
	LDD ACCUM,Z+_DRV_IR_TX_PORT
	MCALL PORT_SET_HI
	LDI TEMP,_DRV_IR_TX_PREAMBLE_HI
	STD Z+_DRV_IR_STATE,TEMP
	STD Z+_DRV_IR_BIT_POS,C0x00
	RJMP _DRV_IR_TIMER_EVENT__END
_DRV_IR_TIMER_EVENT__NO_TX_PREAMBLE_LO:
	CPI TEMP,_DRV_IR_TX_PREAMBLE_HI
	BREQ Pc+0x03
	CPI TEMP,_DRV_IR_TX_DATA_HI
	BRNE _DRV_IR_TIMER_EVENT__NO_TX_PREAMBLE_HI
	;Перамбула или данные, высокий уровень, включаем данные, низкий
	LDD ACCUM,Z+_DRV_IR_TX_PORT
	MCALL PORT_SET_LO
	LDD TEMP,Z+_DRV_IR_BIT_POS
	ANDI TEMP,0xf8
	BRNE _DRV_IR_TIMER_EVENT__SKIP_ACCUM_LOADING
	LSR TEMP
	LSR TEMP
	LSR TEMP
	LDD YH,Z+_DRV_IR_SRC_ADDR+0x00
	LDD YL,Z+_DRV_IR_SRC_ADDR+0x01
	ADD YL,TEMP
	ADC YH,C0x00
	LD TEMP,Y
	STD Z+_DRV_IR_ACCUM,TEMP
_DRV_IR_TIMER_EVENT__SKIP_ACCUM_LOADING:
	;Устанавливаем длительность для данных, низкий уровень
	LDD TEMP,Z+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_PERIOD_HI0
	MCALL C5_TIMER_START_AT
	LDI TEMP,_DRV_IR_TX_DATA_LO
	STD Z+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_TIMER_EVENT__END
_DRV_IR_TIMER_EVENT__NO_TX_PREAMBLE_HI:
	CPI TEMP,_DRV_IR_TX_DATA_LO
	BRNE _DRV_IR_TIMER_EVENT__NO_TX_DATA_LO
	;Данные, низкий уровень включаем выский разной длительностью
	LDD ACCUM,Z+_DRV_IR_TX_PORT
	MCALL PORT_SET_HI
	LDD TEMP,Z+_DRV_IR_BIT_POS
	LDD ACCUM,Z+_DRV_IR_TX_LEN
	CP ACCUM,TEMP
	BREQ _DRV_IR_TIMER_EVENT__TX_DONE
	INC TEMP
	STD Z+_DRV_IR_BIT_POS,TEMP
	LDD TEMP,Z+_DRV_IR_ACCUM
	LSR TEMP
	STD Z+_DRV_IR_ACCUM,TEMP
	BRCC PC+0x05
	LDD TEMP,Z+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_PERIOD_HI1
	MCALL C5_TIMER_START_AT
	LDI TEMP,_DRV_IR_TX_DATA_HI
	STD Z+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_TIMER_EVENT__END
_DRV_IR_TIMER_EVENT__TX_DONE:
	LDD TEMP,Z+_DRV_IR_TIMER_ID
	MCALL C5_TIMER_STOP
	RJMP _DRV_IR_TIMER_EVENT__END

_DRV_IR_TIMER_EVENT__NO_TX_DATA_LO:
	;TODO RX





_DRV_IR_TIMER_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Y
	POP_Z
	RET

;TODO INT EVENT

.endif