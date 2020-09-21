;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;09.03.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_DRIVER_BEEPER
.else
.set DEF_CORE5277_DRIVER_BEEPER = 1

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/io/port_set_lo.inc"
.include	"./inc/io/port_invert.inc"
.include	"./inc/io/port_mode_in.inc"
.include	"./inc/io/port_mode_out.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BEEPER_PORT						= 0x00;1b - Порт
	.EQU	_DRV_BEEPER_TIMER_CNTR				= 0x01;2b - Счетчик для таймера
	.EQU	_DRV_BEEPER_TIMER_TH					= 0x03;2b - Порог для счетчика таймера

	.EQU	_DRV_BEEPER_RAM_SIZE					= 0x05;

	.EQU	P											= 0x00
	.EQU	S											= 0xfe
	.EQU	E											= 0xff

	.EQU	C0											= 0x01
	.EQU	C0d										= 0x02
	.EQU	D0											= 0x03
	.EQU	D0d										= 0x04
	.EQU	E0											= 0x05
	.EQU	E0d										= 0x06
	.EQU	F0											= 0x07
	.EQU	F0d										= 0x08
	.EQU	G0											= 0x09
	.EQU	G0d										= 0x0a
	.EQU	A0											= 0x0b
	.EQU	A0d										= 0x0c
	.EQU	H0											= 0x0d
	.EQU	H0d										= 0x0e

	.EQU	C1											= 0x0f
	.EQU	C1d										= 0x10
	.EQU	D1											= 0x11
	.EQU	D1d										= 0x12
	.EQU	E1											= 0x13
	.EQU	E1d										= 0x14
	.EQU	F1											= 0x15
	.EQU	F1d										= 0x16
	.EQU	G1											= 0x17
	.EQU	G1d										= 0x18
	.EQU	A1											= 0x19
	.EQU	A1d										= 0x1a
	.EQU	H1											= 0x1b
	.EQU	H1d										= 0x1c

	.EQU	C2											= 0x1d
	.EQU	C2d										= 0x1e
	.EQU	D2											= 0x1f
	.EQU	D2d										= 0x20
	.EQU	E2											= 0x21
	.EQU	E2d										= 0x22
	.EQU	F2											= 0x23
	.EQU	F2d										= 0x24
	.EQU	G2											= 0x25
	.EQU	G2d										= 0x26
	.EQU	A2											= 0x27
	.EQU	A2d										= 0x28
	.EQU	H2											= 0x29
	.EQU	H2d										= 0x2a

	.EQU	C3											= 0x2b
	.EQU	C3d										= 0x2c
	.EQU	D3											= 0x2d
	.EQU	D3d										= 0x2e
	.EQU	E3											= 0x2f
	.EQU	E3d										= 0x30
	.EQU	F3											= 0x31
	.EQU	F3d										= 0x32
	.EQU	G3											= 0x33
	.EQU	G3d										= 0x34
	.EQU	A3											= 0x35
	.EQU	A3d										= 0x36
	.EQU	H3											= 0x37
	.EQU	H3d										= 0x38

	.EQU	C4											= 0x39
	.EQU	C4d										= 0x3a
	.EQU	D4											= 0x3b
	.EQU	D4d										= 0x3c
	.EQU	E4											= 0x3d
	.EQU	E4d										= 0x3e
	.EQU	F4											= 0x3f
	.EQU	F4d										= 0x40
	.EQU	G4											= 0x41
	.EQU	G4d										= 0x42
	.EQU	A4											= 0x43
	.EQU	A4d										= 0x44
	.EQU	H4											= 0x45
	.EQU	H4d										= 0x46

	.EQU	C5											= 0x47
	.EQU	C5d										= 0x48
	.EQU	D5											= 0x49
	.EQU	D5d										= 0x4a
	.EQU	E5											= 0x4b
	.EQU	E5d										= 0x4c
	.EQU	F5											= 0x4d
	.EQU	F5d										= 0x4e
	.EQU	G5											= 0x4f
	.EQU	G5d										= 0x50
	.EQU	A5											= 0x51
	.EQU	A5d										= 0x52
	.EQU	H5											= 0x53
	.EQU	H5d										= 0x54

	.EQU	C6											= 0x55
	.EQU	C6d										= 0x56
	.EQU	D6											= 0x57
	.EQU	D6d										= 0x58
	.EQU	E6											= 0x59
	.EQU	E6d										= 0x5a
	.EQU	F6											= 0x5b
	.EQU	F6d										= 0x5c
	.EQU	G6											= 0x5d
	.EQU	G6d										= 0x5e
	.EQU	A6											= 0x5f
	.EQU	A6d										= 0x60
	.EQU	H6											= 0x61
	.EQU	H6d										= 0x62

	.EQU	C7											= 0x63
	.EQU	C7d										= 0x64
	.EQU	D7											= 0x65
	.EQU	D7d										= 0x66
	.EQU	E7											= 0x67
	.EQU	E7d										= 0x68
	.EQU	F7											= 0x69
	.EQU	F7d										= 0x6a
	.EQU	G7											= 0x6b
	.EQU	G7d										= 0x6c
	.EQU	A7											= 0x6d
	.EQU	A7d										= 0x6e
	.EQU	H7											= 0x6f
	.EQU	H7d										= 0x70

	.EQU	C8											= 0x71
	.EQU	C8d										= 0x72
	.EQU	D8											= 0x73
	.EQU	D8d										= 0x74
	.EQU	E8											= 0x75
	.EQU	E8d										= 0x76
	.EQU	F8											= 0x77
	.EQU	F8d										= 0x78
	.EQU	G8											= 0x79
	.EQU	G8d										= 0x7a
	.EQU	A8											= 0x7b
	.EQU	A8d										= 0x7c
	.EQU	H8											= 0x7d
	.EQU	H8d										= 0x7e

	.EQU	N64										= 0x81
	.EQU	N64T										= 0x82
	.EQU	N32										= 0x83
	.EQU	N16										= 0x86
	.EQU	N16D										= 0x89
	.EQU	N16T										= 0x88
	.EQU	N8											= 0x8c
	.EQU	N8D										= 0x92
	.EQU	N8T										= 0x88
	.EQU	N4											= 0x98
	.EQU	N4D										= 0xa4
	.EQU	N4T										= 0x90
	.EQU	N2											= 0xb0
	.EQU	N2D										= 0xc8
	.EQU	N1											= 0xe0


