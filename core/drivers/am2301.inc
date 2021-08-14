;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.07.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;26.04.2021	w5277c@gmail.com			Исправлена ошибка чтения первого бита
;03.06.2021	w5277c@gmail.com			Скорректирована модель анализа таймингов
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
;TODO: Не пройден тест, DRV_AM2301_RESULT_ERROR
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_AM2301
.else
.set DEF__C5_DRV_AM2301 = 1
.message "Included driver AM2301 v0.5"

.include	"./core/drivers/_dht.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
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

;--------------------------------------------------------
DRV_AM2301_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-ид драйвера PCINT,TEMP_L-порт
;--------------------------------------------------------
	LDI ACCUM,_DRV_AM2301_RAM_SIZE
	MCALL C5_RAM_REALLOC
	STD Y+_DRV_AM2301_PCI_DRV,TEMP_H
	STD Y+_DRV_AM2301_PORT,TEMP_L

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP_EH,TEMP_EL-значение влажности в SDNF формате
;TEMP-код результата DRV_DHT_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH ACCUM
	PUSH FLAGS

	MCALL C5_RAM_OFFSET_Y
	LDD ACCUM,Y+_DRV_AM2301_PORT
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

	STD Y+_DRV_AM2301_OFFSET,C0x00
	LDD TEMP,Y+_DRV_AM2301_PCI_DRV
	LDD ACCUM,Y+_DRV_AM2301_PORT
	MOV FLAGS,PID
	LDI_X _DRV_AM2301_EVENT
	MCALL C5_EXEC

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP_H,BYTE3(8/2)
	LDI TEMP_L,BYTE2(8/2)
	LDI TEMP,BYTE1(8/2)
	MCALL C5_WAIT_2MS

	LDD TEMP,Y+_DRV_AM2301_OFFSET
	CPI TEMP,41
	LDI TEMP,DRV_DHT_RESULT_ABSENT
	BREQ PC+0x02
	RJMP _DRV_AM2301_PROC__END

	CLR ACCUM
	LDD TEMP,Y+_DRV_AM2301_BUFFER+0x00
	ADD ACCUM,TEMP
	LDD TEMP,Y+_DRV_AM2301_BUFFER+0x01
	ADD ACCUM,TEMP
	LDD TEMP,Y+_DRV_AM2301_BUFFER+0x02
	ADD ACCUM,TEMP
	LDD TEMP,Y+_DRV_AM2301_BUFFER+0x03
	ADD ACCUM,TEMP
	LDD TEMP,Y+_DRV_AM2301_BUFFER+0x04
	CP TEMP,ACCUM
	LDI TEMP,DRV_DHT_RESULT_ERROR
	BRNE _DRV_AM2301_PROC__ERROR

	LDD TEMP_H,Y+_DRV_AM2301_BUFFER_HH
	LDD TEMP_L,Y+_DRV_AM2301_BUFFER_HL
	LDI TEMP,0x0a
	MCALL DIV16x8
	MOV TEMP_EH,TEMP_L
	LDI TEMP,0x0a
	MCALL MUL8x8
	LDD TEMP,Y+_DRV_AM2301_BUFFER_HL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL MUL8x8
	MOV TEMP_EL,TEMP_L

	LDD TEMP_H,Y+_DRV_AM2301_BUFFER_TH
	MOV ACCUM,TEMP_H
	ANDI TEMP_H,0x7f
	LDD TEMP_L,Y+_DRV_AM2301_BUFFER_TL
	LDI TEMP,0x0a
	MCALL DIV16x8
	PUSH TEMP_L
	LDI TEMP,0x0a
	MCALL MUL8x8
	LDD TEMP,Y+_DRV_AM2301_BUFFER_TL
	SUB TEMP,TEMP_L
	MOV TEMP_L,TEMP
	LDI TEMP,0x0a
	MCALL MUL8x8
	SBRC ACCUM,0x07
	ORI TEMP_L,0x80
	POP TEMP_H

	LDI TEMP,DRV_DHT_RESULT_OK

_DRV_AM2301_PROC__ERROR:
	RJMP _DRV_AM2301_PROC__END
_DRV_AM2301_PROC__ABSENT:
	LDI TEMP,DRV_DHT_RESULT_ABSENT
	MCALL C5_DISPATCHER_UNLOCK
_DRV_AM2301_PROC__END:
	PUSH TEMP
	LDD TEMP,Y+_DRV_AM2301_PCI_DRV
	LDD ACCUM,Y+_DRV_AM2301_PORT
	LDI FLAGS,0xff
	LDI_X 0x0000
	MCALL C5_EXEC
	POP TEMP

	POP FLAGS
	POP ACCUM
	POP_Y
	POP_X
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
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET_Y

	LDD ACCUM,Y+_DRV_AM2301_TCNT
	MCALL C5_TCNT_GET
	STD Y+_DRV_AM2301_TCNT,TEMP
	SBRS FLAGS,0x00
	RJMP _DRV_AM2301_EVENT__HI
	SUB TEMP,ACCUM
	;Надо проверить:
	SUBI TEMP,CORE_FREQ
	STD Y+_DRV_AM2301_TLOW,TEMP
	RJMP _DRV_AM2301_EVENT__END

_DRV_AM2301_EVENT__HI:
	LDD TEMP_H,Y+_DRV_AM2301_OFFSET
	CPI TEMP_H,0x00
	BREQ _DRV_AM2301_EVENT__NEXT
	CPI TEMP_H,41
	BRCC _DRV_AM2301_EVENT__END

	SUB TEMP,ACCUM
	LDD ACCUM,Y+_DRV_AM2301_TLOW
	CP ACCUM,TEMP
	LDD ACCUM,Y+_DRV_AM2301_ACCUM
	ROL ACCUM
	STD Y+_DRV_AM2301_ACCUM,ACCUM

	MOVW ZL,YL
	MOV TEMP,TEMP_H
	ANDI TEMP,0x07
	BRNE _DRV_AM2301_EVENT__NEXT
	MOV TEMP_L,TEMP_H
	DEC TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	ADD ZL,TEMP_L
	ADC ZH,C0x00
	ST Z,ACCUM

_DRV_AM2301_EVENT__NEXT:
	INC TEMP_H
	STD Y+_DRV_AM2301_OFFSET,TEMP_H
_DRV_AM2301_EVENT__END:

	POP_Z
	POP_Y
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
