;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.04.2021	w5277c@gmail.com			Начало
;20.04.2021	w5277c@gmail.com			Отказался от драйвера PCINT
;16.09.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
;Програмный UART 8n1, более быстрый, упрощенный (работает только с портом B) и использует TIMER C
;-----------------------------------------------------------------------------------------------------------------------
;TODO добавить поддержку внешний прерываний
;TODO реализовать вычисление таймаута между передачей байт
;TODO хранить порты в первых байтах формата ./io/port, числа пинов хранить отдельно
;TODO поддержка MAGIC?
;TODO учесть изменения в процедурах TIMER C
;TODO перейти с /io/port_... на /io/reg_...
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_UART_F
.else
.set DEF__C5_DRV_UART_F = 1
.message "Included driver software fast UART v0.1"

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
.include	"./core/eint_enable.inc"
.include	"./core/eint_disable.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_UART_F_RX_PORT					= DRV_UART_RX_PORT	;1B - Порт для приема данных
	.EQU	_DRV_UART_F_TX_PORT					= DRV_UART_TX_PORT	;1B - Порт для передачи данных
	.EQU	_DRV_UART_F_DR_PORT					= DRV_UART_DR_PORT	;1B - Порт для задания направления прием/передача
	.EQU	_DRV_UART_F_STATE						= 0x03			;1B - Статус драйвера
	;---
	.EQU	_DRV_UART_F_SRC_ADDR					= 0x04			;2B - адрес блока данных для передачи
	.EQU	_DRV_UART_F_DST_ADDR					= 0x06			;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_UART_F_BUFFER_POS				= 0x08			;1B - Позиция в буфере
	.EQU	_DRV_UART_F_TX_LEN					= 0x09			;1B - Длина данных для передачи
	.EQU	_DRV_UART_F_RX_LEN					= 0x0a			;1B - Максимальный размер данных для приема
	.EQU	_DRV_UART_F_BIT_NUM					= 0x0b			;1B - Номер текущего бита
	.EQU	_DRV_UART_F_ACCUM						= 0x0c			;1B - аккумулятор (байт подготовленный для передачи/приема)
		;---
	.EQU	_DRV_UART_F_TIMER_PERIOD			= 0x0d			;1B - период работы таймера для приема/передачи битов
	.EQU	_DRV_UART_F_FIRST_TIMEOUT			= 0x0e			;2B - Таймаут ожидания первого байта (2ms)
	.EQU	_DRV_UART_F_NEXT_TIMEOUT			= 0x10			;1B - Таймаут ожидания последующего байта(0.050ms)

	.EQU	_DRV_UART_F_RAM_SIZE					= 0x11			;17 байт необходимо выделить

	;---BAUDRATE_IDS---
	.EQU	DRV_UART_F_BAUDRATE_9600		= CORE_FREQ*125000/9600-1
	.EQU	DRV_UART_F_BAUDRATE_14400		= CORE_FREQ*125000/14400-1
	.EQU	DRV_UART_F_BAUDRATE_19200		= CORE_FREQ*125000/19200-1
	.EQU	DRV_UART_F_BAUDRATE_28800		= CORE_FREQ*125000/28800-1
	.EQU	DRV_UART_F_BAUDRATE_38400		= CORE_FREQ*125000/38400-1
	.EQU	DRV_UART_F_BAUDRATE_57600		= CORE_FREQ*125000/57600-1
	.EQU	DRV_UART_F_BAUDRATE_76800		= CORE_FREQ*125000/76800-1
	.EQU	DRV_UART_F_BAUDRATE_115200		= CORE_FREQ*125000/115200-1

;--------------------------------------------------------
DRV_UART_F_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-B порт RX, TEMP_L-B порт TX
;TEMP_EH-B порт направления передача/прием
;(0xff-не используется)
;TEMP_EL-скорость передачи данных
;FLAGS-ид драйвера PCINT
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_UART_F_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;Устанавливаю состояние READY
	LDI ACCUM,DRV_UART_ST_READY
	STD Y+_DRV_UART_F_STATE,ACCUM
	;Инициализирую порт направления данных
	STD Y+_DRV_UART_F_DR_PORT,TEMP_EH
	CPI TEMP_EH,0xff
	BREQ _DRV_UART_F_INIT__NO_DR_PORT
	MOV TEMP,TEMP_EH
	ANDI TEMP,0x0f
	MCALL BITNUM_TO_NUM
	STD Y+_DRV_UART_F_DR_PORT,TEMP
	IN ACCUM,DDRB
	OR ACCUM,TEMP
	OUT DDRB,ACCUM
	COM TEMP
	IN ACCUM,PORTB
	AND ACCUM,TEMP
	OUT PORTB,ACCUM
