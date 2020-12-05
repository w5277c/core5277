;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.07.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/io/port_mode_in.inc"
.include	"./inc/io/port_mode_out.inc"
.include	"./inc/io/port_set_lo.inc"
.include	"./inc/io/port_set_hi.inc"
.include	"./inc/io/port_get.inc"
.include	"./inc/math/mul8x8.inc"
.include	"./inc/math/div16x8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_AM2301_BUFFER					= 0x00			;5B - буффер для чтения данных
	.EQU		_DRV_AM2301_BUFFER_HH			= 0x00
	.EQU		_DRV_AM2301_BUFFER_HL			= 0x01
	.EQU		_DRV_AM2301_BUFFER_TH			= 0x02
	.EQU		_DRV_AM2301_BUFFER_TL			= 0x03
	.EQU		_DRV_AM2301_BUFFER_PARITY		= 0x04
	.EQU	_DRV_AM2301_PORT						= 0x05			;1B - порт
	.EQU	_DRV_AM2301_PCI_DRV					= 0x06			;1B - драйвер PCI
	.EQU	_DRV_AM2301_TCNT						= 0x07			;1B - счетчик
	.EQU	_DRV_AM2301_ACCUM						= 0x08			;1B - аккумклятор
	.EQU	_DRV_AM2301_OFFSET					= 0x09			;1B - смещение
	;---
	.EQU	_DRV_AM2301_RAM_SIZE					= 0x0a			;10 байт необходимо выделить

;---RESULTS---
	.EQU	DRV_AM2301_RESULT_OK					= 0x00
	.EQU	DRV_AM2301_RESULT_ABSENT			= 0x01
	.EQU	DRV_AM2301_RESULT_ERROR				= 0x02

;--------------------------------------------------------
DRV_AM2301_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H - ид драйвера PCINT, TEMP_L - порт
;--------------------------------------------------------
	LDI ACCUM,_DRV_AM2301_RAM_SIZE
	MCALL C5_RAM_REALLOC
	STD Z+_DRV_AM2301_PCI_DRV,TEMP_H
	STD Z+_DRV_AM2301_PORT,TEMP_L

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: TEMP_H,TEMP_L - значение температуры в SDNF формате
;TEMP_EH,TEMP_EL - значение влажности в SDNF формате
;TEMP - код результата
;0x00 - ok,
;0x01 - датчик не обнаружен
;0x02 - ошибка PARITY
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	LDD ACCUM,Z+_DRV_AM2301_PORT
	MCALL PORT_MODE_OUT											;BE
	MCALL PORT_SET_LO
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x01														;2ms(0.8-20)
	MCALL C5_WAIT_2MS
	MCALL C5_DISPATCHER_LOCK
	MCALL PORT_SET_HI													;GO
	LDI TEMP,(30*2)													;30us(20-200)
	MCALL C5_WAIT_500NS
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	LDI TEMP,(40*2)													;Trel
	MCALL C5_WAIT_500NS
	MCALL PORT_GET
	BRCC PC+0x02
	RJMP _DRV_AM2301_PROC__ABSENT
	LDI TEMP,((40+40)*2)												;Troh
	MCALL C5_WAIT_500NS
	MCALL PORT_GET
	BRCS PC+0x02
	RJMP _DRV_AM2301_PROC__ABSENT

	CLR TEMP
	STD Z+_DRV_AM2301_OFFSET,TEMP
	LDD TEMP,Z+_DRV_AM2301_PCI_DRV
	LDD ACCUM,Z+_DRV_AM2301_PORT
	MOV FLAGS,PID
	LDI YH,high(_DRV_AM2301_EVENT)
	LDI YL,low(_DRV_AM2301_EVENT)
	MCALL C5_EXEC

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP_H,0
	LDI TEMP_L,0
	LDI TEMP,(8/2)
	MCALL C5_WAIT_2MS

	LDD TEMP,Z+_DRV_AM2301_OFFSET
	CPI TEMP,40
	LDI TEMP,DRV_AM2301_RESULT_ABSENT
	BREQ PC+0x02
	RJMP _DRV_AM2301_PROC__END

	CLR ACCUM
	LDD TEMP,Z+_DRV_AM2301_BUFFER+0x00
	ADD ACCUM,TEMP
	LDD TEMP,Z+_DRV_AM2301_BUFFER+0x01
	ADD ACCUM,TEMP
	LDD TEMP,Z+_DRV_AM2301_BUFFER+0x02
	ADD ACCUM,TEMP
	LDD TEMP,Z+_DRV_AM2301_BUFFER+0x03
	ADD ACCUM,TEMP
	LDD TEMP,Z+_DRV_AM2301_BUFFER+0x04
	CP TEMP,ACCUM
	LDI TEMP,DRV_AM2301_RESULT_ERROR
	BRNE _DRV_AM2301_PROC__ERROR

	LDD TEMP_H,Z+_DRV_AM2301_BUFFER_HH
	LDD TEMP_L,Z+_DRV_AM2301_BUFFER_HL
	LDI TEMP,0x0a
	MCALL C5_DIV16x8
	MOV TEMP_EH,TEMP_L
	LDI TEMP,0x0a
	MCALL C5_MUL8x8
	LDD TEMP,Z+_DRV_AM2301_BUFFER_HL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL C5_MUL8x8
	MOV TEMP_EL,TEMP_L

	LDD TEMP_H,Z+_DRV_AM2301_BUFFER_TH
	MOV ACCUM,TEMP_H
	ANDI TEMP_H,0x7f
	LDD TEMP_L,Z+_DRV_AM2301_BUFFER_TL
	LDI TEMP,0x0a
	MCALL C5_DIV16x8
	PUSH TEMP_L
	LDI TEMP,0x0a
	MCALL C5_MUL8x8
	LDD TEMP,Z+_DRV_AM2301_BUFFER_TL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL C5_MUL8x8
	SBRC ACCUM,0x07
	ORI TEMP_L,0x80
	POP TEMP_H

	LDI TEMP,DRV_AM2301_RESULT_OK

