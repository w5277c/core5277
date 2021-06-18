;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.06.2021	w5277c@gmail.com			Начало
;19.06.2021	w5277c@gmail.com			Частичный баг фиксинг
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер инфракрасного приемника и передачтика, стандарт NEC(возможно адаптирую для SIRC)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_IR
.else
.set DEF__C5_DRV_IR = 1
.message "Included driver IR v0.1"

	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./core/eint_enable.inc"
	.include	"./core/eint_disable.inc"
	.include	"./io/port_mode_in.inc"
	.include	"./io/port_mode_out.inc"
	.include	"./io/port_set_hi.inc"
	.include	"./io/port_set_lo.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_IR_RX_PORT								= 0x00	;1B - Порт для приема данных
	.EQU	_DRV_IR_TX_PORT								= 0x01	;1B - Порт для передачи данных
	.EQU	_DRV_IR_LED_PORT								= 0x02	;1B - Порт индикации
	.EQU	_DRV_IR_TIMER_ID								= 0x03	;1B - ИД программного таймера
	.EQU	_DRV_IR_INT_ID									= 0x04	;1B - ИД прерывания C5_IR_INTn
	.EQU	_DRV_IR_STATE									= 0x05	;1B - Состояние драйвера
	.EQU	_DRV_IR_ACCUM									= 0x06	;1B - Аккумулятор
	.EQU	_DRV_IR_TIMESTAMP								= 0x07	;1B - Метка времени (2мс)
	;---
	.EQU	_DRV_IR_TIMEOUT								= 0x08	;2B - Таймаут ожидания данных
	.EQU	_DRV_IR_SRC_ADDR								= 0x0a	;2B - Адрес блока данных для передачи
	.EQU	_DRV_IR_DST_ADDR								= 0x0c	;2B - Адрес блока данных для приема
	;---
	.EQU	_DRV_IR_BIT_POS								= 0x0e	;1B - Позиция текущего бита для передачи/приема
	.EQU	_DRV_IR_TX_LEN									= 0x0f	;1B - Количество бит для передачи
	.EQU	_DRV_IR_RX_LEN									= 0x10	;1B - Максимальное количество бит для приема
	;---

	.EQU	_DRV_IR_TX_PREAMBLE							= 0x83	;6мс, половина преамбулы (2мс период)
	.EQU	_DRV_IR_TX_LO									= 0x0a	;10 = 0.5мс, низкий уровень (0.000050мс период)
	.EQU	_DRV_IR_TX_HI0									= 0x0a	;10 = 0.5мс, высокйи уровень = 0 (0.000050мс период)
	.EQU	_DRV_IR_TX_HI1									= 0x1e	;30 = 1.5мс, высокйи уровень = 1 (0.000050мс период)
	.EQU	_DRV_IR_RX_MIN_PREAMBLE						= 0x82	;4мс, минимальное время преамбулы (2мс период)
	.EQU	_DRV_IR_RX_MAX_PREAMBLE						= 0x88	;16мс, максимальное время преамбулы (2мс период)
	.EQU	_DRV_IR_RX_BIT_TIME_MIN						= 0x0a	;10 = 0.5мс минимальное время передачи бита
	.EQU	_DRV_IR_RX_BIT_TRESHOLD						= 0x1e	;15 = 1.5мс граница между 0 и 1
	.EQU	_DRV_IR_RX_BIT_TIME_MAX						= 0x32	;50 = 2.5мс максимальное время передачи бита

	;---STATES_IDS---
	.EQU	_DRV_IR_TX_PREAMBLE_LO						= 0x00;
	.EQU	_DRV_IR_TX_PREAMBLE_HI						= 0x01;
	.EQU	_DRV_IR_TX_DATA_LO							= 0x02;
	.EQU	_DRV_IR_TX_DATA_HI							= 0x03;
	.EQU	_DRV_IR_RX_PREAMBLE							= 0x04;
	.EQU	_DRV_IR_RX_DATA								= 0x05;
	.EQU	_DRV_IR_TX_DONE								= 0x80;
	.EQU	_DRV_IR_RX_WAIT								= 0x81;
	.EQU	DRV_IR_READY									= 0x82;
	.EQU	DRV_IR_FAIL										= 0x83;
	.EQU	DRV_IR_OVERFLOW								= 0x84;

	.EQU	_DRV_IR_RAM_SIZE								= 0x11			;17B - необходимо выделить

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

	STD Z+_DRV_IR_INT_ID,TEMP_EH

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
	LDI ACCUM,DRV_IR_READY
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
	PUSH_Y
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
	LDI ACCUM,_DRV_IR_TX_PREAMBLE
	MCALL C5_TIMER_START_AT
	;Жду окончания передачи
