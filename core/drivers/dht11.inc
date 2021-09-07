;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.06.2021	w5277c@gmail.com			Начало
;04.07.2021	w5277c@gmail.com			Z->Y, оптимизация
;07.09.2021	w5277c@gmail.com			Багфикс
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_DHT11
.else
.set DEF__C5_DRV_DHT11 = 1
.message "Included driver DHT11 v0.3"

.include	"./core/drivers/_dht.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_DHT11_BUFFER						= 0x00			;5B - буффер для чтения данных
	.EQU		_DRV_DHT11_BUFFER_HH				= 0x00
	.EQU		_DRV_DHT11_BUFFER_HL				= 0x01
	.EQU		_DRV_DHT11_BUFFER_TH				= 0x02
	.EQU		_DRV_DHT11_BUFFER_TL				= 0x03
	.EQU		_DRV_DHT11_BUFFER_PARITY		= 0x04
	.EQU	_DRV_DHT11_PORT						= 0x05			;1B - порт
	.EQU	_DRV_DHT11_PCI_DRV					= 0x06			;1B - драйвер PCI
	.EQU	_DRV_DHT11_TCNT						= 0x07			;1B - счетчик
	.EQU	_DRV_DHT11_TLOW						= 0x08			;1B - время низкого уровня
	.EQU	_DRV_DHT11_ACCUM						= 0x09			;1B - аккумклятор
	.EQU	_DRV_DHT11_OFFSET						= 0x0a			;1B - смещение
	;---
	.EQU	_DRV_DHT11_RAM_SIZE					= 0x0b			;11 байт необходимо выделить

;--------------------------------------------------------
DRV_DHT11_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-ид драйвера PCINT, TEMP_L-порт
;--------------------------------------------------------
	LDI ACCUM,_DRV_DHT11_RAM_SIZE
	MCALL C5_RAM_REALLOC
	STD Y+_DRV_DHT11_PCI_DRV,TEMP_H
	STD Y+_DRV_DHT11_PORT,TEMP_L

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
	LDD ACCUM,Y+_DRV_DHT11_PORT
	MCALL PORT_MODE_OUT												;BE
	MCALL PORT_SET_LO
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x0b														;22ms(>20)
	MCALL C5_WAIT_2MS
	MCALL C5_DISPATCHER_LOCK
	MCALL PORT_SET_HI													;GO
	C5_WAIT_500NS (30*2)												;30us(20-200)
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	C5_WAIT_500NS (40*2)												;Trel
	MCALL PORT_GET
	BRCC PC+0x02
	RJMP _DRV_DHT11_PROC__ABSENT
	C5_WAIT_500NS ((40+40)*2)												;Troh
	MCALL PORT_GET
	BRCS PC+0x02
	RJMP _DRV_DHT11_PROC__ABSENT

	STD Y+_DRV_DHT11_OFFSET,C0x00
	LDD TEMP,Y+_DRV_DHT11_PCI_DRV
	LDD ACCUM,Y+_DRV_DHT11_PORT
	MOV FLAGS,PID
	LDI_X _DRV_DHT11_EVENT
	MCALL C5_EXEC

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP_H,0
	LDI TEMP_L,0
	LDI TEMP,(8/2)
	MCALL C5_WAIT_2MS

	LDD TEMP,Y+_DRV_DHT11_OFFSET
	CPI TEMP,41
	LDI TEMP,DRV_DHT_RESULT_ABSENT
	BREQ PC+0x02
	RJMP _DRV_DHT11_PROC__END

	CLR ACCUM
	LDD TEMP_EH,Y+_DRV_DHT11_BUFFER+0x00
	ADD ACCUM,TEMP_EH
	LDD TEMP_EL,Y+_DRV_DHT11_BUFFER+0x01
	ADD ACCUM,TEMP_EL
	LDD TEMP_H,Y+_DRV_DHT11_BUFFER+0x02
	ADD ACCUM,TEMP_H
	LDD TEMP_L,Y+_DRV_DHT11_BUFFER+0x03
	ADD ACCUM,TEMP_L
	LDD TEMP,Y+_DRV_DHT11_BUFFER+0x04
	CP TEMP,ACCUM
	LDI TEMP,DRV_DHT_RESULT_ERROR
	BRNE _DRV_DHT11_PROC__ERROR
	LDI TEMP,DRV_DHT_RESULT_OK

_DRV_DHT11_PROC__ERROR:
	RJMP _DRV_DHT11_PROC__END
_DRV_DHT11_PROC__ABSENT:
	LDI TEMP,DRV_DHT_RESULT_ABSENT
	MCALL C5_DISPATCHER_UNLOCK
_DRV_DHT11_PROC__END:
	PUSH TEMP
	LDD TEMP,Y+_DRV_DHT11_PCI_DRV
	LDD ACCUM,Y+_DRV_DHT11_PORT
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
_DRV_DHT11_EVENT:
;--------------------------------------------------------
;Обработчик прерывания PCINT,
;IN: FLAGS(бит 0)-состояние порта
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH_Y

	MCALL C5_RAM_OFFSET_Y
	LDD ACCUM,Y+_DRV_DHT11_TCNT
	MCALL C5_TCNT_GET
	STD Y+_DRV_DHT11_TCNT,TEMP
	SBRS FLAGS,0x00
	RJMP _DRV_DHT11_EVENT__HI
	SUB TEMP,ACCUM
	;Надо проверить:
	SUBI TEMP,CORE_FREQ
	STD Y+_DRV_DHT11_TLOW,TEMP
	RJMP _DRV_DHT11_EVENT__END

_DRV_DHT11_EVENT__HI:
	LDD TEMP_H,Y+_DRV_DHT11_OFFSET
	CPI TEMP_H,0x00
	BREQ _DRV_DHT11_EVENT__NEXT
	CPI TEMP_H,41
	BRCC _DRV_DHT11_EVENT__END

	SUB TEMP,ACCUM
	LDD ACCUM,Y+_DRV_DHT11_TLOW
	CP ACCUM,TEMP
	LDD ACCUM,Y+_DRV_DHT11_ACCUM
	ROL ACCUM
	STD Y+_DRV_DHT11_ACCUM,ACCUM

	MOV TEMP,TEMP_H
	ANDI TEMP,0x07
	BRNE _DRV_DHT11_EVENT__NEXT
	MOV TEMP_L,TEMP_H
	DEC TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	LSR TEMP_L
	PUSH_Z
	MOVW ZL,YL
	ADD ZL,TEMP_L
	ADC ZH,C0x00
	ST Z,ACCUM
	POP_Z

_DRV_DHT11_EVENT__NEXT:
	INC TEMP_H
	STD Y+_DRV_DHT11_OFFSET,TEMP_H
_DRV_DHT11_EVENT__END:

	POP_Y
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
