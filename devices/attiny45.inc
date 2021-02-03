;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.08.2020  w5277c@gmail.com			Начало
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;19.01.2021  w5277c@gmail.com			Обновлен блок EEPROM
;21.01.2021  w5277c@gmail.com			Дбавлены порты для инструкций SBI,CBI...
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for ATtiny45 (4KB ROM, 256B EEPROM, 256B SRAM) at 16MHz (LITTLE ENDIAN)

;avrdude -p m8 -c avrispmkII -U flash:w:main.hex
;;;;;Ext crystal 8.0-..., 16K CK/14 CK + 4.1ms, CKDIV8 disabled, BOD disabled, SPI enabled:
;;;;;avrdude -p m8 -c avrispmkII -U lfuse:w:0xef:m -U hfuse:w:0xdc:m -U efuse:w:0xf9:m

	;---SPECIFY-DEVICE---
	.DEVICE ATtiny45

	#pragma AVRPART MEMORY PROG_FLASH 4096
	#pragma AVRPART MEMORY EEPROM 256
	#pragma AVRPART MEMORY INT_SRAM SIZE 256
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60

	#include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATTINY25
	.MESSAGE "######## DEVICE ATtiny45"
	.EQU	_C5_DRIVERS_MAX_QNT					= 0x03			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT						= 0x02			;Максимальное количество задач
	.SET	C5_DRIVERS_QNT							= _C5_DRIVERS_MAX_QNT
	.SET	C5_TASKS_QNT							= _C5_TASKS_MAX_QNT
	.EQU	_C5_IR_QNT								= 0x0f			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE					= 0x08			;Размер очереди к ресурсам(8 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT					= 0x01			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE					= 0x00			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE							= 0x28			;Стек ядра - 40B
	.EQU	RAMSTART									= 0x0060
	.EQU	RAMEND									= 0x015f

	;---IR---
	.EQU	C5_IR_INT0								= 0x01
	.EQU	C5_IR_PCINT0							= 0x02
	.EQU	C5_IR_TIMER1_COMPA					= 0x03
	.EQU	C5_IR_TIMER1_OVF						= 0x04
	.EQU	C5_IR_TIMER0_OVF						= 0x05
	.EQU	C5_IR_EE_READY							= 0x06
	.EQU	C5_IR_ANALOG_COMP						= 0x07
	.EQU	C5_IR_ADC								= 0x08
	.EQU	C5_IR_TIMER1_COMPB					= 0x09
	.EQU	C5_IR_RESERVED_CORE_TIMER1			= 0x0a
	.EQU	C5_IR_RESERVED_CORE_TIMER2			= 0x0b
	.EQU	C5_IR_WDT								= 0x0c
	.EQU	C5_IR_USI_START						= 0x0d
	.EQU	C5_IR_USI_OVF							= 0x0e
	;Отсутствуют
	.EQU	C5_IR_PCINT1							= 0x00
	.EQU	C5_IR_PCINT2							= 0x00

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
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR
	.EQU	PRR										= 0x40
	.EQU		PRTIM1								= 0x03
	.EQU		PRTIM0								= 0x02
	.EQU		PRUSI									= 0x01
	.EQU		PRADC									= 0x00
	;---TIMERS---
	.EQU	TIMSK										= 0x59
	.EQU		OCIE1A								= 0x06
	.EQU		OCIE1B								= 0x05
	.EQU		OCIE0A								= 0x04
	.EQU		OCIE0B								= 0x03
	.EQU		TOIE1									= 0x02
	.EQU		TOIE0									= 0x01
	.EQU	TIFR										= 0x58
	.EQU		OCF1A									= 0x06
	.EQU		OCF1B									= 0x05
	.EQU		OCF0A									= 0x04
	.EQU		OCF0B									= 0x03
	.EQU		TOV1									= 0x02
	.EQU		TOV0									= 0x01
	;---TIMER0---
	.EQU	GTCCR										= 0x4C
	.EQU		TSM									= 0x07
	.EQU		PWM1B									= 0x06
	.EQU		COM1B1								= 0x05
	.EQU		COM1B0								= 0x04
	.EQU		FOC1B									= 0x03
	.EQU		FOC1A									= 0x02
	.EQU		PSR1									= 0x01
	.EQU		PSR0									= 0x00
	.EQU	TCCR0A									= 0x4A
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
	.EQU	OCR0A										= 0x49
	.EQU	OCR0B										= 0x48
	;---TIMER1---
	.EQU	TCCR1										= 0x50
	.EQU		CTC1									= 0x07
	.EQU		PWM1A									= 0x06
	.EQU		COM1A1								= 0x05
	.EQU		COM1A0								= 0x04
	.EQU		CS13									= 0x03
	.EQU		CS12									= 0x02
	.EQU		CS11									= 0x01
	.EQU		CS10									= 0x00
	.EQU	TCNT1										= 0x4F
	.EQU	OCR1A										= 0x4E
	.EQU	OCR1B										= 0x4B
	.EQU	OCR1C										= 0x4D
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
	.EQU		BODS									= 0x07
	.EQU		PUD									= 0x06
	.EQU		SE										= 0x05
	.EQU		SM1									= 0x04
	.EQU		SM0									= 0x03
	.EQU		BODSE									= 0x02
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
	.EQU	EEARH										= 0x3F
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
	.EQU		REFS1									= 0x07
	.EQU		REFS0									= 0x06
	.EQU		ADLAR									= 0x05
	.EQU		REFS2									= 0x04
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
	.EQU	ADCSRB									= 0x23
	.EQU		BIN									= 0x07
	.EQU		ACME									= 0x06
	.EQU		IPR									= 0x05
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
	.EQU	ADC5										= 0x05
	.EQU	ADC6										= 0x06
	.EQU	ADC7										= 0x07
	.EQU	ADCT										= 0x08

	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AVCC							= (0<<REFS2)|(0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AREF							= (0<<REFS2)|(0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_1_1							= (0<<REFS2)|(1<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_1_1_CAP						= (0<<REFS2)|(1<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56							= (1<<REFS2)|(1<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_2_56_CAP						= (1<<REFS2)|(1<<REFS1)|(1<<REFS0)

	.SET	INT0_PORT								= PB2

.ORG 0x0000
	RJMP	MAIN															; Reset Handler
	RCALL	_C5_IR														; External Interrupt0 Handler
	RCALL	_C5_IR														; Pin Change Interrupt Request 0
	RCALL	_C5_IR														; Timer/Counter1 Compare Match A
	RCALL	_C5_IR														; Timer/Counter1 Overflow
	RCALL	_C5_IR														; Timer/Counter0 Overflow
	RCALL	_C5_IR														; EEPROM Ready
	RCALL	_C5_IR														; Analog Comparator
	RCALL	_C5_IR														; ADC Conversion Complete
	RCALL	_C5_IR														; Timer/Counter1 Compare Match B
	RJMP	_C5_TIMER_A_IR												; Timer/Counter0 Compare Match A
	RJMP	_C5_TIMER_B_IR												; Timer/Counter0 Compare Match B
	RCALL	_C5_IR														; Watchdog Time-out Interrupt
	RCALL	_C5_IR														; USI start
	RCALL	_C5_IR														; USI overflow

PORTS_TABLE:															;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA,PORTB,DDRB,PINB и т.д.
	.db	0x00,0x00,0x00,0x38,0x37,0x36,0x00,0x00,0x00,0x00,0x00,0x00
PCINT_TABLE:
	.db	0x00,PCMSK,0x00,0x00

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
	LDS TEMP,TIMSK
	ORI TEMP,(1<<OCIE0A)
	STS TIMSK,TEMP
.ENDMACRO

.MACRO _CORRE5277_TIMERA_CORRECTOR
	PUSH ACCUM
	LDS ACCUM,TCNT0
	ADD ACCUM,@0
	STS OCR0A,ACCUM
	LDI ACCUM,(1<<OCF0A)
	STS TIFR0,ACCUM
	POP ACCUM
.ENDMACRO

.MACRO _CORRE5277_IR_OFFSET_CORRECTOR
	NOP
	DEC _C5_TEMP_L												;Получено на базе адреса выхода из процедуры(не входа), т.е. увеличен на 1
.ENDMACRO

.MACRO _C5_TIMERB
	LDS TEMP_L,TCNT0
	ADD TEMP_L,TEMP
	STS OCR0B,TEMP_L
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
	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP,TIMSK
	ANDI TEMP,~(1<<OCIE0B)
	STS TIMSK,TEMP
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
	LDS @0,TCNT0
.ENDMACRO