_DRV_AM2301_PROC__ERROR:
	RJMP _DRV_AM2301_PROC__END
_DRV_AM2301_PROC__ABSENT:
	LDI TEMP,DRV_AM2301_RESULT_ABSENT
	MCALL C5_DISPATCHER_UNLOCK
_DRV_AM2301_PROC__END:
	PUSH TEMP
	LDD TEMP,Z+_DRV_AM2301_PCI_DRV
	LDD ACCUM,Z+_DRV_AM2301_PORT
	LDI FLAGS,0xff
	LDI YH,0x00
	LDI YL,0x00
	MCALL C5_EXEC
	POP TEMP

	POP ACCUM
	POP_Z
	RET


;--------------------------------------------------------
_DRV_AM2301_EVENT:
;--------------------------------------------------------
;Обработчик прерывания PCINT
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP
	PUSH ACCUM
	PUSH_Z

	MCALL C5_RAM_OFFSET

	LDD ACCUM,Z+_DRV_AM2301_TCNT
	MCALL C5_TCNT_GET
	STD Z+_DRV_AM2301_TCNT,TEMP
	SBRC FLAGS,0x00
	RJMP _DRV_AM2301_EVENT__END
	SUB TEMP,ACCUM
	COM TEMP
	CPI TEMP,(256-145)
	LDD ACCUM,Z+_DRV_AM2301_ACCUM
	ROL ACCUM
	STD Z+_DRV_AM2301_ACCUM,ACCUM
	LDD TEMP,Z+_DRV_AM2301_OFFSET
	CPI TEMP,40
	BREQ _DRV_AM2301_EVENT__END
	MOV TEMP_H,TEMP
	INC TEMP
	STD Z+_DRV_AM2301_OFFSET,TEMP
MOV TEMP,TEMP_H
	ANDI TEMP,0x07
CPI TEMP,0x07
	BRNE _DRV_AM2301_EVENT__END
	LSR TEMP_H
	LSR TEMP_H
	LSR TEMP_H
	ADD ZL,TEMP_H
	CLR TEMP
	ADC ZH,TEMP
	ST Z,ACCUM
_DRV_AM2301_EVENT__END:

	POP_Z
	POP ACCUM
	POP TEMP
	POP TEMP_H
	RET
