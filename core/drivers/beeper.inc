;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;09.03.2020  w5277c@gmail.com        Начало
;26.10.2020  w5277c@gmail.com        Переход на программный таймер
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_BEEPER
.else
.set DEF_DRIVER_BEEPER = 1

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_invert.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BEEPER_PORT						= 0x00;2b - Порт
	.EQU	_DRV_BEEPER_TIMER_ID					= 0x02;1b - ид таймера
	.EQU	_DRV_BEEPER_TIMER_CNTR				= 0x03;2b - Счетчик для таймера
	.EQU	_DRV_BEEPER_TIMER_TH					= 0x05;2b - Порог для счетчика таймера

	.EQU	_DRV_BEEPER_RAM_SIZE					= 0x07;

	.EQU	P											= 0x00
	.EQU	S											= 0xfe
	.EQU	E											= 0xff

	.EQU	C1											= 0x01
	.EQU	C1d										= 0x02
	.EQU	D1											= 0x03
	.EQU	D1d										= 0x04
	.EQU	E1											= 0x05
	.EQU	E1d										= 0x06
	.EQU	F1											= 0x07
	.EQU	F1d										= 0x08
	.EQU	G1											= 0x09
	.EQU	G1d										= 0x0a
	.EQU	A1											= 0x0b
	.EQU	A1d										= 0x0c
	.EQU	H1											= 0x0d
	.EQU	H1d										= 0x0e

	.EQU	C2											= 0x0f
	.EQU	C2d										= 0x10
	.EQU	D2											= 0x11
	.EQU	D2d										= 0x12
	.EQU	E2											= 0x13
	.EQU	E2d										= 0x14
	.EQU	F2											= 0x15
	.EQU	F2d										= 0x16
	.EQU	G2											= 0x17
	.EQU	G2d										= 0x18
	.EQU	A2											= 0x19
	.EQU	A2d										= 0x1a
	.EQU	H2											= 0x1b
	.EQU	H2d										= 0x1c

	.EQU	C3											= 0x1d
	.EQU	C3d										= 0x1e
	.EQU	D3											= 0x1f
	.EQU	D3d										= 0x20
	.EQU	E3											= 0x21
	.EQU	E3d										= 0x22
	.EQU	F3											= 0x23
	.EQU	F3d										= 0x24
	.EQU	G3											= 0x25
	.EQU	G3d										= 0x26
	.EQU	A3											= 0x27
	.EQU	A3d										= 0x28
	.EQU	H3											= 0x29
	.EQU	H3d										= 0x2a

	.EQU	C4											= 0x2b
	.EQU	C4d										= 0x2c
	.EQU	D4											= 0x2d
	.EQU	D4d										= 0x2e
	.EQU	E4											= 0x2f
	.EQU	E4d										= 0x30
	.EQU	F4											= 0x31
	.EQU	F4d										= 0x32
	.EQU	G4											= 0x33
	.EQU	G4d										= 0x34
	.EQU	A4											= 0x35
	.EQU	A4d										= 0x36
	.EQU	H4											= 0x37
	.EQU	H4d										= 0x38

	.EQU	C5											= 0x39
	.EQU	C5d										= 0x3a
	.EQU	D5											= 0x3b
	.EQU	D5d										= 0x3c
	.EQU	E5											= 0x3d
	.EQU	E5d										= 0x3e
	.EQU	F5											= 0x3f
	.EQU	F5d										= 0x40
	.EQU	G5											= 0x41
	.EQU	G5d										= 0x42
	.EQU	A5											= 0x43
	.EQU	A5d										= 0x44
	.EQU	H5											= 0x45
	.EQU	H5d										= 0x46

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
.IF TIMERS_SPEED == TIMERS_SPEED_25NS
	.DW	15625/  26, 15625/  29, 15625/  32, 15625/  34, 15625/  38, 15625/  43, 15625/  48	;1
	.DW	15625/  51, 15625/  57, 15625/  64, 15625/  68, 15625/  77, 15625/  86, 15625/  96	;2
	.DW	15625/ 102, 15625/ 115, 15625/ 129, 15625/ 136, 15625/ 153, 15625/ 172, 15625/ 193	;3
	.DW	15625/ 204, 15625/ 229, 15625/ 258, 15625/ 273, 15625/ 306, 15625/ 344, 15625/ 386	;4
	.DW	15625/ 409, 15625/ 459, 15625/ 515, 15625/ 546, 15625/ 613, 15625/ 688, 15625/ 772	;5
	.DW	15625/ 818