_DRV_UART_F_INIT__NO_DR_PORT:
	;Инициализирую порт RX
	MOV TEMP,TEMP_H
	ANDI TEMP,0x0f
	MCALL BITNUM_TO_NUM
	STD Y+_DRV_UART_F_RX_PORT,TEMP
	IN ACCUM,PORTB
	OR ACCUM,TEMP
	OUT PORTB,ACCUM
	COM TEMP
	IN ACCUM,DDRB
	AND ACCUM,TEMP
	OUT DDRB,ACCUM
	;Инициализирую порт TX
	MOV TEMP,TEMP_L
	ANDI TEMP,0x0f
	MCALL BITNUM_TO_NUM
	STD Y+_DRV_UART_F_TX_PORT,TEMP
	IN ACCUM,PORTB
	OR ACCUM,TEMP
	OUT PORTB,ACCUM
	IN ACCUM,DDRB
	OR ACCUM,TEMP
	OUT DDRB,ACCUM
	;Записываю период таймера
	STD Y+_DRV_UART_F_TIMER_PERIOD,TEMP_EL
	MOV TEMP,TEMP_EL
	;Расчитываю NEXT TIMEOUT
	;TODO!!!
	STD Y+_DRV_UART_F_NEXT_TIMEOUT,TEMP_EL

	;Устанавливаем адрес прерывания
	LDI ACCUM,C5_IR_PCINT0
	LDI TEMP_H,high(_DRV_UART_F_PCINT_EVENT)
	LDI TEMP_L,low(_DRV_UART_F_PCINT_EVENT)
	MCALL C5_IR_VECTOR_SET

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
	PUSH FLAGS

	MCALL C5_RAM_OFFSET

	;Запоминаем таймаут ожидания первого байта
	STD Y+_DRV_UART_F_FIRST_TIMEOUT+0x00,TEMP_H
	STD Y+_DRV_UART_F_FIRST_TIMEOUT+0x01,TEMP_L
	;Запминаем длину TX и RX
	STD Y+_DRV_UART_F_TX_LEN,TEMP_EH
	STD Y+_DRV_UART_F_RX_LEN,TEMP_EL

	LDI TEMP,DRV_UART_ST_READY
	STD Y+_DRV_UART_F_STATE,TEMP

	;Блок передачи
	CPI TEMP_EH,0x00
	BREQ _DRV_UART_F_PROC__TX_SKIP
	;Запоминаем SRC
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_UART_F_SRC_ADDR+0x00,ZH
	STD Y+_DRV_UART_F_SRC_ADDR+0x01,ZL
	;Запоминаем DST
	STD Y+_DRV_UART_F_DST_ADDR+0x00,XH
	STD Y+_DRV_UART_F_DST_ADDR+0x01,XL

	;Инициализирую таймер
	LDI_X _DRV_UART_F_EVENT
	MCALL C5_TIMER_C_SET_VECTOR

	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_UART_F_BUFFER_POS,C0x00
	STD Y+_DRV_UART_F_BIT_NUM,C0x00

	;Запуск передачи
	LDI TEMP,DRV_UART_ST_SENDING
	STD Y+_DRV_UART_F_STATE,TEMP
	;Порт направления в HI
	LDD ACCUM,Y+_DRV_UART_F_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x09
	LDS ZL,SREG
	CLI
	IN TEMP,PORTB
	OR TEMP,ACCUM
	OUT PORTB,TEMP
	STS SREG,ZL
	;Запускаю таймер
	LDI TEMP_H,0x00
	LDD TEMP_L,Y+_DRV_UART_F_TIMER_PERIOD
	MCALL C5_TIMER_C_START
	;Жду окончания передачи
_DRV_UART_F_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_UART_F_STATE
	CPI TEMP,DRV_UART_ST_READY
	BRNE _DRV_UART_F_PROC__SENDING_WAIT

	;Порт направления в LO
	LDD ACCUM,Y+_DRV_UART_F_DR_PORT
	CPI ACCUM,0xff
	BREQ PC+0x08
	COM ACCUM
	LDS ZL,SREG
	CLI
	IN TEMP,PORTB
	AND TEMP,ACCUM
	OUT PORTB,TEMP
	STS SREG,ZL
