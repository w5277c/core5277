;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.05.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;19.01.2021	w5277c@gmail.com			Обновлен блок EEPROM
;21.01.2021	w5277c@gmail.com			Добавлены порты для инструкций SBI,CBI...
;05.06.2021	w5277c@gmail.com			Добавлены регистры SPI
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

	.SET	BOOT_512B_ADDR							= 0x1e00
	.SET	BOOT_1024B_ADDR						= 0x1c00
	.SET 	FLASH_PAGESIZE							= 0x80			;128 байт - размер страницы FLASH
	.SET	EEPROM_SIZE								= 0x200

	#include "/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATMEGA16
	.MESSAGE "######## DEVICE ATmega16"
	.SET	_IV_STEP									= 0x02;			;Шаг между адресами переходов на обработчики прерываний
	.EQU	_C5_DRIVERS_MAX_QNT					= 0x10			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT						= 0x08			;Максимальное количество задач
	.SET	C5_DRIVERS_QNT							= _C5_DRIVERS_MAX_QNT
	.SET	C5_TASKS_QNT							= _C5_TASKS_MAX_QNT
	.EQU	_C5_IR_QNT								= 0x15			;Количество перырваний(без RESET)
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
	.EQU	C5_IR_SPI								= 0x0a
	.EQU	C5_IR_UART_RX							= 0x0b
	.EQU	C5_IR_UART_UDRE						= 0x0c
	.EQU	C5_IR_UART_TX							= 0x0d
	.EQU	C5_IR_ADC								= 0x0e
	.EQU	C5_IR_EE_READY							= 0x0f
	.EQU	C5_IR_ANALOG_COMP						= 0x10
	.EQU	C5_IR_TWI								= 0x11
	.EQU	C5_IR_INT2								= 0x12
	.EQU	C5_IR_TIMER0_COMP						= 0x13
	.EQU	C5_IR_SPM_READY						= 0x14
	;Отсутствуют
	.EQU	C5_IR_PCINT0							= 0x00
	.EQU	C5_IR_PCINT1							= 0x00
	.EQU	C5_IR_PCINT2							= 0x00

	;SBI/CBI/SBIS/SBIC port defines
	.EQU	PORTA										= 0x1b
	.EQU	DDRA										= 0x1a
	.EQU	PINA										= 0x19
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

	.EQU	SDA										= PC1
	.EQU	SCL										= PC0
	.EQU	RXD										= PD0
	.EQU	TXD										= PD1
	.EQU	RXD0										= RXD
	.EQU	TXD0										= TXD
	.EQU	SCK										= PB7
	.EQU	MISO										= PB6
	.EQU	MOSI										= PB5
	.EQU	SS											= PB4

	;---IO-REGISTERS---------------------------------------
	.EQU	SREG										= 0x5f
	.EQU	SPH										= 0x5e
	.EQU	SPL										= 0x5d
	.EQU	MCUCSR									= 0x54
	.EQU	MCUSR										= MCUCSR;
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
	;TODO дополнить
	;---UART---
	.EQU	UDR0										= 0x2c
	.EQU	UCSR0A									= 0x2b
	.EQU	UCSR0B									= 0x2a
	.EQU	UCSR0C									= 0x40
	.EQU	UBRR0H									= 0x40
	.EQU	UBRR0L									= 0x29
	.EQU	UCSZ00									= 0x01
	.EQU	UCSZ01									= 0x02
	.EQU	UCSZ02									= 0x02
	.EQU	URSEL0									= 0x07
	.EQU	UMSEL0									= 0x06
	.EQU	UPM00										= 0x04
	.EQU	UPM01										= 0x05
	.EQU	USBS0										= 0x03
	.EQU	RXC0										= 0x07
	.EQU	TXC0										= 0x06
	.EQU	RXCIE0									= 0x07
	.EQU	TXCIE0									= 0x06
	.EQU	RXEN0										= 0x04
	.EQU	TXEN0										= 0x03
	;---PCINT---
	.EQU	PCMSK0									= 0x00
	.EQU	PCMSK1									= 0x00
	.EQU	PCMSK2									= 0x00
	;---INT---
	.EQU	MCUCR										= 0x55
	.EQU		SM2									= 0x07
	.EQU		SE										= 0x06
	.EQU		SM1									= 0x05
	.EQU		SM0									= 0x04
	.EQU		ISC11									= 0x03
	.EQU		ISC10									= 0x02
	.EQU		ISC01									= 0x01
	.EQU		ISC00									= 0x00
	.EQU	GICR										= 0x5b
	.EQU		INT0									= 0x06
	.EQU		INT1									= 0x07
	.EQU		INT2									= 0x05
	.EQU		IVSEL									= 0x01
	.EQU		IVCE									= 0x00
	.EQU	GIFR										= 0x5a
	.EQU		INTF0									= 0x06
	.EQU		INTF1									= 0x07
	.EQU		INTF2									= 0x05
	.EQU	EICRA										= MCUCR
	.EQU	EIMSK										= GICR
	.EQU	EIFR										= GIFR
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
	;--SPI---
	.EQU	SPCR										= 0x2d
	.EQU		SPIE									= 0x07
	.EQU		SPE									= 0x06
	.EQU		DORD									= 0x05
	.EQU		MSTR									= 0x04
	.EQU		CPOL									= 0x03
	.EQU		CPHA									= 0x02
	.EQU		SPR1									= 0x01
	.EQU		SPR0									= 0x00
	.EQU	SPSR										= 0x2e
	.EQU		SPIF									= 0x07
	.EQU		WCOL									= 0x06
	.EQU		SPI2X									= 0x00
	.EQU	SPDR										= 0x2f
	;--SPM---
	.EQU	SPMCSR									= 0x57
	.EQU	SPMCR										= SPMCSR
	.EQU		SPMIE									= 0x07
	.EQU		RWWSB									= 0x06
	.EQU		SIGRD									= 0x05
	.EQU		RWWSRE								= 0x04
	.EQU		BLBSET								= 0x03
	.EQU		PGWRT									= 0x02
	.EQU		PGERS									= 0x01
	.EQU		SPMEN									= 0x00

	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AREF							= (0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AVCC							= (0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56_CAP						= (1<<REFS1)|(1<<REFS0)

	.SET	INT0_PORT								= PD2
	.SET	INT1_PORT								= PD3

.ORG 0x0000
	JMP	MAIN															; Reset
	CALL	_C5_IR														; External Interrupt0
	CALL	_C5_IR														; External Interrupt1
.IF TIMER_C_ENABLE == 0x01
	CALL	_C5_TIMER_C_IR												; Timer/Counter2 Compare
.ELSE
	CALL	_C5_IR														; Timer/Counter2 Compare
.ENDIF
	CALL	_C5_IR														; Timer/Counter2 Overflow
	CALL	_C5_IR														; Timer/Counter1 Capture Event
	JMP	_C5_TIMER_A_IR												; Timer/Counter1 Compare Match A
	JMP	_C5_TIMER_B_IR												; Timer/Counter1 Compare Match B
	CALL	_C5_IR														; Timer/Counter1 Overflow
	CALL	_C5_IR														; Timer/Counter0 Overflow
	CALL	_C5_IR														; SPI Serial Transfer Complete
	CALL	_C5_IR														; USART Rx Complete
	CALL	_C5_IR														; USART, Data Register Empty
	CALL	_C5_IR														; USART, Tx Complete
	CALL	_C5_IR														; ADC Conversion Complete
	CALL	_C5_IR														; EEPROM Ready
	CALL	_C5_IR														; Analog Comparator
	CALL	_C5_IR														; 2-wire Serial Interface
	CALL	_C5_IR														; External Interrupt2
	CALL	_C5_IR														; Timer/Counter0 Compare
	CALL	_C5_IR														; Store Program Memory Ready

PORTS_TABLE:															;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA,PORTB,DDRB,PINB и т.д.
	.db	0x3b,0x3a,0x39,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30
PCINT_TABLE:
	.db	0x00,0x00,0x00,0x00
INT_TABLE:
	.db	(1<<INT0),(1<<INT1),(1<<INT2)

APPLICATION_BLOCK:

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

.MACRO _C5_TIMERB
	;SUBI TEMP,0x08														;Подобрано вручную, не должно быть вообще
	PUSH TEMP
	LDS TEMP_L,TCNT1L
	LDS TEMP_H,TCNT1H
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	STS OCR1BH,TEMP_H
	STS OCR1BL,TEMP_L

	LDS TEMP_H,SREG
	CLI
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
	STS SREG,TEMP_H

	SBRC _C5_COREFLAGS,_CFL_TIMER_B_USE
	RJMP PC-0x01
	;Останов таймера
	LDS TEMP_H,SREG
	CLI
	LDS TEMP,TIMSK
	ANDI TEMP,~(1<<OCIE1B)
	STS TIMSK,TEMP
	STS SREG,TEMP_H
	POP TEMP
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