_DRV_BEEPER_OCTAVS_TABLE:
	.DW	15625/  16, 15625/  18, 15625/  21, 15625/  22, 15625/  25, 15625/  28, 15625/  31	;0
	.DW	15625/  33, 15625/  37, 15625/  41, 15625/  44, 15625/  49, 15625/  55, 15625/  62	;1
	.DW	15625/  65, 15625/  73, 15625/  82, 15625/  87, 15625/  98, 15625/ 110, 15625/ 123	;2
	.DW	15625/ 131, 15625/ 147, 15625/ 165, 15625/ 175, 15625/ 196, 15625/ 220, 15625/ 247	;3
	.DW	15625/ 262, 15625/ 294, 15625/ 330, 15625/ 349, 15625/ 392, 15625/ 440, 15625/ 494	;4
	.DW	15625/ 523, 15625/ 587, 15625/ 659, 15625/ 698, 15625/ 784, 15625/ 880, 15625/ 988	;5
	.DW	15625/1047, 15625/1175, 15625/1319, 15625/1397, 15625/1568, 15625/1760, 15625/1976	;6
	.DW	15625/2093, 15625/2349, 15625/2637, 15625/2794, 15625/3136, 15625/3520, 15625/3951	;7
	.DW	15625/4186, 15625/4699, 15625/5274, 15625/5588, 15625/6272, 15625/7040, 15625/7902	;8
	.DW	15625/8764