_DRV_UART_F_PROC__TX_SKIP:

	;Сброс позиции в буфере и номера бита
	STD Y+_DRV_UART_F_BUFFER_POS,C0x00
	STD Y+_DRV_UART_F_BIT_NUM,C0x00

	;Блок приема
	CPI TEMP_EL,0x00
	BRNE PC+0x02
	RJMP _DRV_UART_F_PROC__RX_SKIP

	;Устанавливаю бит маски PCINT
	LDD TEMP,Y+_DRV_UART_F_RX_PORT
	LDS ACCUM,PCMSK0
	OR ACCUM,TEMP
	STS PCMSK0,ACCUM

	;Запуск приема
	LDI TEMP,DRV_UART_ST_WAIT_DATA
	STD Y+_DRV_UART_F_STATE,TEMP

	;Включаю прерывание для RX порта
	MCALL _DRV_UART_F__INT_ENABLE
	;TODO проверить на возможность использования C5_WAIT при ожидании начала данных
	;Запоминаем текущий UPTIME
	LDS ZH,_C5_UPTIME+0x03
	LDS ZL,_C5_UPTIME+0x04
_DRV_UART_F_PROC__RECEIVING_WAIT:
	LDD TEMP,Y+_DRV_UART_F_STATE
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ _DRV_UART_F_PROC__RECEIVING_WAIT
	CPI TEMP,DRV_UART_ST_WAIT_DATA
	BRNE _DRV_UART_F_PROC__DONE
		;Проверка FIRST TIMEOUT
		LDD TEMP,Y+_DRV_UART_F_FIRST_TIMEOUT+0x00
		LDD ACCUM,Y+_DRV_UART_F_FIRST_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE PC+0x03
		CPI ACCUM,0x00
		BREQ _DRV_UART_F_PROC__RECEIVING_WAIT
		LDS TEMP_H,_C5_UPTIME+0x03
		LDS TEMP_L,_C5_UPTIME+0x04
		SUB TEMP_L,ZL
		SBC TEMP_H,ZH
		CP TEMP_H,TEMP
		BRCS _DRV_UART_F_PROC__RECEIVING_WAIT
		BRNE PC+0x03
		CP TEMP_L,ACCUM
		BRCS _DRV_UART_F_PROC__RECEIVING_WAIT
		;Достигли таймаута
		LDI TEMP,DRV_UART_ST_FAIL
		STD Y+_DRV_UART_F_STATE,TEMP
		;Останавливаю таймер
		MCALL C5_TIMER_C_STOP

		;Выключаю прерывание для RX порта
		MCALL _DRV_UART_F__INT_DISABLE
		;Снимаю бит в маске PCINT
		LDD TEMP,Y+_DRV_UART_F_RX_PORT
		COM TEMP
		LDS ACCUM,PCMSK0
		AND ACCUM,TEMP
		STS PCMSK0,ACCUM

_DRV_UART_F_PROC__RX_SKIP:
_DRV_UART_F_PROC__DONE:

;	LDD YH,Y+_DRV_UART_F_DST_ADDR+0x00
;	LDD YL,Y+_DRV_UART_F_DST_ADDR+0x01
;	LDI TEMP,0x08
;	MCALL C5_OUT_BYTES

	;Результат
	LDD TEMP_H,Y+_DRV_UART_F_STATE
	LDD TEMP_L,Y+_DRV_UART_F_BUFFER_POS

	POP FLAGS
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_UART_F_EVENT:
;--------------------------------------------------------
;Событие от таймера
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM
	PUSH FLAGS

	LDD TEMP,Y+_DRV_UART_F_STATE
	CPI TEMP,DRV_UART_ST_SENDING
	BREQ PC+0x02
	RJMP _DRV_UART_F_EVENT__NOT_SENDING

	LDD TEMP_H,Y+_DRV_UART_F_BIT_NUM
	CPI TEMP_H,0x00
	BRNE __DRV_UART_F_SEND_EVENT__NO_START
		;START (TX=LO)
		LDD ACCUM,Y+_DRV_UART_F_TX_PORT
		COM ACCUM
		IN TEMP,PORTB
		AND TEMP,ACCUM
		OUT PORTB,TEMP

		;Вычисляю смещение
		LDD TEMP_L,Y+_DRV_UART_F_BUFFER_POS
		LDD TEMP,Y+_DRV_UART_F_SRC_ADDR+0x00
		LDD ZL,Y+_DRV_UART_F_SRC_ADDR+0x01
		MOV ZH,TEMP
		MOV ACCUM,ZH
		ANDI ZH,0x7f
		ADD ZL,TEMP_L
		ADC ZH,C0x00
		;Считываем с буфера байт и передаем
		SBRS ACCUM,0x07
		LD TEMP,Z
		SBRC ACCUM,0x07
		LPM TEMP,Z
		STD Y+_DRV_UART_F_ACCUM,TEMP
		RJMP __DRV_UART_F_SEND_EVENT__NEXT
