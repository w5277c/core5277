;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.08.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

;---DESCRIPTION------------------------------------------
;core5277 for ATtiny85 (8KB ROM, 512B EEPROM, 512B SRAM) at 16MHz (LITTLE ENDIAN)


;avrdude -p m8 -c avrispmkII -U flash:w:main.hex
;;;;;Ext crystal 8.0-..., 16K CK/14 CK + 4.1ms, CKDIV8 disabled, BOD disabled, SPI enabled:
;;;;;avrdude -p m8 -c avrispmkII -U lfuse:w:0xef:m -U hfuse:w:0xdc:m -U efuse:w:0xf9:m

	;---SPECIFY-DEVICE---
	.DEVICE ATtiny45

	#pragma AVRPART MEMORY PROG_FLASH 8192
	#pragma AVRPART MEMORY EEPROM 512
	#pragma AVRPART MEMORY INT_SRAM SIZE 512
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60

	#include "/inc/devices/_common.inc"

;---CONSTANTS--------------------------------------------
	.SET	DEVICE_FAMILY							= ATTINY25
	.MESSAGE "######## DEVICE ATtiny45 ########"
	.EQU	_C5_DRIVERS_MAX_QNT			= 0x08			;Максимальное количество драйверов
	.EQU	_C5_TASKS_MAX_QNT				= 0x04			;Максимальное количество задач
	.EQU	_C5_IR_QNT						= 0x0f			;Количество перырваний(без RESET)
	.EQU	_C5_RES_QUEUE_SIZE			= 0x10			;Размер очереди к ресурсам(8 элементов по 2 байта)
	.EQU	_C5_PCINT_PORTS_QNT			= 0x01			;Количество портов поддерживающих внешние прерывания(PCINT)
	.EQU	_C5_RAM_BORDER_SIZE			= 0x02			;Минимальное допустимое расстояние между вершинами FREE_RAM и STACK
	.EQU	_C5_STACK_SIZE					= 0x40			;Стек ядра
	.EQU	RAMEND									= 0x02ff

	;---IR---
	.EQU	C5_IR_INT0						= 0x01
	.EQU	C5_IR_PCINT0					= 0x02
	.EQU	C5_IR_TIMER1_COMPA			= 0x03
	.EQU	C5_IR_TIMER1_OVF				= 0x04
	.EQU	C5_IR_TIMER0_OVF				= 0x05
	.EQU	C5_IR_EE_READY					= 0x06
	.EQU	C5_IR_ANALOG_COMP				= 0x07
	.EQU	C5_IR_ADC						= 0x08
	.EQU	C5_IR_TIMER1_COMPB			= 0x09
	.EQU	C5_IR_TIMER0_COMPA			= 0x0a
	.EQU	C5_IR_TIMER0_COMPB			= 0x0b
	.EQU	C5_IR_WDT						= 0x0c
	.EQU	C5_IR_USI_START				= 0x0d
	.EQU	C5_IR_USI_OVF					= 0x0e
	;Отсутствуют
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
	;---ADC-VOLTAGE-REFERENCE---
	.EQU	ADC_VREF_AVCC							= (0<<REFS2)|(0<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_AREF							= (0<<REFS2)|(0<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_1_1							= (0<<REFS2)|(1<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_1_1_CAP						= (0<<REFS2)|(1<<REFS1)|(1<<REFS0)
	.EQU	ADC_VREF_2_56							= (1<<REFS2)|(1<<REFS1)|(0<<REFS0)
	.EQU	ADC_VREF_2_56_CAP						= (1<<REFS2)|(1<<REFS1)|(1<<REFS0)

.ORG 0x0000
	RJMP	MAIN															; Reset Handler
	RCALL	_C5_IR												; External Interrupt0 Handler
	RCALL	_C5_IR												; Pin Change Interrupt Request 0
	RCALL	_C5_IR												; Timer/Counter1 Compare Match A
	RCALL	_C5_IR												; Timer/Counter1 Overflow
	RCALL	_C5_IR												; Timer/Counter0 Overflow
	RCALL	_C5_IR												; EEPROM Ready
	RCALL	_C5_IR												; Analog Comparator
	RCALL	_C5_IR												; ADC Conversion Complete
	RCALL	_C5_IR												; Timer/Counter1 Compare Match B
	RJMP	_C5_TIMER_A_IR										; Timer/Counter0 Compare Match A
	RCALL	_C5_TIMER_B_IR										; Timer/Counter0 Compare Match B
	RCALL	_C5_IR												; Watchdog Time-out Interrupt
	RCALL	_C5_IR												; USI start
	RCALL	_C5_IR												; USI overflow

_C5_PORTS_TABLE:												;Таблица из адресов IO регистров (A,B,C,D), поочередно PORTA,DDRA,PINA, и т.д.
	.db	0x00,0x00,0x00,0x38,0x37,0x36,0x00,0x00,0x00,0x00,0x00,0x00
_C5_PCINT_TABLE:
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

.MACRO _C5_PRR_RESET
	PUSH TEMP
	LDS TEMP,PRR
	ANDI TEMP,~(1<<@0)
	STS PRR,TEMP
	POP TEMP
.ENDMACRO

.MACRO _C5_TIMER_TCNT
	LDS TEMP,TCNT0
.ENDMACRO
