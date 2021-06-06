;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.07.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;26.04.2021	w5277c@gmail.com			Исправлена ошибка чтения первого бита
;03.06.2021	w5277c@gmail.com			Скорректирована модель анализа таймингов
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_AM2301
.else
.set DEF__C5_DRV_AM2301 = 1
.message "Included driver AM2301 v0.3"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"
.include	"./math/mul8x8.inc"
.include	"./math/div16x8.inc"

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
	.EQU	_DRV_AM2301_TLOW						= 0x08			;1B - время низкого уровня
	.EQU	_DRV_AM2301_ACCUM						= 0x09			;1B - аккумклятор
	.EQU	_DRV_AM2301_OFFSET					= 0x0a			;1B - смещение
	;---
	.EQU	_DRV_AM2301_RAM_SIZE					= 0x0b			;11 байт необходимо выделить

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
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP_EH,TEMP_EL-значение влажности в SDNF формате
;TEMP - код результата DRV_AM2301_RESULT_...
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
	C5_WAIT_500NS (30*2)												;30us(20-200)
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	C5_WAIT_500NS (40*2)												;Trel
	MCALL PORT_GET
	BRCC PC+0x02
	RJMP _DRV_AM2301_PROC__ABSENT
	C5_WAIT_500NS ((40+40)*2)												;Troh
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
	CPI TEMP,41
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
	MCALL DIV16x8
	MOV TEMP_EH,TEMP_L
	LDI TEMP,0x0a
	MCALL MUL8x8
	LDD TEMP,Z+_DRV_AM2301_BUFFER_HL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL MUL8x8
	MOV TEMP_EL,TEMP_L

	LDD TEMP_H,Z+_DRV_AM2301_BUFFER_TH
	MOV ACCUM,TEMP_H
	ANDI TEMP_H,0x7f
	LDD TEMP_L,Z+_DRV_AM2301_BUFFER_TL
	LDI TEMP,0x0a
	MCALL DIV16x8
	PUSH TEMP_L
	LDI TEMP,0x0a
	MCALL MUL8x8
	LDD TEMP,Z+_DRV_AM2301_BUFFER_TL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL MUL8x8
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
;Обработчик прерывания PCINT,
;IN: FLAGS(бит 0)-состояние порта
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH_Z

	MCALL C5_RAM_OFFSET

	LDD ACCUM,Z+_DRV_AM2301_TCNT
	MCALL C5_TCNT_GET
	STD Z+_DRV_AM2301_TCNT,TEMP
	SBRS FLAGS,0x00
	RJMP _DRV_AM2301_EVENT__HI
	SUB TEMP,ACCUM
	;Надо проверить:
	SUBI TEMP,CORE_FREQ
	STD Z+_DRV_AM2301_TLOW,TEMP
	RJMP _DRV_AM2301_EVENT__END

_DRV_AM2301_EVENT__HI:
	LDD TEMP_H,Z+_DRV_AM2301_OFFSET
	CPI TEMP_H,0x00
	BREQ _DRV_AM2301_EVENT__NEXT
	CPI TEMP_H,41
	BRCC _DRV_AM2301_EVENT__END

	SUB TEMP,ACCUM
	LDD ACCUM,Z+_DRV_AM2301_TLOW
	CP ACCUM,TEMP
	LDD ACCUM,Z+_DRV_AM2301_ACCUM
	ROL ACCUM
	STD Z+_DRV_AM2301_ACCUM,ACCUM

	MOV TEMP,TEMP_H
	ANDI TEMP,0x07
	BRNE _DRV_AM2301_EVENT__NEXT
	MOV TEMP_L,TEMP_H
	DEC TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	PUSH_Z
	ADD ZL,TEMP_L
	CLR TEMP
	ADC ZH,TEMP
	ST Z,ACCUM
	POP_Z

_DRV_AM2301_EVENT__NEXT:
	INC TEMP_H
	STD Z+_DRV_AM2301_OFFSET,TEMP_H
_DRV_AM2301_EVENT__END:

	POP_Z
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