__DRV_UART_F_SEND_EVENT__NO_START:
	CPI TEMP_H,0xff
	BRNE __DRV_UART_F_SEND_EVENT__NOT_FINISH
		;Останавливаю таймер
		MCALL C5_TIMER_C_STOP

		LDI TEMP,DRV_UART_ST_READY
		STD Y+_DRV_UART_F_STATE,TEMP
		RJMP __DRV_UART_F_SEND_EVENT__END
__DRV_UART_F_SEND_EVENT__NOT_FINISH:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_F_SEND_EVENT__STOP
__DRV_UART_F_SEND_EVENT__DATA:
		LDD ACCUM,Y+_DRV_UART_F_TX_PORT
		IN TEMP_L,PORTB
		OR TEMP_L,ACCUM
		LDD TEMP,Y+_DRV_UART_F_ACCUM
		LSR TEMP
		STD Y+_DRV_UART_F_ACCUM,TEMP
		BRCS PC+0x03
		COM ACCUM
		AND TEMP_L,ACCUM
		OUT PORTB,TEMP_L												;Перерывания должны быть отключены
		RJMP __DRV_UART_F_SEND_EVENT__NEXT
__DRV_UART_F_SEND_EVENT__STOP:
		;STOP (TX=HI)
		LDD ACCUM,Y+_DRV_UART_F_TX_PORT
		IN TEMP,PORTB
		OR TEMP,ACCUM
		OUT PORTB,TEMP
		LDI TEMP,0x00
		STD Y+_DRV_UART_F_BIT_NUM,TEMP
		LDD TEMP_L,Y+_DRV_UART_F_BUFFER_POS
		INC TEMP_L
		STD Y+_DRV_UART_F_BUFFER_POS,TEMP_L
		LDD TEMP,Y+_DRV_UART_F_TX_LEN
		CP TEMP_L,TEMP
		BRNE __DRV_UART_F_SEND_EVENT__END
		;Делаю метку для выхода
		LDI TEMP,0xff
		STD Y+_DRV_UART_F_BIT_NUM,TEMP
		RJMP __DRV_UART_F_SEND_EVENT__END
__DRV_UART_F_SEND_EVENT__NEXT:
	INC TEMP_H
	STD Y+_DRV_UART_F_BIT_NUM,TEMP_H
__DRV_UART_F_SEND_EVENT__END:
	RJMP __DRV_UART_F_EVENT__END



_DRV_UART_F_EVENT__NOT_SENDING:
	CPI TEMP,DRV_UART_ST_RECEIVING
	BREQ PC+0x02
	RJMP __DRV_UART_F_EVENT__END

	;Читаем состояние пина в FLAGS
	LDD ACCUM,Y+_DRV_UART_F_RX_PORT
	IN FLAGS,PINB
	AND FLAGS,ACCUM
	;Читаем номер бита
	LDD TEMP_H,Y+_DRV_UART_F_BIT_NUM
	CPI TEMP_H,0x00
	BRNE __DRV_UART_F_RECV_EVENT__NO_NEXT_TIMEOUT
		;Отработал таймер после STOP, т.е. прошло времени на несколько бит и START не был обнаружен
		;Останавливаю таймер
		MCALL C5_TIMER_C_STOP
		;Прием данных завершен
		LDI TEMP,DRV_UART_ST_READY
		LDD TEMP_L,Y+_DRV_UART_F_BUFFER_POS
		LDD ACCUM,Y+_DRV_UART_F_RX_LEN
		CP TEMP_L,ACCUM
		BRNE PC+0x02
		LDI TEMP,DRV_UART_ST_OVERFLOW
		STD Y+_DRV_UART_F_STATE,TEMP
		RJMP __DRV_UART_F_RECV_EVENT__END
__DRV_UART_F_RECV_EVENT__NO_NEXT_TIMEOUT:
	CPI TEMP_H,0x09
	BREQ __DRV_UART_F_RECV_EVENT__STOP
__DRV_UART_F_RECV_EVENT__DATA:
		LDD TEMP,Y+_DRV_UART_F_ACCUM
		LSR TEMP
		CPI FLAGS,0x00
		BREQ PC+0x02
		ORI TEMP,0x80
		STD Y+_DRV_UART_F_ACCUM,TEMP
		RJMP __DRV_UART_F_RECV_EVENT__NEXT_BIT