.ELSE	;50ns
	.DW	15625/  51, 15625/  57, 15625/  64, 15625/  68, 15625/  77, 15625/  86, 15625/  96	;1
	.DW	15625/ 102, 15625/ 115, 15625/ 129, 15625/ 136, 15625/ 153, 15625/ 172, 15625/ 193	;2
	.DW	15625/ 204, 15625/ 229, 15625/ 258, 15625/ 273, 15625/ 306, 15625/ 344, 15625/ 386	;3
	.DW	15625/ 409, 15625/ 459, 15625/ 515, 15625/ 546, 15625/ 613, 15625/ 688, 15625/ 772	;4
	.DW	15625/ 818, 15625/ 918, 15625/1030, 15625/1091, 15625/1225, 15625/1375, 15625/1543	;5 !E=Ed, G=Gd, A=Ad, H=Hd
	.DW	15625/1635
.ENDIF

;--------------------------------------------------------
DRV_BEEPER_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - порт, FLAGS - ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	PUSH ACCUM
	LDI ACCUM,_DRV_BEEPER_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Инициализирую таймер (0,000050s)
	MOV TEMP,FLAGS
	STD Z+_DRV_BEEPER_TIMER_ID,TEMP
	LDI TEMP_H,high(_DRV_BEEPER_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_BEEPER_TIMER_EVENT)
	LDI ACCUM,0x01|(0<<0x07)
	MCALL C5_TIMER_SET
	
	POP ACCUM
	STD Z+_DRV_BEEPER_PORT,ACCUM
	;Инициализация порта
	MCALL PORT_SET_LO
	MCALL PORT_MODE_OUT
	;Записываем адрес PORTx и число
	MCALL PORT_OFFSETS
	STD Z+_DRV_BEEPER_PORT+0x00,TEMP_H
	STD Z+_DRV_BEEPER_PORT+0x01,ACCUM

	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	MCALL C5_READY
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
	PUSH FLAGS

	;Считываем информацию о пине
	MCALL C5_RAM_OFFSET

	LDD FLAGS,Z+_DRV_BEEPER_TIMER_ID

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
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	POP FLAGS
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
;IN: TEMP - байт, LOOP_CNTR - длина ноты,
;FLAGS - ид таймера
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
	MCALL C5_RAM_OFFSET
	STD Z+_DRV_BEEPER_TIMER_TH+0x00,TEMP_H
	STD Z+_DRV_BEEPER_TIMER_TH+0x01,TEMP_L
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x00,TEMP_H
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x01,TEMP_L

_DRV_BEEPER_PLAY_BYTE__SKIP:
	MOV TEMP,FLAGS
	MCALL C5_TIMER_START
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
	MCALL C5_WAIT_2MS

	MOV TEMP,FLAGS
	MCALL C5_TIMER_STOP

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
	PUSH_Y

	;Сичтываем информацию о пине
	MCALL C5_RAM_OFFSET

	LDD YH,Z+_DRV_BEEPER_TIMER_CNTR+0x00
	LDD YL,Z+_DRV_BEEPER_TIMER_CNTR+0x01
	SBIW YL,0x01
	BREQ PC+0x04
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x00,YH
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x01,YL
	RJMP _DRV_BEEPER_TIMER_EVENT__END

	LDD YH,Z+_DRV_BEEPER_TIMER_TH+0x00
	LDD YL,Z+_DRV_BEEPER_TIMER_TH+0x01
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x00,YH
	STD Z+_DRV_BEEPER_TIMER_CNTR+0x01,YL

	PUSH ACCUM
	;INVERT
	LDD YH,Z+_DRV_BEEPER_PORT+0x01
	LDD ZL,Z+_DRV_BEEPER_PORT+0x00
	CLR ZH
	LDS ACCUM,SREG
	CLI
	LD YL,Z
	EOR YL,YH
	ST Z,YL
	STS SREG,ACCUM
	POP ACCUM

_DRV_BEEPER_TIMER_EVENT__END:
	POP_Y
	POP_Z
	RET
.endif
