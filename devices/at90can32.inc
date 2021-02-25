;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for AT90CAN32 (32Kb ROM, 1Kb EEPROM, 2Kb SRAM) at 16MHz max (LITTLE ENDIAN)

	;---SPECIFY-DEVICE---
	.DEVICE AT90can32

	#pragma AVRPART MEMORY PROG_FLASH 32768
	#pragma AVRPART MEMORY INT_SRAM SIZE 2048
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100
	#pragma AVRPART MEMORY EEPROM 1024

	#include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= CAN32
	.MESSAGE "######## DEVICE AT90can32"
	.SET	_IV_STEP									= 0x02;			;Шаг между адресами переходов на обработчики прерываний
	.EQU	_C5_DRIVERS_MAX_QNT					= 0x10			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT						= 0x10			;Максимальное количество задач
	.SET	C5_DRIVERS_QNT							= _C5_DRIVERS_MAX_QNT
	.SET	C5_TASKS_QNT							= _C5_TASKS_MAX_QNT
	.EQU	_C5_IR_QNT								= 0x25			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE					= 0x20			;Размер очереди к ресурсам(16 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT					= 0x00			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE					= 0x20			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE							= 0x40			;Стек ядра
	.EQU	RAMSTART									= 0x0100
	.EQU	RAMEND									= 0x08ff

	;---IR---
	.EQU	C5_IR_INT0								= 0x01
	.EQU	C5_IR_INT1								= 0x02
	.EQU	C5_IR_INT2								= 0x03
	.EQU	C5_IR_INT3								= 0x04
	.EQU	C5_IR_INT4								= 0x05
	.EQU	C5_IR_INT5								= 0x06
	.EQU	C5_IR_INT6								= 0x07
	.EQU	C5_IR_INT7								= 0x08
	.EQU	C5_IR_TIMER2_COMP						= 0x09
	.EQU	C5_IR_TIMER2_OVF						= 0x0a
	.EQU	C5_IR_TIMER1_CAPT						= 0x0b
	.EQU	C5_IR_RESERVED_CORE_TIMER1			= 0x0c
	.EQU	C5_IR_RESERVED_CORE_TIMER2			= 0x0d
	.EQU	C5_IR_TIMER1_COMPC					= 0x0e
	.EQU	C5_IR_TIMER1_OVF						= 0x0f
	.EQU	C5_IR_TIMER0_COMP						= 0x10
	.EQU	C5_IR_TIMER0_OVF						= 0x11
	.EQU	C5_IR_CANIT								= 0x12
	.EQU	C5_IR_OVRIT								= 0x13
	.EQU	C5_IR_SPI_STC							= 0x14
	.EQU	C5_IR_UART0_RX							= 0x15
	.EQU	C5_IR_UART0_UDRE						= 0x16
	.EQU	C5_IR_UART0_TX							= 0x17
	.EQU	C5_IR_ANALOG_COMP						= 0x18
	.EQU	C5_IR_ADC								= 0x19
	.EQU	C5_IR_EE_READY							= 0x1a
	.EQU	C5_IR_TIMER3_CAPT						= 0x1b
	.EQU	C5_IR_TIMER3_COMPA					= 0x1c
	.EQU	C5_IR_TIMER3_COMPB					= 0x1d
	.EQU	C5_IR_TIMER3_COMPC					= 0x1e
	.EQU	C5_IR_TIMER3_OVF						= 0x1f
	.EQU	C5_IR_UART1_RX							= 0x20
	.EQU	C5_IR_UART1_UDRE						= 0x21
	.EQU	C5_IR_UART1_TX							= 0x22
	.EQU	C5_IR_TWI								= 0x23
	.EQU	C5_IR_SPM_READY						= 0x24

	;SBI/CBI/SBIS/SBIC port defines
	.EQU	PORTA										= 0x02
	.EQU	DDRA										= 0x01
	.EQU	PINA										= 0x00
	.EQU	PORTB										= 0x05
	.EQU	DDRB										= 0x04
	.EQU	PINB										= 0x03
	.EQU	PORTC										= 0x08
	.EQU	DDRC										= 0x07
	.EQU	PINC										= 0x06
	.EQU	PORTD										= 0x0B
	.EQU	DDRD										= 0x0A
	.EQU	PIND										= 0x09
	.EQU	PORTE										= 0x0E
	.EQU	DDRE										= 0x0D
	.EQU	PINE										= 0x0C
	.EQU	PORTF										= 0x11
	.EQU	DDRF										= 0x10
	.EQU	PINF										= 0x0F
	.EQU	PORTG										= 0x14
	.EQU	DDRG										= 0x13
	.EQU	PING										= 0x12

	.EQU	SDA										= PD1
	.EQU	SCL										= PD0
	.EQU	RXD										= PE0
	.EQU	TXD										= PE1
	.EQU	RXD0										= RXD
	.EQU	TXD0										= TXD
	.EQU	RXD1										= PD2
	.EQU	TXD1										= PD3
	.EQU	SCK										= PB1
	.EQU	MISO										= PB3
	.EQU	MOSI										= PB2
	.EQU	SS											= PB0

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR
	;---TWI---
	.EQU	TWCR										= 0xbc
	.EQU		TWINT									= 0x07
	.EQU		TWEA									= 0x06
	.EQU		TWSTA									= 0x05
	.EQU		TWSTO									= 0x04
	.EQU		TWWC									= 0x03
	.EQU		TWEN									= 0x02
	.EQU		TWIE									= 0x00
	.EQU	TWDR										= 0xbb
	.EQU	TWAR										= 0xba
	.EQU		TWGCE									= 0x00
	.EQU	TWSR										= 0xb9
	.EQU		TWPS1									= 0x01
	.EQU		TWPS0									= 0x00
	.EQU	TWBR										= 0xb8
	;---TIMER0---
	.EQU	TIMSK0									= 0x6e
	.EQU		OCIE0A								= 0x01
	.EQU		TOIE0									= 0x00
	.EQU	TCCR0A									= 0x44
	.EQU		FOC0A									= 0x07
	.EQU		WGM00									= 0x06
	.EQU		COM0A1								= 0x05
	.EQU		COM0A0								= 0x04
	.EQU		WGM01									= 0x03
	.EQU		CS02									= 0x02
	.EQU		CS01									= 0x01
	.EQU		CS00									= 0x00
	.EQU	TCNT0										= 0x46
	.EQU	OCR0A										= 0x47
	.EQU	TIFR0										= 0x35
	.EQU		OCF0A									= 0x01
	.EQU		TOV0									= 0x00
	;---TIMER1---
	.EQU	TIMSK1									= 0x6f
	.EQU		ICIE1									= 0x05
	.EQU		OCIE1C								= 0x03
	.EQU		OCIE1B								= 0x02
	.EQU		OCIE1A								= 0x01
	.EQU		TOIE1									= 0x00
	.EQU	TCCR1A									= 0x80
	.EQU		COM1A1								= 0x07
	.EQU		COM1A0								= 0x06
	.EQU		COM1B1								= 0x05
	.EQU		COM1B0								= 0x04
	.EQU		COM1C1								= 0x03
	.EQU		COM1C0								= 0x02
	.EQU		WGM11									= 0x01
	.EQU		WGM10									= 0x00
	.EQU	TCCR1B									= 0x81
	.EQU		ICNC1									= 0x07
	.EQU		ICES1									= 0x06
	.EQU		WGM13									= 0x04
	.EQU		WGM12									= 0x03
	.EQU		CS12									= 0x02
	.EQU		CS11									= 0x01
	.EQU		CS10									= 0x00
	.EQU	TCCR1C									= 0x82
	.EQU		FOC1A									= 0x07
	.EQU		FOC1B									= 0x06
	.EQU		FOC1C									= 0x05
	.EQU	TCNT1H									= 0x85
	.EQU	TCNT1L									= 0x84
	.EQU	OCR1AH									= 0x89
	.EQU	OCR1AL									= 0x88
	.EQU	OCR1BH									= 0x8b
	.EQU	OCR1BL									= 0x8a
	.EQU	OCR1CH									= 0x8d
	.EQU	OCR1CL									= 0x8c
	.EQU	TIFR1										= 0x36
	.EQU		ICF1									= 0x05
	.EQU		OCF1C									= 0x03
	.EQU		OCF1B									= 0x02
	.EQU		OCF1A									= 0x01
	.EQU		TOV1									= 0x00
	;---TIMER2---
	.EQU	TIMSK2									= 0x70
	.EQU		OCIE2A								= 0x01
	.EQU		TOIE2									= 0x00
	.EQU	TCCR2A									= 0xb0
	.EQU		FOC2A									= 0x07
	.EQU		WGM20									= 0x06
	.EQU		COM2A1								= 0x05
	.EQU		COM2A0								= 0x04
	.EQU		WGM21									= 0x03
	.EQU		CS02									= 0x02
	.EQU		CS01									= 0x01
	.EQU		CS00									= 0x00
	.EQU	TCNT2										= 0xb2
	.EQU	OCR2A										= 0xb3
	.EQU	TIFR2										= 0x37
	.EQU		OCF2A									= 0x01
	.EQU		TOV2									= 0x00
	;---TIMER3---
	.EQU	TIMSK3									= 0x71
	.EQU		ICIE3									= 0x05
	.EQU		OCIE3C								= 0x03
	.EQU		OCIE3B								= 0x02
	.EQU		OCIE3A								= 0x01
	.EQU		TOIE3									= 0x00
	.EQU	TCCR3A									= 0x90
	.EQU		COM3A1								= 0x07
	.EQU		COM3A0								= 0x06
	.EQU		COM3B1								= 0x05
	.EQU		COM3B0								= 0x04
	.EQU		COM3C1								= 0x03
	.EQU		COM3C0								= 0x02
	.EQU		WGM31									= 0x01
	.EQU		WGM30									= 0x00
	.EQU	TCCR3B									= 0x91
	.EQU		ICNC3									= 0x07
	.EQU		ICES3									= 0x06
	.EQU		WGM33									= 0x04
	.EQU		WGM32									= 0x03
	.EQU		CS32									= 0x02
	.EQU		CS31									= 0x01
	.EQU		CS30									= 0x00
	.EQU	TCCR3C									= 0x92
	.EQU		FOC3A									= 0x07
	.EQU		FOC3B									= 0x06
	.EQU		FOC3C									= 0x05
	.EQU	TCNT3H									= 0x95
	.EQU	TCNT3L									= 0x94
	.EQU	OCR3AH									= 0x99
	.EQU	OCR3AL									= 0x98
	.EQU	OCR3BH									= 0x9b
	.EQU	OCR3BL									= 0x9a
	.EQU	OCR3CH									= 0x9d
	.EQU	OCR3CL									= 0x9c
	.EQU	TIFR3										= 0x38
	.EQU		ICF3									= 0x05
	.EQU		OCF3C									= 0x03
	.EQU		OCF3B									= 0x02
	.EQU		OCF3A									= 0x01
	.EQU		TOV3									= 0x00
	;---UART0---
	.EQU	UDR0										= 0xc6
	.EQU	UCSR0A									= 0xc0
	.EQU		RXC0									= 0x07
	.EQU		TXC0									= 0x06
	.EQU		UDRE0									= 0x05
	.EQU		FE0									= 0x04
	.EQU		DOR0									= 0x03
	.EQU		UPE0									= 0x02
	.EQU		U2X0									= 0x01
	.EQU		MPCM0									= 0x00
	.EQU	UCSR0B									= 0xc1
	.EQU		RXCIE0								= 0x07
	.EQU		TXCIE0								= 0x06
	.EQU		UDRIE0								= 0x05
	.EQU		RXEN0									= 0x04
	.EQU		TXEN0									= 0x03
	.EQU		UCSZ02								= 0x02
	.EQU		RXB80									= 0x01
	.EQU		TXB80									= 0x00
	.EQU	UCSR0C									= 0xc2
	.EQU		UMSEL0								= 0x06
	.EQU		UPM01									= 0x05
	.EQU		UPM00									= 0x04
	.EQU		USBS0									= 0x03
	.EQU		UCSZ01								= 0x02
	.EQU		UCSZ00								= 0x01
	.EQU		UCPOL0								= 0x00
	.EQU	UBRR0H									= 0xc5
	.EQU	UBRR0L									= 0xc4
	;---UART1---
	.EQU	UDR1										= 0xce
	.EQU	UCSR1A									= 0xc8
	.EQU		RXC1									= 0x07
	.EQU		TXC1									= 0x06
	.EQU		UDRE1									= 0x05
	.EQU		FE1									= 0x04
	.EQU		DOR1									= 0x03
	.EQU		UPE1									= 0x02
	.EQU		U2X1									= 0x01
	.EQU		MPCM1									= 0x00
	.EQU	UCSR1B									= 0xc9
	.EQU		RXCIE1								= 0x07
	.EQU		TXCIE1								= 0x06
	.EQU		UDRIE1								= 0x05
	.EQU		RXEN1									= 0x04
	.EQU		TXEN1									= 0x03
	.EQU		UCSZ12								= 0x02
	.EQU		RXB81									= 0x01
	.EQU		TXB81									= 0x00
	.EQU	UCSR1C									= 0xca
	.EQU		UMSEL1								= 0x06
	.EQU		UPM11									= 0x05
	.EQU		UPM10									= 0x04
	.EQU		USBS1									= 0x03
	.EQU		UCSZ11								= 0x02
	.EQU		UCSZ10								= 0x01
	.EQU		UCPOL1								= 0x00
	.EQU	UBRR1H									= 0xcd
	.EQU	UBRR1L									= 0xcc
	;---INT---
	.EQU	EICRA										= 0x69
	.EQU		ISC31									= 0x07
	.EQU		ISC30									= 0x06
	.EQU		ISC21									= 0x05
	.EQU		ISC20									= 0x04
	.EQU		ISC11									= 0x03
	.EQU		ISC10									= 0x02
	.EQU		ISC01									= 0x01
	.EQU		ISC00									= 0x00
	.EQU	EICRB										= 0x6a
	.EQU		ISC71									= 0x07
	.EQU		ISC70									= 0x06
	.EQU		ISC61									= 0x05
	.EQU		ISC60									= 0x04
	.EQU		ISC51									= 0x03
	.EQU		ISC50									= 0x02
	.EQU		ISC41									= 0x01
	.EQU		ISC40									= 0x00
	.EQU	EIMSK										= 0x3d
	.EQU		INT7									= 0x07
	.EQU		INT6									= 0x06
	.EQU		INT5									= 0x05
	.EQU		INT4									= 0x04
	.EQU		INT3									= 0x03
	.EQU		INT2									= 0x02
	.EQU		INT1									= 0x01
	.EQU		INT0									= 0x00
	.EQU	EIFR										= 0x3c
	.EQU		INTF7									= 0x07
	.EQU		INTF6									= 0x06
	.EQU		INTF5									= 0x05
	.EQU		INTF4									= 0x04
	.EQU		INTF3									= 0x03
	.EQU		INTF2									= 0x02
	.EQU		INTF1									= 0x01
	.EQU		INTF0									= 0x00
	;---EEPROM---
	.EQU	EEARH										= 0x42
	.EQU	EEARL										= 0x41
	.EQU	EEDR										= 0x40
	.EQU	EECR										= 0x3F
	.EQU		EERIE									= 0x03
	.EQU		EEMPE									= 0x02
	.EQU		EEMWE									= EEMPE
	.EQU		EEPE									= 0x01
	.EQU		EEWE									= EEPE
	.EQU		EERE									= 0x00
	;---ADC---
	.EQU	ADMUX										= 0x7c
	.EQU		REFS1									= 0x07
	.EQU		REFS0									= 0x06
	.EQU		ADLAR									= 0x05
	.EQU		MUX3									= 0x03
	.EQU		MUX2									= 0x02
	.EQU		MUX1									= 0x01
	.EQU		MUX0									= 0x00
	.EQU	ADCSRA									= 0x7a
	.EQU		ADEN									= 0x07
	.EQU		ADSC									= 0x06
	.EQU		ADATE									= 0x05
	.EQU		ADIF									= 0x04
	.EQU		ADIE									= 0x03
	.EQU		ADPS2									= 0x02
	.EQU		ADPS1									= 0x01
	.EQU		ADPS0									= 0x00
	.EQU	ADCSRB									= 0x7b
	.EQU		ACME									= 0x06
	.EQU		ADTS2									= 0x02
	.EQU		ADTS1									= 0x01
	.EQU		ADTS0									= 0x00
	.EQU	ADCL										= 0x78
	.EQU	ADCH										= 0x79
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
	.EQU	ADC_VREF_AREF							= (0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AVCC							= (0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56_CAP						= (1<<REFS1)|(1<<REFS0)
	;---ADC-PRESCALLER---
	.EQU	ADC_PRESC_X1							= 0x00	;Нет x1, есть два x2, ошибка в даташите?
	.EQU	ADC_PRESC_X2							= 0x01
	.EQU	ADC_PRESC_X4							= 0x02
	.EQU	ADC_PRESC_X8							= 0x03
	.EQU	ADC_PRESC_X16							= 0x04
	.EQU	ADC_PRESC_X32							= 0x05
	.EQU	ADC_PRESC_X64							= 0x06
	.EQU	ADC_PRESC_X128							= 0x07

	.SET	INT0_PORT								= PD0
	.SET	INT1_PORT								= PD1
	.SET	INT2_PORT								= PD2
	.SET	INT3_PORT								= PD3
	.SET	INT4_PORT								= PE4
	.SET	INT5_PORT								= PE5
	.SET	INT6_PORT								= PE6
	.SET	INT7_PORT								= PE7

.ORG 0x0000
	JMP	MAIN															; Reset Handler
	CALL	_C5_IR														; External Interrupt0 Handler
	CALL	_C5_IR														; External Interrupt1 Handler
	CALL	_C5_IR														; External Interrupt2 Handler
	CALL	_C5_IR														; External Interrupt3 Handler
	CALL	_C5_IR														; External Interrupt4 Handler
	CALL	_C5_IR														; External Interrupt5 Handler
	CALL	_C5_IR														; External Interrupt6 Handler
	CALL	_C5_IR														; External Interrupt7 Handler
	CALL	_C5_IR														; Timer/Counter2 Compare Match
	CALL	_C5_IR														; Timer/Counter2 Overflow
	CALL	_C5_IR														; Timer/Counter1 Capture Event
	JMP	_C5_TIMER_A_IR												; Timer/Counter1 Compare Match A
	JMP	_C5_TIMER_B_IR												; Timer/Counter1 Compare Match B
	CALL	_C5_IR														; Timer/Counter1 Compare Match C
	CALL	_C5_IR														; Timer/Counter1 Overflow
	CALL	_C5_IR														; Timer/Counter0 Compare Match
	CALL	_C5_IR														; Timer/Counter0 Overflow
	CALL	_C5_IR														; CAN Transfer Complete or Error
	CALL	_C5_IR														; CAN Timer Overrun
	CALL	_C5_IR														; SPI Serial Transfer Complete
	CALL	_C5_IR														; USART0, Rx Complete
	CALL	_C5_IR														; USART0, Data Register Empty
	CALL	_C5_IR														; USART0, Tx Complete
	CALL	_C5_IR														; Analog Comparator
	CALL	_C5_IR														; ADC Conversion Complete
	CALL	_C5_IR														; EEPROM Ready
	CALL	_C5_IR														; Timer/Counter3 Capture Event
	CALL	_C5_IR														; Timer/Counter3 Compare Match A
	CALL	_C5_IR														; Timer/Counter3 Compare Match B
	CALL	_C5_IR														; Timer/Counter3 Compare Match C
	CALL	_C5_IR														; Timer/Counter3 Overflow
	CALL	_C5_IR														; USART1, Rx Complete
	CALL	_C5_IR														; USART1, Data Register Empty
	CALL	_C5_IR														; USART1, Tx Complete
	CALL	_C5_IR														; 2-wire Serial Interface
	CALL	_C5_IR														; Store Program Memory Ready


PORTS_TABLE:															;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA,PORTB,DDRB,PINB и т.д.
	.db	0x22,0x21,0x20,0x25,0x24,0x23,0x28,0x27,0x26,0x2b,0x2a,0x29,0x2e,0x2d,0x2c,0x31,0x30,0x2f,0x34,0x33,0x32
PCINT_TABLE:
	.db	0x00,0x00,0x00,0x00,0x00,0x00,0x00
INT_TABLE:
	.db	(1<<INT0),(1<<INT1),(1<<INT2),(1<<INT3),(1<<INT4),(1<<INT5),(1<<INT6),(1<<INT7)

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
	STS TIFR1,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_START
	LDS TEMP,TIMSK1
	ORI TEMP,(1<<OCIE1A)
	STS TIMSK1,TEMP
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
	STS TIFR1,TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP
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
	LDS TEMP,TIFR1
	ORI TEMP,(1<<OCF1B)
	STS TIFR1,TEMP
	;Установка флага
	LDI TEMP,(1<<_CFL_TIMER_B_USE)
	OR _C5_COREFLAGS,TEMP
	;Запуск таймера
	LDS TEMP,TIMSK1
	ORI TEMP,(1<<OCIE1B)
	STS TIMSK1,TEMP
	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP,TIMSK1
	ANDI TEMP,~(1<<OCIE1B)
	STS TIMSK1,TEMP
.ENDMACRO

.MACRO _C5_POWER_ON
.ENDMACRO
.MACRO _C5_POWER_OFF
.ENDMACRO

.MACRO _C5_TIMER_TCNT
	LDS @0,TCNT1L
.ENDMACRO