_DRV_IR_PROC__SENDING_WAIT:
	LDD TEMP,Z+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_TX_DONE
	BRNE _DRV_IR_PROC__SENDING_WAIT
	MOVW YL,ZL
	MCALL _DRV_IR_DONE

_DRV_IR_PROC__TX_SKIP:
	;Сброс позиции в буфере и номера бита
	STD Z+_DRV_IR_BIT_POS,C0x00

	;Блок приема
	CPI TEMP_EL,0x00
	BREQ _DRV_IR_PROC__RX_SKIP

	;Запоминаем DST
	STD Z+_DRV_IR_DST_ADDR+0x00,XH
	STD Z+_DRV_IR_DST_ADDR+0x01,XL

	;Запуск приема
	LDI TEMP,_DRV_IR_RX_WAIT
	STD Z+_DRV_IR_STATE,TEMP

	;Включаю прерывание для RX порта
	LDD TEMP_H,Z+_DRV_IR_INT_ID
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	LDI YH,high(_DRV_IR_EINT_EVENT)
	LDI YL,low(_DRV_IR_EINT_EVENT)
	MCALL C5_EINT_ENABLE

	;Запоминаем текущий UPTIME
	LDS YH,_C5_UPTIME+0x03
	LDS YL,_C5_UPTIME+0x04
_DRV_IR_PROC__RECEIVING_WAIT:
	LDD TEMP,Z+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_RX_WAIT
	BRNE _DRV_IR_PROC__NO_RX_WAIT
		;Проверка FIRST TIMEOUT
		LDD TEMP,Z+_DRV_IR_TIMEOUT+0x00
		LDD ACCUM,Z+_DRV_IR_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE PC+0x03
		CPI ACCUM,0x00
		BREQ _DRV_IR_PROC__RECEIVING_WAIT
		LDS TEMP_H,_C5_UPTIME+0x03
		LDS TEMP_L,_C5_UPTIME+0x04
		SUB TEMP_L,YL
		SBC TEMP_H,YH
		CP TEMP_H,TEMP
		BRCS _DRV_IR_PROC__RECEIVING_WAIT
		BRNE PC+0x03
		CP TEMP_L,ACCUM
		BRCS _DRV_IR_PROC__RECEIVING_WAIT
		;Достигли таймаута
		MOVW YL,ZL
		MCALL _DRV_IR_DONE
		LDI TEMP,DRV_IR_FAIL
		STD Z+_DRV_IR_STATE,TEMP
		RJMP _DRV_IR_PROC__RX_DONE
_DRV_IR_PROC__NO_RX_WAIT:
		SBRS TEMP,0x07
		RJMP _DRV_IR_PROC__RECEIVING_WAIT
_DRV_IR_PROC__RX_DONE:
_DRV_IR_PROC__RX_SKIP:
	LDD TEMP_H,Z+_DRV_IR_STATE
	LDD TEMP_L,Z+_DRV_IR_BIT_POS

	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_IR_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик прерывания от таймера
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	MOVW YL,ZL
	LDD TEMP,Y+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_RX_DATA+0x01
	BRCS PC+0x06
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_FAIL
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_TIMER_EVENT__END
	LSL TEMP
	LDI_Z _DRV_IR_TIMER_EVENT__JUMP_TABLE*2
	ADD ZL,TEMP
	ADC ZH,C0x00
	LPM TEMP,Z+
	LPM ZH,Z
	MOV ZL,TEMP
	LDD ACCUM,Y+_DRV_IR_TX_PORT
	LDD TEMP,Y+_DRV_IR_BIT_POS
	ICALL
_DRV_IR_TIMER_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

