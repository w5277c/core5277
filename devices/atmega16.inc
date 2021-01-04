;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.05.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for ATMega16 (16Kb ROM, 512b EEPROM, 1Kb SRAM) at 16MHz (LITTLE ENDIAN)
;avrdude -p m16 -c avrispmkII -U flash:w:main.hex
;Ext crystal hi freq, 1K CK + 4ms, SPI enabled:
;avrdude -p m16 -c avrispmkII -U lfuse:w:0xfe:m -U hfuse:w:0xde:m

	;---SPECIFY-DEVICE---
   .DEVICE ATmega16

	#pragma AVRPART MEMORY PROG_FLASH 16384
	#pragma AVRPART MEMORY EEPROM 512
	#pragma AVRPART MEMORY INT_SRAM SIZE 1024
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60

	#include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATMEGA16
	.MESSAGE "######## DEVICE ATmega16 ########"
	.EQU	_C5_DRIVERS_MAX_QNT			= 0x10			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT				= 0x08			;Максимальное количество задач
	.EQU	_C5_IR_QNT						= 0x15			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE			= 0x10			;Размер очереди к ресурсам(8 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT			= 0x00			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE			= 0x20			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE					= 0x40			;Стек ядра
	.EQU	RAMEND							= 0x045f

	;---IR---
	.EQU	C5_IR_INT0						= 0x01
	.EQU	C5_IR_INT1						= 0x02
	.EQU	C5_IR_TIMER2_COMP				= 0x03
	.EQU	C5_IR_TIMER2_OVF				= 0x04
	.EQU	C5_IR_TIMER1_CAPT				= 0x05
	.EQU	C5_IR_RESERVED_CORE_TIMER1	= 0x06
	.EQU	C5_IR_RESERVED_CORE_TIMER2	= 0x07
	.EQU	C5_IR_TIMER1_OVF				= 0x08
	.EQU	C5_IR_TIMER0_OVF				= 0x09
	.EQU	C5_IR_SPI						= 0x0a
	.EQU	C5_IR_UART_RX					= 0x0b
	.EQU	C5_IR_UART_UDRE				= 0x0c
	.EQU	C5_IR_UART_TX					= 0x0d
	.EQU	C5_IR_ADC						= 0x0e
	.EQU	C5_IR_EE_READY					= 0x0f
	.EQU	C5_IR_ANALOG_COMP				= 0x10
	.EQU	C5_IR_TWI						= 0x11
	.EQU	C5_IR_INT2						= 0x12
	.EQU	C5_IR_TIMER0_COMP				= 0x13
	.EQU	C5_IR_SPM_READY				= 0x14
	;Отсутствуют
	.EQU	C5_IR_PCINT0					= 0x00
	.EQU	C5_IR_PCINT1					= 0x00
	.EQU	C5_IR_PCINT2					= 0x00

	.EQU	PA0										= (0x00<<4)|0x00
	.EQU	PA1										= (0x00<<4)|0x01
	.EQU	PA2										= (0x00<<4)|0x02
	.EQU	PA3										= (0x00<<4)|0x03
	.EQU	PA4										= (0x00<<4)|0x04
	.EQU	PA5										= (0x00<<4)|0x05
	.EQU	PA6										= (0x00<<4)|0x06
	.EQU	PA7										= (0x00<<4)|0x07
	.EQU	PB0										= (0x01<<4)|0x00
	.EQU	PB1										= (0x01<<4)|0x01
	.EQU	PB2										= (0x01<<4)|0x02
	.EQU	PB3										= (0x01<<4)|0x03
	.EQU	PB4										= (0x01<<4)|0x04
	.EQU	PB5										= (0x01<<4)|0x05
	.EQU	PB6										= (0x01<<4)|0x06
	.EQU	PB7										= (0x01<<4)|0x07
	.EQU	PC0										= (0x02<<4)|0x00
	.EQU	PC1										= (0x02<<4)|0x01
	.EQU	PC2										= (0x02<<4)|0x02
	.EQU	PC3										= (0x02<<4)|0x03
	.EQU	PC4										= (0x02<<4)|0x04
	.EQU	PC5										= (0x02<<4)|0x05
	.EQU	PC6										= (0x02<<4)|0x06
	.EQU	PC7										= (0x02<<4)|0x07
	.EQU	PD0										= (0x03<<4)|0x00
	.EQU	PD1										= (0x03<<4)|0x01
	.EQU	PD2										= (0x03<<4)|0x02
	.EQU	PD3										= (0x03<<4)|0x03
	.EQU	PD4										= (0x03<<4)|0x04
	.EQU	PD5										= (0x03<<4)|0x05
	.EQU	PD6										= (0x03<<4)|0x06
	.EQU	PD7										= (0x03<<4)|0x07

	.EQU	SDA										= PC1
	.EQU	SCL										= PC0
	.EQU	RXD										= PD0
	.EQU	TXD										= PD1

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR;
	;---TWI---
	.EQU	TWCR										= 0x56
	.EQU	TWDR										= 0x23
	.EQU	TWAR										= 0x22
	.EQU	TWSR										= 0x21
	.EQU	TWBR										= 0x20
	;---
	.EQU	TWINT										= 0x07
	.EQU	TWEA										= 0x06
	.EQU	TWSTA										= 0x05
	.EQU	TWSTO										= 0x04
	.EQU	TWWC										= 0x03
	.EQU	TWEN										= 0x02
	.EQU	TWIE										= 0x00
	.EQU	TWPS0										= 0x00
	.EQU	TWPS1										= 0x01
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
	;---UART---
	.EQU	UDR										= 0x2c
	.EQU	UCSRA										= 0x2b
	.EQU	UCSRB										= 0x2a
	.EQU	UCSRC										= 0x40
	.EQU	UBRRH										= 0x40
	.EQU	UBRRL										= 0x29
	.EQU	UCSZ0										= 0x01
	.EQU	UCSZ1										= 0x02
	.EQU	UCSZ2										= 0x02
	.EQU	URSEL										= 0x07
	.EQU	UMSEL										= 0x06
	.EQU	UPM0										= 0x04
	.EQU	UPM1										= 0x05
	.EQU	USBS										= 0x03
	.EQU	RXC										= 0x07
	.EQU	TXC										= 0x06
	.EQU	RXCIE										= 0x07
	.EQU	TXCIE										= 0x06
	.EQU	RXEN										= 0x04
	.EQU	TXEN										= 0x03
	.EQU	UDR0										= UDR
	.EQU	UCSR0A									= UCSRA
	.EQU	UCSR0B									= UCSRB
	.EQU	UCSR0C									= UCSRC
	.EQU	UBRR0H									= UBRRH
	.EQU	UBRR0L									= UBRRL
	;---PCINT---
	.EQU	PCMSK0									= 0x00
	.EQU	PCMSK1									= 0x00
	.EQU	PCMSK2									= 0x00
	;---ADC---
	.EQU	ADMUX										= 0x27
	.EQU		REFS1									= 0x07
	.EQU		REFS0									= 0x06
	.EQU		ADLAR									= 0x05
	.EQU		MUX4									= 0x04
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

	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AREF						= (0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AVCC						= (0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56_CAP					= (1<<REFS1)|(1<<REFS0)

.ORG 0x0000
	JMP	MAIN															; Reset
	CALL	_C5_IR												; External Interrupt0
	CALL	_C5_IR												; External Interrupt1
	CALL	_C5_IR												; Timer/Counter2 Compare
	CALL	_C5_IR												; Timer/Counter2 Overflow
	CALL	_C5_IR												; Timer/Counter1 Capture Event
	JMP	_C5_TIMER_A_IR										; Timer/Counter1 Compare Match A
	JMP	_C5_TIMER_B_IR										; Timer/Counter1 Compare Match B
	CALL	_C5_IR												; Timer/Counter1 Overflow
	CALL	_C5_IR												; Timer/Counter0 Overflow
	CALL	_C5_IR												; SPI Serial Transfer Complete
	CALL	_C5_IR												; USART Rx Complete
	CALL	_C5_IR												; USART, Data Register Empty
	CALL	_C5_IR												; USART, Tx Complete
	CALL	_C5_IR												; ADC Conversion Complete
	CALL	_C5_IR												; EEPROM Ready
	CALL	_C5_IR												; Analog Comparator
	CALL	_C5_IR												; 2-wire Serial Interface
	CALL	_C5_IR												; External Interrupt2
	CALL	_C5_IR												; Timer/Counter0 Compare
	CALL	_C5_IR												; Store Program Memory Ready

PORTS_TABLE:												;Таблица из адресов IO регистров (A,B,C,D), поочередно DDRA,PORTA,PINA,DDRB,PORTB,PINB и т.д.
	.db	0x3b,0x3a,0x39,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30
PCINT_TABLE:
	.db	0x00,0x00,0x00,0x00
.MACRO MCALL
	CALL @0
.ENDMACRO
.MACRO MJMP
	JMP @0
.ENDMACRO

.MACRO _C5_TIMERA_INIT
	LDI TEMP,(0<<WGM11)|(0<<WGM10)
	STS TCCR1A,TEMP
	LDI TEMP,(0<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10);/8 - for 16MHz
	STS TCCR1B,TEMP
	CLR TEMP
	STS TCNT1L,TEMP
	STS TCNT1H,TEMP
	LDI TEMP,TIMERS_SPEED-0x01										;0x64 - 100(16MHz)
	STS OCR1AL,TEMP
	LDI TEMP,0x00
	STS OCR1AH,TEMP
	LDI TEMP,(1<<OCF1A)
	STS TIFR,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_START
	LDS TEMP,TIMSK
	ORI TEMP,(1<<OCIE1A)
	STS TIMSK,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_CORRECTOR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	LDS TEMP_L,TCNT1L
	LDS TEMP_H,TCNT1H
	ADD TEMP_L,@0
	CLR TEMP
	ADC TEMP_H,TEMP
	STS OCR1AH,TEMP_H
	STS OCR1AL,TEMP_L
	LDI TEMP,(1<<OCF1A)
	STS TIFR,TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_CORRECTOR
	PUSH TEMP_H
	PUSH TEMP_L
	LDS TEMP_L,TCNT1L
	LDS TEMP_H,TCNT1H
	LDI TEMP,TIMERS_SPEED-0x01
	SBRC _C5_COREFLAGS,_CFL_TIMER_UART_CORRECTION
	LDI TEMP,TIMERS_SPEED+0x02
	ADD TEMP_L,TEMP
	CLR TEMP
	ADC TEMP_H,TEMP
	STS OCR1AH,TEMP_H
	STS OCR1AL,TEMP_L
	POP TEMP_L
	POP TEMP_H
.ENDMACRO

.MACRO _CORRE5277_IR_OFFSET_CORRECTOR
	LSR _C5_TEMP_L
	DEC _C5_TEMP_L												;Получено на базе адреса выхода из процедуры(не входа), т.е. увеличен на 1
.ENDMACRO

.MACRO _C5_TIMERB
	;SUBI TEMP,0x08														;Подобрано вручную, не должно быть вообще
	LDS TEMP_L,TCNT1L
	LDS TEMP_H,TCNT1H
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	STS OCR1BH,TEMP_H
	STS OCR1BL,TEMP_L
	LDS TEMP,TIFR
	ORI TEMP,(1<<OCF1B)
	STS TIFR,TEMP
	;Установка флага
	LDI TEMP,(1<<_CFL_TIMER_B_USE)
	OR _C5_COREFLAGS,TEMP
	;Запуск таймера
	LDS TEMP,TIMSK
	ORI TEMP,(1<<OCIE1B)
	STS TIMSK,TEMP
	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP,TIMSK
	ANDI TEMP,~(1<<OCIE1B)
	STS TIMSK,TEMP
.ENDMACRO

.MACRO _C5_POWER_ON
.ENDMACRO
.MACRO _C5_POWER_OFF
.ENDMACRO

.MACRO _C5_TIMER_TCNT
	LDS @0,TCNT1L
.ENDMACRO