;--------------------------------------------------------
DRV_BEEPER_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - порт
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_BEEPER_RAM_SIZE
	MCALL CORE5277_RAM_REALLOC
	POP ACCUM
	STD Z+_DRV_BEEPER_PORT,ACCUM

	;Инициализирую таймер (64*8=0,000032s)
.IF DEVICE_FAMILY = ATTINY25
	.MESSAGE "######## BEEPER, TIMER1 usage ########"
	_CORE5277_PRR_RESET PRTIM1										;Подпитываю TIMER1
	LDI TEMP,0x00
	STS GTCCR,TEMP
	LDI TEMP,(1<<CTC1)|(0<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10)		;/64 - for 16MHz
	STS TCCR1,TEMP
	CLR TEMP
	STS TCNT1,TEMP
	LDI TEMP,0x07
	STS OCR1A,TEMP
	LDI TEMP,(1<<OCF1A)
	STS TIFR,TEMP
	;Устанавливаем вектор прерывания
	LDI ACCUM,CORE5277_IR_TIMER1_COMPA
.ELSEIF DEVICE_FAMILY = ATMEGA48
	.MESSAGE "######## BEEPER, TIMER2 usage ########"
	_CORE5277_PRR_RESET PRTIM2										;Подпитываю TIMER2
	LDI TEMP,(1<<WGM21)|(0<<WGM20)
	STS TCCR2A,TEMP
	LDI TEMP,(0<<WGM22)|(1<<CS22)|(0<<CS21)|(0<<CS20)		;/64 - for 16MHz
	STS TCCR2B,TEMP
	CLR TEMP
	STS TCNT2,TEMP
	LDI TEMP,0x07
	STS OCR2A,TEMP
	LDI TEMP,(1<<OCF2A)
	STS TIFR2,TEMP
	;Устанавливаем вектор прерывания
	LDI ACCUM,CORE5277_IR_TIMER2_COMPA
.ELSE
	.MESSAGE "######## ERR: BEEPER, unknown FAMILY type ########"
.ENDIF

	LDI TEMP_H,high(_DRV_BEEPER_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_BEEPER_TIMER_EVENT)
	MCALL CORE5277_IR_VECTOR_SET

	MCALL CORE5277_READY
;--------------------------------------------------------
;Основной код, проигрывание звука
;IN: Y - адрес на данные
;(15-ый бит false - RAM, true - ROM)
;0x00 - конец
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	;Считываем информацию о пине
	MCALL CORE5277_RAM_OFFSET
	LDD ACCUM,Z+_DRV_BEEPER_PORT
	;Инициализация порта
	MCALL CORE5277_PORT_SET_LO
	MCALL CORE5277_PORT_MODE_OUT

	PUSH_Z
	MOV ZH,YH
	MOV ZL,YL

	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80

	LDI LOOP_CNTR,N16D
	ANDI LOOP_CNTR,0x7f

_DRV_BEEPER_PROC__BYTES_LOOP:
	;Считываем байт
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000

	CPI TEMP,E
	BREQ _DRV_BEEPER_PROC__END
	SBRS TEMP,0x07
	RJMP _DRV_BEEPER_PROC__NO_TEMP
	CPI TEMP,P
	BREQ _DRV_BEEPER_PROC__NO_TEMP
	CPI TEMP,S
	BREQ _DRV_BEEPER_PROC__NO_TEMP
	ANDI TEMP,0x7f
	MOV LOOP_CNTR,TEMP
	RJMP _DRV_BEEPER_PROC__BYTES_LOOP
_DRV_BEEPER_PROC__NO_TEMP:
	MCALL _DRV_BEEPER_PLAY_BYTE
	RJMP _DRV_BEEPER_PROC__BYTES_LOOP