_DRV_IR_TIMER_EVENT__JUMP_TABLE:
	.dw	_DRV_IR_TIMER_EVENT__TX_PREAMBLE_LO, _DRV_IR_TIMER_EVENT__TX_PREAMBLE_HI,_DRV_IR_TIMER_EVENT__TX_DATA_LO,\
			_DRV_IR_TIMER_EVENT__TX_DATA_HI, _DRV_IR_TIMER_EVENT__RX_PREAMBLE, _DRV_IR_TIMER_EVENT__RX_DATA

_DRV_IR_TIMER_EVENT__TX_PREAMBLE_LO:
	;Перамбула, низкий уровень, включаем высокий
	MCALL PORT_SET_HI
	LDI TEMP,_DRV_IR_TX_PREAMBLE_HI
	STD Y+_DRV_IR_STATE,TEMP
	STD Y+_DRV_IR_BIT_POS,C0x00
	RET
_DRV_IR_TIMER_EVENT__TX_PREAMBLE_HI:
_DRV_IR_TIMER_EVENT__TX_DATA_HI:
	;Перамбула или данные, высокий уровень, включаем данные, низкий
	MCALL PORT_SET_LO
	ANDI TEMP,0xf8
	BRNE _DRV_IR_TIMER_EVENT__SKIP_ACCUM_LOADING
	LSR TEMP
	LSR TEMP
	LSR TEMP
	LDD ZH,Y+_DRV_IR_SRC_ADDR+0x00
	LDD ZL,Y+_DRV_IR_SRC_ADDR+0x01
	ADD ZL,TEMP
	ADC ZH,C0x00
	LD TEMP,Z
	STD Y+_DRV_IR_ACCUM,TEMP
_DRV_IR_TIMER_EVENT__SKIP_ACCUM_LOADING:
	;Устанавливаем длительность для данных, низкий уровень
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_TX_LO
	MCALL C5_TIMER_START_AT
	LDI TEMP,_DRV_IR_TX_DATA_LO
	STD Y+_DRV_IR_STATE,TEMP
	RET
_DRV_IR_TIMER_EVENT__TX_DATA_LO:
	;Данные, низкий уровень включаем выский разной длительностью
	MCALL PORT_SET_HI
	LDD ACCUM,Y+_DRV_IR_TX_LEN
	CP ACCUM,TEMP
	BREQ _DRV_IR_TIMER_EVENT__TX_DONE
	INC TEMP
	STD Y+_DRV_IR_BIT_POS,TEMP
	LDD TEMP,Y+_DRV_IR_ACCUM
	LSR TEMP
	STD Y+_DRV_IR_ACCUM,TEMP
	LDI ACCUM,_DRV_IR_TX_HI0
	LDI TEMP,_DRV_IR_TX_DATA_LO
	BRCC PC+0x03
	LDI ACCUM,_DRV_IR_TX_HI1
	LDI TEMP,_DRV_IR_TX_DATA_HI
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	MCALL C5_TIMER_START_AT
	STD Y+_DRV_IR_STATE,TEMP
	RET
_DRV_IR_TIMER_EVENT__TX_DONE:
	MCALL _DRV_IR_DONE
	LDI TEMP,_DRV_IR_TX_DONE
	STD Y+_DRV_IR_STATE,TEMP
	RET
_DRV_IR_TIMER_EVENT__RX_PREAMBLE:
	;Таймаут преамбулы или приема данных
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_FAIL
	STD Y+_DRV_IR_STATE,TEMP
	RET
_DRV_IR_TIMER_EVENT__RX_DATA:
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_READY
	STD Y+_DRV_IR_STATE,TEMP
	RET