__DRV_UART_F_RECV_EVENT__STOP:
		;STOP (TX=HI)
		CPI FLAGS,0x00
		BREQ __DRV_UART_F_RECV_EVENT__ERROR

		;Включаю прерывание для RX порта
		MCALL _DRV_UART_F__INT_ENABLE

		LDD TEMP_L,Y+_DRV_UART_F_BUFFER_POS
		LDD ACCUM,Y+_DRV_UART_F_RX_LEN
		CP TEMP_L,ACCUM
		BREQ __DRV_UART_F_RECV_EVENT__NEXT_BYTE
		LDD ACCUM,Y+_DRV_UART_F_ACCUM
		LDD TEMP,Y+_DRV_UART_F_DST_ADDR+0x00
		LDD ZL,Y+_DRV_UART_F_DST_ADDR+0x01
		MOV ZH,TEMP
		ADD ZL,TEMP_L
		ADC ZH,C0x00
		ST Z,ACCUM
		INC TEMP_L
		STD Y+_DRV_UART_F_BUFFER_POS,TEMP_L
__DRV_UART_F_RECV_EVENT__NEXT_BYTE:
		;Сбрасываю номер бита
		STD Y+_DRV_UART_F_BIT_NUM,C0x00
		;Задаю новый период таймеру для отсчета таймаута
		LDI TEMP_H,0x00
		LDI TEMP_L,0xff
		;!!! TODO LDD ACCUM,Y+_DRV_UART_F_NEXT_TIMEOUT
		MCALL C5_TIMER_C_START
		RJMP __DRV_UART_F_RECV_EVENT__END
__DRV_UART_F_RECV_EVENT__ERROR:
	;Останавливаю таймер
	MCALL C5_TIMER_C_STOP
	LDI TEMP,DRV_UART_ST_FAIL
	STD Y+_DRV_UART_F_STATE,TEMP
	RJMP __DRV_UART_F_RECV_EVENT__END
__DRV_UART_F_RECV_EVENT__NEXT_BIT:
	INC TEMP_H
	STD Y+_DRV_UART_F_BIT_NUM,TEMP_H
__DRV_UART_F_RECV_EVENT__END:

__DRV_UART_F_EVENT__END:
	POP FLAGS
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_UART_F_PCINT_EVENT:
;--------------------------------------------------------
;Обработчик смены состояния пина
;--------------------------------------------------------
	PUSH_Y
	PUSH ACCUM
	PUSH FLAGS
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	MCALL C5_RAM_OFFSET
	;Читаем состояние пина в FLAGS
	LDD ACCUM,Y+_DRV_UART_F_RX_PORT
	IN FLAGS,PINB
	AND FLAGS,ACCUM
	BRNE __DRV_UART_F_PCINT_EVENT__END

	LDD TEMP,Y+_DRV_UART_F_BIT_NUM
	CPI TEMP,0x00
	BRNE __DRV_UART_F_PCINT_EVENT__END

	LDI TEMP,0x01
	STD Y+_DRV_UART_F_BIT_NUM,TEMP

	LDI TEMP,DRV_UART_ST_RECEIVING
	STD Y+_DRV_UART_F_STATE,TEMP

	;Запускаю таймер с небольшим смещением
	LDI TEMP_H,0x08
	LDD TEMP_L,Y+_DRV_UART_F_TIMER_PERIOD
	MCALL C5_TIMER_C_START

	MCALL _DRV_UART_F__INT_DISABLE

__DRV_UART_F_PCINT_EVENT__END:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP FLAGS
	POP ACCUM
	POP_Y
	RET
.endif

;--------------------------------------------------------
_DRV_UART_F__INT_ENABLE:
;--------------------------------------------------------
;Включаем обработку прерывания INT0-1/PCINT0-7
;--------------------------------------------------------
	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDI ACCUM,(1<<PCIE0)
	LDS TEMP,PCICR
	OR TEMP,ACCUM
	STS PCICR,TEMP
	POP TEMP
	STS SREG,TEMP
	RET

;--------------------------------------------------------
_DRV_UART_F__INT_DISABLE:
;--------------------------------------------------------
;Выключаем обработку прерывания
;--------------------------------------------------------
	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDI ACCUM,~(1<<PCIE0)
	LDS TEMP,PCICR
	AND TEMP,ACCUM
	STS PCICR,TEMP
	POP TEMP
	STS SREG,TEMP
	RET

