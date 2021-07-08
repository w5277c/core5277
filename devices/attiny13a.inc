;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.05.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Ядро не применимо
;-----------------------------------------------------------------------------------------------------------------------
;TODO обновить из atmega328

;---DESCRIPTION------------------------------------------
;ATtiny13a (1KB FLASH, 64B EEPROM, 64B SRAM) at 16MHz

	;---SPECIFY-DEVICE---
	.DEVICE ATtiny13a

	#pragma AVRPART MEMORY PROG_FLASH 1024
	#pragma AVRPART MEMORY EEPROM 64
	#pragma AVRPART MEMORY INT_SRAM SIZE 64
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60

	.include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATTINY13A
	.MESSAGE "######## DEVICE ATtiny13A"
	.EQU	RAMSTART									= 0x0060
	.EQU	RAMEND									= 0x009f

	;SBI/CBI/SBIS/SBIC port defines
	.EQU	PORTA										= 0xff
	.EQU	DDRA										= 0xff
	.EQU	PINA										= 0xff
	.EQU	PORTB										= 0x18
	.EQU	DDRB										= 0x17
	.EQU	PINB										= 0x16
	.EQU	PORTC										= 0xff
	.EQU	DDRC										= 0xff
	.EQU	PINC										= 0xff
	.EQU	PORTD										= 0xff
	.EQU	DDRD										= 0xff
	.EQU	PIND										= 0xff
	.EQU	PORTE										= 0xff
	.EQU	DDRE										= 0xff
	.EQU	PINE										= 0xff
	.EQU	PORTF										= 0xff
	.EQU	DDRF										= 0xff
	.EQU	PINF										= 0xff
	.EQU	PORTG										= 0xff
	.EQU	DDRG										= 0xff
	.EQU	PING										= 0xff

	.EQU	SDA										= 0xff
	.EQU	SCL										= 0xff
	.EQU	RXD										= 0xff
	.EQU	TXD										= 0xff
	.EQU	RXD0										= RXD
	.EQU	TXD0										= TXD
	.EQU	SCK										= PB2
	.EQU	MISO										= PB1
	.EQU	MOSI										= PB0
	.EQU	SS											= PB3	;не описан в документации

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR
	.EQU		WDRF									= 0x03
	.EQU		BORF									= 0x02
	.EQU		EXTRF									= 0x01
	.EQU		PORF									= 0x00

	.EQU	PRR										= 0x45
	.EQU		PRTIM0								= 0x01
	.EQU		PRADC									= 0x00
	;---TIMERS---
	.EQU	TIMSK										= 0x59
	.EQU		OCIE0B								= 0x03
	.EQU		OCIE0A								= 0x02
	.EQU		TOIE0									= 0x01
	.EQU	TIFR										= 0x58
	.EQU		OCF0B									= 0x03
	.EQU		OCF0A									= 0x02
	.EQU		TOV0									= 0x01
	;---TIMER0---
	.EQU	GTCCR										= 0x48
	.EQU		TSM									= 0x07
	.EQU		PSR0									= 0x00
	.EQU	TCCR0A									= 0x4F
	.EQU		COM0A1								= 0x07
	.EQU		COM0A0								= 0x06
	.EQU		COM0B1								= 0x05
	.EQU		COM0B0								= 0x04
	.EQU		WGM01									= 0x01
	.EQU		WGM00									= 0x00
	.EQU	TCCR0B									= 0x53
	.EQU		FOC0A									= 0x07
	.EQU		FOC0B									= 0x06
	.EQU		WGM02									= 0x03
	.EQU		CS02									= 0x02
	.EQU		CS01									= 0x01
	.EQU		CS00									= 0x00
	.EQU	TCNT0										= 0x52
	.EQU	OCR0A										= 0x56
	.EQU	OCR0B										= 0x49
	;---PCINT---
	.EQU	PCMSK										= 0x35
	.EQU		PCINT5								= 0x05
	.EQU		PCINT4								= 0x04
	.EQU		PCINT3								= 0x03
	.EQU		PCINT2								= 0x02
	.EQU		PCINT1								= 0x01
	.EQU		PCINT0								= 0x00
	;---INT---
	.EQU	MCUCR										= 0x55
	.EQU		PUD									= 0x06
	.EQU		SE										= 0x05
	.EQU		SM1									= 0x04
	.EQU		SM0									= 0x03
	.EQU		ISC01									= 0x01
	.EQU		ISC00									= 0x00
	.EQU	GIMSK										= 0x5b
	.EQU		INT0									= 0x06
	.EQU		PCIE									= 0x05
	.EQU	GIFR										= 0x5a
	.EQU		INTF0									= 0x06
	.EQU		PCIF									= 0x05
	.EQU	EICRA										= MCUCR
	.EQU	EIMSK										= GIMSK
	.EQU	EIFR										= GIFR

	;---EEPROM---
	.EQU	EEARH										= 0xFF
	.EQU	EEARL										= 0x3E
	.EQU	EEDR										= 0x3D
	.EQU	EECR										= 0x3C
	.EQU		EEPM1									= 0x05
	.EQU		EEPM0									= 0x04
	.EQU		EERIE									= 0x03
	.EQU		EEMPE									= 0x02
	.EQU		EEMWE									= EEMPE
	.EQU		EEPE									= 0x01
	.EQU		EEWE									= EEPE
	.EQU		EERE									= 0x00
	;---ADC---
	.EQU	ADMUX										= 0x27
	.EQU		REFS0									= 0x06
	.EQU		ADLAR									= 0x05
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
	.EQU	ADCSRB									= 0x23
	.EQU		ACME									= 0x06
	.EQU		ADTS2									= 0x02
	.EQU		ADTS1									= 0x01
	.EQU		ADTS0									= 0x00
	.EQU	ADCL										= 0x24
	.EQU	ADCH										= 0x25
	.EQU	ADC0										= 0x00
	.EQU	ADC1										= 0x01
	.EQU	ADC2										= 0x02
	.EQU	ADC3										= 0x03
	.EQU	ADC4										= 0x04

	.EQU	WDTCR										= 0x41
	.EQU		WDTIF									= 0x07
	.EQU		WDTIE									= 0x06
	.EQU		WDP3									= 0x05
	.EQU		WDCE									= 0x04
	.EQU		WDE									= 0x03
	.EQU		WDP2									= 0x02
	.EQU		WDP1									= 0x01
	.EQU		WDP0									= 0x00

	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AVCC							= (0<<REFS0)
	.EQU	ADC_VREF_1_1							= (1<<REFS0)

	.SET	INT0_PORT								= PB1