;--------------------------------------------------------
_DRV_IR_EINT_EVENT:
;--------------------------------------------------------
;Обработчик внешнего прерывания
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	MOVW YL,ZL

	LDD TEMP,Y+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_RX_WAIT
	BRNE _DRV_IR_EINT_EVENT__NO_RX_WAIT
	;Запоминаем метку времени для TIMEOUT(x2ms)
	LDS TEMP,_C5_UPTIME+0x04
	STD Y+_DRV_IR_TIMESTAMP,TEMP
	;Запускаем таймер для PREAMBLE TIMEOUT
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_RX_MAX_PREAMBLE
	MCALL C5_TIMER_START_AT
	LDI TEMP,_DRV_IR_RX_PREAMBLE
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
_DRV_IR_EINT_EVENT__NO_RX_WAIT:
	CPI TEMP,_DRV_IR_RX_PREAMBLE
	BRNE _DRV_IR_EINT_EVENT__NO_RX_PREAMBLE
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
	;TODO Нужно ли проверять на минимальную преамбулу?
	LDS TEMP,_C5_UPTIME+0x04
	LDD ACCUM,Y+_DRV_IR_TIMESTAMP
	SUB TEMP,ACCUM
	CPI TEMP,_DRV_IR_RX_MIN_PREAMBLE&0x7f
	BRCC PC+0x06
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_FAIL
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
	;Запоминаем метку времени
	LDS TEMP,_C5_MAIN_TIMER_CNTR
	STD Y+_DRV_IR_TIMESTAMP,TEMP
	;Запускаем таймер для BIT TIMEOUT
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_RX_BIT_TIME_MAX
	MCALL C5_TIMER_START_AT
	;Включаю индикатор передачи
	LDD ACCUM,Y+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_HI
	;Устанавливаем состояние приема данных
	LDI TEMP,_DRV_IR_RX_DATA
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
_DRV_IR_EINT_EVENT__NO_RX_PREAMBLE:
	CPI TEMP,_DRV_IR_RX_DATA
	BRNE _DRV_IR_EINT_EVENT__NO_RX_DATA
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
SBI PINC,PC4&0x0f
	;Запоминаем метку времени
	LDD TEMP,Y+_DRV_IR_TIMESTAMP
	LDS ACCUM,_C5_MAIN_TIMER_CNTR
	STD Y+_DRV_IR_TIMESTAMP,ACCUM
	SUB ACCUM,TEMP
	CPI ACCUM,_DRV_IR_RX_BIT_TIME_MIN
	BRCC PC+0x06
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_READY
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
	;Проверка на максимальную длину
	LDD TEMP,Y+_DRV_IR_BIT_POS
	LDD ZH,Y+_DRV_IR_RX_LEN
	CP TEMP,ZH
	BRNE PC+0x06
	MCALL _DRV_IR_DONE
	LDI TEMP,DRV_IR_OVERFLOW
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
	;Записываем бит
	LDD ZL,Y+_DRV_IR_ACCUM
	CPI ACCUM,_DRV_IR_RX_BIT_TRESHOLD
	ROR ZL
	STD Y+_DRV_IR_ACCUM,ZL
	;Инкрементируем позицию бита
	INC TEMP
	STD Y+_DRV_IR_BIT_POS,TEMP
	MOV ZL,TEMP
	ANDI ZL,0xf8
	BRNE _DRV_IR_EINT_EVENT__SKIP_STORE_ACCUM
	;Cохраняем ACCUM
	COM ACCUM
	LSR TEMP
	LSR TEMP
	LSR TEMP
	LDD ZH,Y+_DRV_IR_DST_ADDR+0x00
	LDD ZL,Y+_DRV_IR_DST_ADDR+0x01
	ADD ZL,TEMP
	ADC ZH,C0x00
	ST Z,ACCUM
_DRV_IR_EINT_EVENT__SKIP_STORE_ACCUM:
	;Запускаем таймер для BIT TIMEOUT
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	LDI ACCUM,_DRV_IR_RX_BIT_TIME_MAX
	MCALL C5_TIMER_START_AT
	RJMP _DRV_IR_EINT_EVENT__END
_DRV_IR_EINT_EVENT__NO_RX_DATA:
	MCALL _DRV_IR_DONE

_DRV_IR_EINT_EVENT__END:
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_IR_DONE:
;--------------------------------------------------------
;Завершение операции
;--------------------------------------------------------
	;Таймаут преамбулы или приема данных
	LDD TEMP,Y+_DRV_IR_TIMER_ID
	MCALL C5_TIMER_STOP
	LDD TEMP_H,Z+_DRV_IR_INT_ID
	MCALL C5_EINT_DISABLE
	;Выключаю индикатор передачи
	LDD ACCUM,Y+_DRV_IR_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x03
	MCALL PORT_SET_LO
	RET
.endif
