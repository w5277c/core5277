;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.05.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for ATMega88 (8Kb ROM, 512b EEPROM, 1Kb SRAM) at 16MHz (LITTLE ENDIAN)
;avrdude -p m88 -c avrispmkII -U flash:w:main.hex
;Ext crystal 8.0-..., 16K CK/14 CK + 4.1ms, CKDIV8 disabled, BOD disabled, SPI enabled:
;avrdude -p m88 -c avrispmkII -U lfuse:w:0xef:m -U hfuse:w:0xdc:m -U efuse:w:0xf9:m

	;---SPECIFY-DEVICE---
	.DEVICE ATmega88

	#pragma AVRPART MEMORY PROG_FLASH 8192
	#pragma AVRPART MEMORY EEPROM 512
	#pragma AVRPART MEMORY INT_SRAM SIZE 1024
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100

	#include "/inc/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATMEGA48
	.MESSAGE "######## DEVICE ATmega88 ########"
	.EQU	_C5_DRIVERS_MAX_QNT			= 0x10			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT				= 0x08			;Максимальное количество задач
	.EQU	_C5_IR_QNT						= 0x1a			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE			= 0x10			;Размер очереди к ресурсам(8 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT			= 0x03			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE			= 0x04			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE					= 0x40			;Стек ядра
	.EQU	RAMEND									= 0x04ff

	;---IR---
	.EQU	C5_IR_INT0						= 0x01
	.EQU	C5_IR_INT1						= 0x02
	.EQU	C5_IR_PCINT0					= 0x03
	.EQU	C5_IR_PCINT1					= 0x04
	.EQU	C5_IR_PCINT2					= 0x05
	.EQU	C5_IR_WDT						= 0x06
	.EQU	C5_IR_TIMER2_COMPA			= 0x07
	.EQU	C5_IR_TIMER2_COMPB			= 0x08
	.EQU	C5_IR_TIMER2_OVF				= 0x09
	.EQU	C5_IR_TIMER1_CAPT				= 0x0a
	.EQU	C5_IR_TIMER1_COMPA			= 0x0b
	.EQU	C5_IR_TIMER1_COMPB			= 0x0c
	.EQU	C5_IR_TIMER1_OVF				= 0x0d
	.EQU	C5_IR_RESERVED_CORE_TIMER	= 0x0e
	.EQU	C5_IR_TIMER0_COMPB			= 0x0f
	.EQU	C5_IR_TIMER0_OVF				= 0x10
	.EQU	C5_IR_SPI						= 0x11
	.EQU	C5_IR_UART_RX					= 0x12
	.EQU	C5_IR_UART_UDRE				= 0x13
	.EQU	C5_IR_UART_TX					= 0x14
	.EQU	C5_IR_ADC						= 0x15
	.EQU	C5_IR_EE_READY					= 0x16
	.EQU	C5_IR_ANALOG_COMP				= 0x17
	.EQU	C5_IR_TWI						= 0x18
	.EQU	C5_IR_SPM_READY				= 0x19

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

	.EQU	SDA										= PC4
	.EQU	SCL										= PC5
	.EQU	RXD										= PD0
	.EQU	TXD										= PD1

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR
	.EQU	PRR										= 0x64
	.EQU		PRTWI									= 0x07
	.EQU		PRTIM2								= 0x06
	.EQU		PRTIM0								= 0x05
	.EQU		PRTIM1								= 0x03
	.EQU		PRSPI									= 0x02
	.EQU		PRUSART0								= 0x01
	.EQU		PRADC									= 0x00
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
	.EQU		OCIE0B								= 0x02
	.EQU		OCIE0A								= 0x01
	.EQU		TOIE0									= 0x00
	.EQU	TCCR0A									= 0x44
	.EQU		COM0A1								= 0x07
	.EQU		COM0A0								= 0x06
	.EQU		COM0B1								= 0x05
	.EQU		COM0B0								= 0x04
	.EQU		WGM01									= 0x01
	.EQU		WGM00									= 0x00
	.EQU	TCCR0B									= 0x45
	.EQU		FOC0A									= 0x07
	.EQU		FOC0B									= 0x06
	.EQU		WGM02									= 0x03
	.EQU		CS02									= 0x02
	.EQU		CS01									= 0x01
	.EQU		CS00									= 0x00
	.EQU	TCNT0										= 0x46
	.EQU	OCR0A										= 0x47
	.EQU	OCR0B										= 0x48
	.EQU	TIFR0										= 0x35
	.EQU		OCF0B									= 0x02
	.EQU		OCF0A									= 0x01
	.EQU		TOV0									= 0x00
	;---TIMER1---
	.EQU	TIMSK1									= 0x6f
	.EQU		ICIE1									= 0x05
	.EQU		OCIE1B								= 0x02
	.EQU		OCIE1A								= 0x01
	.EQU		TOIE1									= 0x00
	.EQU	TCCR1A									= 0x80
	.EQU		COM1A1								= 0x07
	.EQU		COM1A0								= 0x06
	.EQU		COM1B1								= 0x05
	.EQU		COM1B0								= 0x04
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
	.EQU	TCNT1H									= 0x85
	.EQU	TCNT1L									= 0x84
	.EQU	OCR1AH									= 0x89
	.EQU	OCR1AL									= 0x88
	.EQU	OCR1BH									= 0x8b
	.EQU	OCR1BL									= 0x8a
	.EQU	TIFR1										= 0x36
	.EQU		ICF1									= 0x05
	.EQU		OCF1B									= 0x02
	.EQU		OCF1A									= 0x01
	.EQU		TOV1									= 0x00
	;---TIMER2---
	.EQU	TIMSK2									= 0x70
	.EQU		OCIE2B								= 0x02
	.EQU		OCIE2A								= 0x01
	.EQU		TOIE2									= 0x00
	.EQU	TCCR2A									= 0xb0
	.EQU		COM2A1								= 0x07
	.EQU		COM2A0								= 0x06
	.EQU		COM2B1								= 0x05
	.EQU		COM2B0								= 0x04
	.EQU		WGM21									= 0x01
	.EQU		WGM20									= 0x00
	.EQU	TCCR2B									= 0xb1
	.EQU		FOC2A									= 0x07
	.EQU		FOC2B									= 0x06
	.EQU		WGM22									= 0x03
	.EQU		CS22									= 0x02
	.EQU		CS21									= 0x01
	.EQU		CS20									= 0x00
	.EQU	TCNT2										= 0xb2
	.EQU	OCR2A										= 0xb3
	.EQU	OCR2B										= 0xb4
	.EQU	TIFR2										= 0x37
	.EQU		OCF2B									= 0x02
	.EQU		OCF2A									= 0x01
	.EQU		TOV2									= 0x00
	;---UART---
	.EQU	UDR										= 0xc6
	.EQU	UCSRA										= 0xc0
	.EQU		RXC									= 0x07
	.EQU		TXC									= 0x06
	.EQU		UDRE									= 0x05
	.EQU		FE										= 0x04
	.EQU		DOR									= 0x03
	.EQU		UPE									= 0x02
	.EQU		U2X									= 0x01
	.EQU		MPCM									= 0x00
	.EQU	UCSRB										= 0xc1
	.EQU		RXCIE									= 0x07
	.EQU		TXCIE									= 0x06
	.EQU		UDRIE									= 0x05
	.EQU		RXEN									= 0x04
	.EQU		TXEN									= 0x03
	.EQU		UCSZ2									= 0x02
	.EQU		RXB8									= 0x01
	.EQU		TXB8									= 0x00
	.EQU	UCSRC										= 0xc2
	.EQU		UMSEL1								= 0x07
	.EQU		UMSEL0								= 0x06
	.EQU		UPM1									= 0x05
	.EQU		UPM0									= 0x04
	.EQU		USBS									= 0x03
	.EQU		UCSZ1									= 0x02
	.EQU		UDCRD									= 0x02
	.EQU		UCSZ0									= 0x01
	.EQU		UCPHA									= 0x01
	.EQU		UCPOL									= 0x00
	.EQU		URSEL									= 0x08			;Для совместимости
	.EQU	UBRRH										= 0xc5
	.EQU	UBRRL										= 0xc4
	;---
	.EQU	UDR0										= UDR
	.EQU	UCSR0A									= UCSRA
	.EQU		RXC0									= RXC
	.EQU		TXC0									= TXC
	.EQU		UDRE0									= UDRE
	.EQU		FE0									= FE
	.EQU		DOR0									= DOR
	.EQU		UPE0									= UPE
	.EQU		U2X0									= U2X
	.EQU		MPCM0									= MPCM
	.EQU	UCSR0B									= UCSRB
	.EQU		RXCIE0								= RXCIE
	.EQU		TXCIE0								= TXCIE
	.EQU		UDRIE0								= UDRIE
	.EQU		RXEN0									= RXEN
	.EQU		TXEN0									= TXEN
	.EQU		UCSZ02								= UCSZ2
	.EQU		RXB80									= RXB8
	.EQU		TXB80									= TXB8
	.EQU	UCSR0C									= UCSRC
	.EQU		UMSEL01								= UMSEL1
	.EQU		UMSEL00								= UMSEL0
	.EQU		UPM01									= UPM1
	.EQU		UPM00									= UPM0
	.EQU		USBS0									= USBS
	.EQU		UCSZ01								= UCSZ1
	.EQU		UDCRD0								= UDCRD
	.EQU		UCSZ00								= UCSZ0
	.EQU		UCPHA0								= UCPHA
	.EQU		UCPOL0								= UCPOL
	.EQU	UBRR0H									= UBRRH
	.EQU	UBRR0L									= UBRRL
	;---PCINT---
	.EQU	PCICR										= 0x68
	.EQU		PCIE2									= 0x02
	.EQU		PCIE1									= 0x01
	.EQU		PCIE0									= 0x00
	.EQU	PCIFR										= 0x3b
	.EQU		PCIF2									= 0x02
	.EQU		PCIF1									= 0x02
	.EQU		PCIF0									= 0x00
	.EQU	PCMSK0									= 0x6b
	.EQU		PCINT7								= 0x07
	.EQU		PCINT6								= 0x06
	.EQU		PCINT5								= 0x05
	.EQU		PCINT4								= 0x04
	.EQU		PCINT3								= 0x03
	.EQU		PCINT2								= 0x02
	.EQU		PCINT1								= 0x01
	.EQU		PCINT0								= 0x00
	.EQU	PCMSK1									= 0x6c
	.EQU		PCINT14								= 0x06
	.EQU		PCINT13								= 0x05
	.EQU		PCINT12								= 0x04
	.EQU		PCINT11								= 0x03
	.EQU		PCINT10								= 0x02
	.EQU		PCINT9								= 0x01
	.EQU		PCINT8								= 0x00
	.EQU	PCMSK2									= 0x6d
	.EQU		PCINT23								= 0x07
	.EQU		PCINT22								= 0x06
	.EQU		PCINT21								= 0x05
	.EQU		PCINT20								= 0x04
	.EQU		PCINT19								= 0x03
	.EQU		PCINT18								= 0x02
	.EQU		PCINT17								= 0x01
	.EQU		PCINT16								= 0x00
	;---EEPROM---
	.EQU	EEARH										= 0x42
	.EQU	EEARL										= 0x41
	.EQU	EEDR										= 0x40
	.EQU	EECR										= 0x3F
	.EQU		EEPM1									= 0x05
	.EQU		EEPM0									= 0x04
	.EQU		EERIE									= 0x03
	.EQU		EEMPE									= 0x02
	.EQU		EEPE									= 0x01
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
	.EQU	ADC_VREF_AREF						= (0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AVCC						= (0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_1_1_CAP					= (1<<REFS1)|(1<<REFS0)

.ORG 0x0000
	RJMP	MAIN															; Reset Handler
	RCALL	_C5_IR												; External Interrupt0 Handler
	RCALL	_C5_IR												; External Interrupt1 Handler
	RCALL	_C5_IR												; Pin Change Interrupt Request 0
	RCALL	_C5_IR												; Pin Change Interrupt Request 1
	RCALL	_C5_IR												; Pin Change Interrupt Request 2
	RCALL	_C5_IR												; Watchdog Time-out Interrupt
	RCALL	_C5_IR												; Timer/Counter2 Compare Match A
	RCALL	_C5_IR												; Timer/Counter2 Compare Match B
	RCALL	_C5_IR												; Timer/Counter2 Overflow
	RCALL	_C5_IR												; Timer/Counter1 Capture Event
	RCALL	_C5_IR												; Timer/Counter1 Compare Match A
	RCALL	_C5_IR												; Timer/Counter1 Compare Match B
	RCALL	_C5_IR												; Timer/Counter1 Overflow
	RJMP	_C5_TIMER_A_IR										; Timer/Counter0 Compare Match A
	RCALL	_C5_TIMER_B_IR										; Timer/Counter0 Compare Match B
	RCALL	_C5_IR												; Timer/Counter0 Overflow
	RCALL	_C5_IR												; SPI Serial Transfer Complete
	RCALL	_C5_IR												; USART Rx Complete
	RCALL	_C5_IR												; USART, Data Register Empty
	RCALL	_C5_IR												; USART, Tx Complete
	RCALL	_C5_IR												; ADC Conversion Complete
	RCALL	_C5_IR												; EEPROM Ready
	RCALL	_C5_IR												; Analog Comparator
	RCALL	_C5_IR												; 2-wire Serial Interface
	RCALL	_C5_IR												; Store Program Memory Ready

_C5_PORTS_TABLE:												;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA, и т.д.
	.db	0x00,0x00,0x00,0x25,0x24,0x23,0x28,0x27,0x26,0x2b,0x2a,0x29
_C5_PCINT_TABLE:
	.db	0x00,PCMSK0,PCMSK1,PCMSK2


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
	STS TIFR0,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_START
	LDS TEMP,TIMSK0
	ORI TEMP,(1<<OCIE0A)
	STS TIMSK0,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_CORRECTOR
	LDS TEMP,TCNT0
	SBRS _C5_COREFLAGS,_CFL_TIMER_UART_CORRECTION
	SUBI TEMP,low(0x100-(TIMERS_SPEED-0x02))					;Подобрано вручную, было 0x64
	SBRC _C5_COREFLAGS,_CFL_TIMER_UART_CORRECTION
	SUBI TEMP,low(0x100-(TIMERS_SPEED+0x02))
	STS OCR0A,TEMP
.ENDMACRO

.MACRO _CORRE5277_IR_OFFSET_CORRECTOR
	NOP
	DEC _C5_TEMP_L												;Получено на базе адреса выхода из процедуры(не входа), т.е. увеличен на 1
.ENDMACRO

.MACRO _C5_TIMERB
	;SUBI TEMP,0x08														;Подобрано вручную, не должно быть вообще
	LDS TEMP_L,TCNT0
	ADD TEMP_L,TEMP
	STS OCR0B,TEMP_L
	LDS TEMP,TIFR0
	ORI TEMP,(1<<OCF0B)
	STS TIFR0,TEMP
	;Установка флага
	LDI TEMP,(1<<_CFL_TIMER_B_USE)
	OR _C5_COREFLAGS,TEMP
	;Запуск таймера
	LDS TEMP,TIMSK0
	ORI TEMP,(1<<OCIE0B)
	STS TIMSK0,TEMP
	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP,TIMSK0
	ANDI TEMP,~(1<<OCIE0B)
	STS TIMSK0,TEMP
.ENDMACRO

.MACRO _C5_POWER_ON
	PUSH TEMP
	LDS TEMP,PRR
	ANDI TEMP,low(~(1<<@0))
	STS PRR,TEMP
	POP TEMP
.ENDMACRO

.MACRO _C5_POWER_OFF
	PUSH TEMP
	LDS TEMP,PRR
	ORI TEMP,low(1<<@0)
	STS PRR,TEMP
	POP TEMP
.ENDMACRO

.MACRO _C5_TIMER_TCNT
	LDS TEMP,TCNT1L
.ENDMACRO

