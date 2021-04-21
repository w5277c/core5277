;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.05.2020  w5277c@gmail.com			Начало
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;19.01.2021  w5277c@gmail.com			Обновлен блок EEPROM
;21.01.2021  w5277c@gmail.com			Дбавлены порты для инструкций SBI,CBI...
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for ATMega8 (8Kb ROM, 512b EEPROM, 1Kb SRAM) at 16MHz (LITTLE ENDIAN)
;avrdude -p m8 -c avrispmkII -U flash:w:main.hex
;;;;;Ext crystal 8.0-..., 16K CK/14 CK + 4.1ms, CKDIV8 disabled, BOD disabled, SPI enabled:
;;;;;avrdude -p m8 -c avrispmkII -U lfuse:w:0xef:m -U hfuse:w:0xdc:m -U efuse:w:0xf9:m

	;---SPECIFY-DEVICE---
	.DEVICE ATmega8

	#pragma AVRPART MEMORY PROG_FLASH 8192
	#pragma AVRPART MEMORY EEPROM 512
	#pragma AVRPART MEMORY INT_SRAM SIZE 1024
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60

	#include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATMEGA8
	.MESSAGE "######## DEVICE ATmega8"
	.SET	_IV_STEP									= 0x01;			;Шаг между адресами переходов на обработчики прерываний
	.EQU	_C5_DRIVERS_MAX_QNT					= 0x10			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT						= 0x08			;Максимальное количество задач
	.SET	C5_DRIVERS_QNT							= _C5_DRIVERS_MAX_QNT
	.SET	C5_TASKS_QNT							= _C5_TASKS_MAX_QNT
	.EQU	_C5_IR_QNT								= 0x14			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE					= 0x10			;Размер очереди к ресурсам(8 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT					= 0x00			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE					= 0x04			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE							= 0x40			;Стек ядра
	.EQU	RAMSTART									= 0x0060
	.EQU	RAMEND									= 0x045f
	.SET	INPUT_BUFFER_SIZE						= 0x40			;Размер буфера ввода (не менее 2)

	;---IR---
	.EQU	C5_IR_INT0								= 0x01
	.EQU	C5_IR_INT1								= 0x02
	.EQU	C5_IR_TIMER2_COMP						= 0x03
	.EQU	C5_IR_TIMER2_OVF						= 0x04
	.EQU	C5_IR_TIMER1_CAPT						= 0x05
	.EQU	C5_IR_RESERVED_CORE_TIMER1			= 0x06
	.EQU	C5_IR_RESERVED_CORE_TIMER2			= 0x07
	.EQU	C5_IR_TIMER1_OVF						= 0x08
	.EQU	C5_IR_TIMER0_OVF						= 0x09
	.EQU	C5_IR_RESERVED_CORE_TIMER			= 0x0a
	.EQU	C5_IR_SPI								= 0x0b
	.EQU	C5_IR_UART_RX							= 0x0c
	.EQU	C5_IR_UART_UDRE						= 0x0d
	.EQU	C5_IR_UART_TX							= 0x0e
	.EQU	C5_IR_ADC								= 0x0f
	.EQU	C5_IR_EE_READY							= 0x10
	.EQU	C5_IR_ANALOG_COMP						= 0x11
	.EQU	C5_IR_TWI								= 0x12
	.EQU	C5_IR_SPM_READY						= 0x13

	;SBI/CBI/SBIS/SBIC port defines
	.EQU	PORTA										= 0xff
	.EQU	DDRA										= 0xff
	.EQU	PINA										= 0xff
	.EQU	PORTB										= 0x18
	.EQU	DDRB										= 0x17
	.EQU	PINB										= 0x16
	.EQU	PORTC										= 0x15
	.EQU	DDRC										= 0x14
	.EQU	PINC										= 0x13
	.EQU	PORTD										= 0x12
	.EQU	DDRD										= 0x11
	.EQU	PIND										= 0x10
	.EQU	PORTE										= 0xff
	.EQU	DDRE										= 0xff
	.EQU	PINE										= 0xff
	.EQU	PORTF										= 0xff
	.EQU	DDRF										= 0xff
	.EQU	PINF										= 0xff
	.EQU	PORTG										= 0xff
	.EQU	DDRG										= 0xff
	.EQU	PING										= 0xff

	;Отсутствуют
	.EQU	C5_IR_PCINT0							= 0x00
	.EQU	C5_IR_PCINT1							= 0x00
	.EQU	C5_IR_PCINT2							= 0x00

	.EQU	SDA										= PC4
	.EQU	SCL										= PC5
	.EQU	RXD										= PD0
	.EQU	TXD										= PD1
	.EQU	RXD0										= RXD
	.EQU	TXD0										= TXD
	.EQU	SCK										= PB5
	.EQU	MISO										= PB4
	.EQU	MOSI										= PB3
	.EQU	SS											= PB2

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	;--FAKE-PRR---
	.EQU	PRR										= 0xff
	.EQU		PRTWI									= 0x00
	.EQU		PRTIM2								= 0x00
	.EQU		PRTIM0								= 0x00
	.EQU		PRTIM1								= 0x00
	.EQU		PRSPI									= 0x00
	.EQU		PRUSART0								= 0x00
	.EQU		PRADC									= 0x00
	;---TWI---
	.EQU	TWCR										= 0xbc
	.EQU	TWDR										= 0xbb
	.EQU	TWAR										= 0xba
	.EQU	TWSR										= 0xb9
	.EQU	TWBR										= 0xb8
	;---TIMERS---
	.EQU	TIMSK										= 0x59
	.EQU	TCCR1A									= 0x4f
	.EQU	TCCR1B									= 0x4e
	.EQU	TCNT1H									= 0x4d
	.EQU	TCNT1L									= 0x4c
	.EQU	OCR1AH									= 0x4b
	.EQU	OCR1AL									= 0x4a
	.EQU	OCR1BH									= 0x49
	.EQU	OCR1BL									= 0x48
	.EQU	TIFR										= 0x58
	;---
	.EQU	OCIE1A									= 0x04
	.EQU	WGM13										= 0x04
	.EQU	WGM12										= 0x03
	.EQU	WGM11										= 0x01
	.EQU	WGM10										= 0x00
	.EQU	CS12										= 0x02
	.EQU	CS11										= 0x01
	.EQU	CS10										= 0x00
	.EQU	OCF1A										= 0x04
	.EQU	OCF1B										= 0x03
	.EQU	OCIE1B									= 0x03
	;TODO дополнить
	;---EEPROM---
	.EQU	EEARH										= 0x3F
	.EQU	EEARL										= 0x3E
	.EQU	EEDR										= 0x3D
	.EQU	EECR										= 0x3C
	.EQU		EERIE									= 0x03
	.EQU		EEMPE									= 0x02
	.EQU		EEMWE									= EEMPE
	.EQU		EEPE									= 0x01
	.EQU		EEWE									= EEPE
	.EQU		EERE									= 0x00
	;---ADC---
	.EQU	ADMUX										= 0x27
	.EQU		REFS1									= 0x07
	.EQU		REFS0									= 0x06
	.EQU		ADLAR									= 0x05
	.EQU		MUX3									= 0x03
	.EQU		MUX2									= 0x02
	.EQU		MUX1									= 0x01
	.EQU		MUX0									= 0x00
	.EQU	ADCSRA									= 0x26
	.EQU		ADEN									= 0x07
	.EQU		ADSC									= 0x06
	.EQU		ADATE									= 0x05
	.EQU		ADIF									= 0x04
	.EQU		ADIE									= 0x03
	.EQU		ADPS2									= 0x02
	.EQU		ADPS1									= 0x01
	.EQU		ADPS0									= 0x00
	.EQU	ADCL										= 0x24
	.EQU	ADCH										= 0x25
	.EQU	ADC0										= 0x00
	.EQU	ADC1										= 0x01
	.EQU	ADC2										= 0x02
	.EQU	ADC3										= 0x03
	.EQU	ADC4										= 0x04
	.EQU	ADC5										= 0x05
	.EQU	ADC6										= 0x06
	.EQU	ADC7										= 0x07
	.EQU	ADCT										= 0x08
	;---INT---
	.EQU	MCUCR										= 0x55
	.EQU		SE										= 0x07
	.EQU		SM2									= 0x06
	.EQU		SM1									= 0x05
	.EQU		SM0									= 0x04
	.EQU		ISC11									= 0x03
	.EQU		ISC10									= 0x02
	.EQU		ISC01									= 0x01
	.EQU		ISC00									= 0x00
	.EQU	GICR										= 0x5b
	.EQU		INT0									= 0x06
	.EQU		INT1									= 0x07
	.EQU		IVSEL									= 0x01
	.EQU		IVCE									= 0x00
	.EQU	GIFR										= 0x5a
	.EQU		INTF0									= 0x06
	.EQU		INTF1									= 0x07
	.EQU	EICRA										= MCUCR
	.EQU	EIMSK										= GICR
	.EQU	EIFR										= GIFR
	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AREF							= (0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AVCC							= (0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56_CAP						= (1<<REFS1)|(1<<REFS0)

	.SET	INT0_PORT								= PD2
	.SET	INT1_PORT								= PD3

.ORG 0x0000
	RJMP	MAIN															; Reset Handler
	RCALL	_C5_IR														; External Interrupt0 Handler
	RCALL	_C5_IR														; External Interrupt1 Handler
	RCALL	_C5_IR														; Pin Change Interrupt Request 0
	RCALL	_C5_IR														; Pin Change Interrupt Request 1
	RCALL	_C5_IR														; Pin Change Interrupt Request 2
	RCALL	_C5_IR														; Watchdog Time-out Interrupt
	RCALL	_C5_TIMER_C_IR												; Timer/Counter2 Compare Match A
	RCALL	_C5_IR														; Timer/Counter2 Compare Match B
	RCALL	_C5_IR														; Timer/Counter2 Overflow
	RCALL	_C5_IR														; Timer/Counter1 Capture Event
	RCALL	_C5_IR														; Timer/Counter1 Compare Match A
	RCALL	_C5_IR														; Timer/Counter1 Compare Match B
	RCALL	_C5_IR														; Timer/Counter1 Overflow
	RJMP	_C5_TIMER_A_IR												; Timer/Counter0 Compare Match A
	RJMP	_C5_TIMER_B_IR												; Timer/Counter0 Compare Match B
	RCALL	_C5_IR														; Timer/Counter0 Overflow
	RCALL	_C5_IR														; SPI Serial Transfer Complete
	RCALL	_C5_IR														; USART Rx Complete
	RCALL	_C5_IR														; USART, Data Register Empty
	RCALL	_C5_IR														; USART, Tx Complete
	RCALL	_C5_IR														; ADC Conversion Complete
	RCALL	_C5_IR														; EEPROM Ready
	RCALL	_C5_IR														; Analog Comparator
	RCALL	_C5_IR														; 2-wire Serial Interface
	RCALL	_C5_IR														; Store Program Memory Ready



PORTS_TABLE:															;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA,PORTB,DDRB,PINB и т.д.
	.db	0x00,0x00,0x00,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30
PCINT_TABLE:
	.db	0x00,0x00,0x00,0x00
INT_TABLE:
	.db	(1<<INT0),(1<<INT1)

APPLICATION_BLOCK:

.MACRO MCALL
	RCALL @0
	NOP
.ENDMACRO
.MACRO MJMP
	RJMP @0
	NOP
.ENDMACRO

.MACRO _C5_TIMERA_INIT
	LDS TEMP,PRR
	ANDI TEMP,~(1<<PRTIM0)											;Подпитываю TIMER0
	STS PRR,TEMP

	LDI TEMP,(0<<WGM01)|(0<<WGM00)
	STS TCCR0A,TEMP
	LDI TEMP,(0<<CS02)|(1<<CS01)|(0<<CS00)						;/8 - for 16MHz
	STS TCCR0B,TEMP
	CLR TEMP
	STS TCNT0,TEMP
	LDI TEMP,TIMERS_SPEED-0x01										;0x64 - 100(16MHz)
	STS OCR0A,TEMP
	LDI TEMP,(1<<OCF0A)
	STS TIFR,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_START
	LDI TEMP,(1<<OCIE0A)
	STS TIMSK,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_CORRECTOR
	PUSH ACCUM
	LDS ACCUM,TCNT0
	ADD ACCUM,@0
	STS OCR0A,ACCUM
	LDI ACCUM,(1<<OCF0A)
	STS TIFR,ACCUM
	POP ACCUM
.ENDMACRO

.MACRO _C5_TIMERB
	;SUBI TEMP,0x08														;Подобрано вручную, не должно быть вообще
	LDS TEMP_L,TCNT0
	ADD TEMP_L,TEMP
	STS OCR0B,TEMP_L

	LDS TEMP_L,SREG
	CLI
	LDS TEMP,TIFR
	ORI TEMP,(1<<OCF0B)
	STS TIFR,TEMP
	;Установка флага
	LDI TEMP,(1<<_CFL_TIMER_B_USE)
	OR _C5_COREFLAGS,TEMP
	;Запуск таймера
	LDS TEMP,TIMSK
	ORI TEMP,(1<<OCIE0B)
	STS TIMSK,TEMP
	STS SREG,TEMP_L
	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP_L,SREG
	CLI
	LDS TEMP,TIMSK
	ANDI TEMP,~(1<<OCIE0B)
	STS TIMSK,TEMP
	STS SREG,TEMP_L
.ENDMACRO

.MACRO _C5_TIMERC_SET_PERIOD
	STS TCNT2,TEMP_H
	STS OCR2,TEMP_L
.ENDMACRO
.MACRO _C5_TIMERC_START
	LDI TEMP,(1<<WGM21)|(0<<WGM20)|(0<<CS22)|(1<<CS21)|(0<<CS20);/8
	STS TCCR2,TEMP
	_C5_TIMERC_SET_PERIOD
	LDS TEMP_H,SREG
	CLI
	LDS TEMP,TIMSK
	ORI TEMP,(1<<OCIE2A)
	STS TIMSK,TEMP
	LDS TEMP,TIFR
	ORI TEMP,(1<<OCF2)
	STS TIFR,TEMP
	STS SREG,TEMP_H
.ENDMACRO
.MACRO _C5_TIMERC_STOP
	LDS TEMP,SREG
	CLI
	PUSH TEMP
	LDS TEMP,TIFR
	ANDI TEMP,~(1<<OCF2)
	STS TIFR,TEMP
	POP TEMP
	STS SREG,TEMP
.ENDMACRO

.MACRO _C5_POWER_ON
.ENDMACRO
.MACRO _C5_POWER_OFF
.ENDMACRO

.MACRO _C5_TIMER_TCNT
	LDS @0,TCNT1L
.ENDMACRO
