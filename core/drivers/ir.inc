;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.06.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер инфракрасного приемника и передачтика
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_IR
.else
.set DEF__C5_DRV_IR = 1
.message "Included driver IR v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_IR_RX_PORT						= 0x00			;1B - Порт для приема данных
	.EQU	_DRV_IR_TX_PORT						= 0x01			;1B - Порт для передачи данных
	.EQU	_DRV_IR_LED_PORT						= 0x02			;1B - Порт индикации
	.EQU	_DRV_IR_STATE							= 0x03			;1B - Статус драйвера
	.EQU	_DRV_IR_TIMER_ID						= 0x04			;1B - ИД программного таймера
	.EQU	_DRV_IR_INT_ID							= 0x05			;1B - ИД прерывания C5_IR_INTn
	.EQU	_DRV_IR_STATE							= 0x06			;1B - Состояние драйвера
	;---
	.EQU	_DRV_IR_SRC_ADDR						= 0x07			;2B - Адрес блока данных для передачи
	.EQU	_DRV_IR_DST_ADDR						= 0x09			;2B - Адрес блока данных для приема
	;---
	.EQU	_DRV_IR_BITNUM							= 0x0b			;1B - Номер бита для примема/передачи
	;---
	;---STATES_IDS---
	.EQU	DRV_IR_READY							= 0x00;
	.EQU	DRV_IR_SENDING							= 0x01;
	.EQU	DRV_IR_WAIT_DATA						= 0x02;
	.EQU	DRV_IR_RECEIVING						= 0x03;
	.EQU	DRV_IR_FAIL								= 0x04;
	.EQU	DRV_IR_OVERFLOW						= 0x05;
	
	.EQU	_DRV_IR_RAM_SIZE						= 0x0c			;12B - необходимо выделить

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
	LDI TEMP_H,high(_DRV_IR_EVENT)
	LDI TEMP_L,low(_DRV_IR_EVENT)
	LDD ACCUM,0x64;todo
	MCALL C5_TIMER_SET

	;Устанавливаю состояние READY
	LDI ACCUM,DRV_IR_READY
	STD Z+_DRV_IR_STATE,ACCUM

	POP ACCUM
	POP_Z
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y-src адрес, X-dst адрес,
;TEMP_EH-TX длина, TEMP_EL-RX максимальная длина
;TEMP_H,TEMP_L-таймаут ожидания данных(2ms)
;OUT: TEMP_H-результат, TEMP_L-длина полученных данных
;--------------------------------------------------------
	PUSH_Z

	MCALL C5_RAM_OFFSET

	;Запоминаем таймаут ожидания первого байта
	STD Z+_DRV_IR_FIRST_TIMEOUT+0x00,TEMP_H
	STD Z+_DRV_IR_FIRST_TIMEOUT+0x01,TEMP_L
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
	LDI TEMP,DRV_IR_SENDING
	STD Z+_DRV_IR_STATE,TEMP
	;Сброс позиции в буфере
	STD Z+_DRV_IR_BIT_NUM,C0x00
	;Включаю индикатор передачи
	LDD ACCUM,Z+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI

	;Запуск передачи
	LDD TEMP,Z+_DRV_IR_TIMER_ID	;TODO ???
	MCALL C5_TIMER_START
	;Жду окончания передачи
_DRV_IR_PROC__SENDING_WAIT:
	LDD TEMP,Z+_DRV_IR_STATE
	CPI TEMP,DRV_IR_READY
	BRNE _DRV_IR_PROC__SENDING_WAIT
	
	;Выключаю индикатор
	LDD ACCUM,Z+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
_DRV_IR_PROC__TX_SKIP:




	;TODO


.endif