_DRV_BEEPER_PROC__END:
	POP_Z
	LDD ACCUM,Z+_DRV_BEEPER_PORT
	;Отключаем порт переводя его как вход без подтяжки
	MCALL CORE5277_PORT_MODE_IN
	MCALL CORE5277_PORT_SET_LO

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_BEEPER_PLAY_BYTE:
;--------------------------------------------------------
;Печать байта
;IN: TEMP - байт, LOOP_CNTR - длина ноты
;--------------------------------------------------------
	;START
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	CPI TEMP,P
	BREQ _DRV_BEEPER_PLAY_BYTE__SKIP
	CPI TEMP,S
	BREQ _DRV_BEEPER_PLAY_BYTE__SILENT

	DEC TEMP
	MOV ACCUM,TEMP
	ANDI TEMP,0xfe
	LDI ZH,high(_DRV_BEEPER_OCTAVS_TABLE*2)
	LDI ZL,low(_DRV_BEEPER_OCTAVS_TABLE*2)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LPM TEMP_L,Z+
	LPM TEMP_H,Z+
	SBRS ACCUM,0x00
	RJMP _DRV_BEEPER_PLAY_BYTE__HALF_OCTAVE_SKIP
	LPM TEMP,Z+
	LPM ACCUM,Z
	SUB TEMP_L,TEMP
	SBC TEMP_H,ACCUM
	LSR TEMP_H
	ROR TEMP_L
	ADD TEMP_L,TEMP
	ADC TEMP_H,ACCUM
_DRV_BEEPER_PLAY_BYTE__HALF_OCTAVE_SKIP:
	MCALL CORE5277_RAM_OFFSET
	STD Z+_DRV_BEEPER_TIMER_TH+0x00,TEMP_H
	STD Z+_DRV_BEEPER_TIMER_TH+0x01,TEMP_L
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x00,TEMP_H
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x01,TEMP_L

_DRV_BEEPER_PLAY_BYTE__SKIP:
	CLR TEMP
.IF DEVICE_FAMILY = ATTINY25
	STS TCNT1,TEMP
	LDI TEMP,(1<<OCIE1A)
	STS TIMSK,TEMP
.ELSEIF DEVICE_FAMILY = ATMEGA48
	STS TCNT2,TEMP
	LDI TEMP,(1<<OCIE2A)
	STS TIMSK2,TEMP
.ENDIF

_DRV_BEEPER_PLAY_BYTE__SILENT:
	;Умножаем на 10
	CLR TEMP_L
	MOV TEMP,LOOP_CNTR
	LSL TEMP
	ROR TEMP_L
	PUSH TEMP_L
	PUSH TEMP
	LSL TEMP
	ROL TEMP_L
	LSL TEMP
	ROL TEMP_L
	POP TEMP_H
	ADD TEMP,TEMP_H
	POP TEMP_H
	ADC TEMP_L,TEMP_H
	CLR TEMP_H
	MCALL CORE5277_WAIT_2MS

.IF DEVICE_FAMILY = ATTINY25
	LDI TEMP,(0<<OCIE1A)
	STS TIMSK,TEMP
.ELSEIF DEVICE_FAMILY = ATMEGA48
	LDI TEMP,(0<<OCIE2A)
	STS TIMSK2,TEMP
.ENDIF
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET

;--------------------------------------------------------
_DRV_BEEPER_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	;Сичтываем информацию о пине
	MCALL CORE5277_RAM_OFFSET
	LDD TEMP_H,Z+_DRV_BEEPER_TIMER_CNTR+0x00
	LDD TEMP_L,Z+_DRV_BEEPER_TIMER_CNTR+0x01
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	CPI TEMP_H,0x00
	BRNE _DRV_BEEPER_TIMER_EVENT__NO_TH
	CPI TEMP_L,0x00
	BRNE _DRV_BEEPER_TIMER_EVENT__NO_TH

	LDD ACCUM,Z+_DRV_BEEPER_PORT
	MCALL CORE5277_PORT_INVERT

	LDD TEMP_H,Z+_DRV_BEEPER_TIMER_TH+0x00
	LDD TEMP_L,Z+_DRV_BEEPER_TIMER_TH+0x01
_DRV_BEEPER_TIMER_EVENT__NO_TH:
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x00,TEMP_H
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x01,TEMP_L

_DRV_BEEPER_TIMER_END:
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET
.endif
