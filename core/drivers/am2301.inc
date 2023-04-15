;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.07.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;26.04.2021	w5277c@gmail.com			Исправлена ошибка чтения первого бита
;03.06.2021	w5277c@gmail.com			Скорректирована модель анализа таймингов
;03.07.2021	w5277c@gmail.com			Z->Y
;20.12.2021	w5277c@gmail.com			Приведение результата к виду DRV_RESULT_...
;15.04.2023	konstantin@5277.ru		Оттестировано c C5_IR_INTn
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_AM2301
.else
.set DEF__C5_DRV_AM2301 = 1
.message "Included driver AM2301 v0.6"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"
.include	"./math/mul10.inc"
.include	"./math/div16x8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_AM2301_BUFFER					= 0x00			;5B - буффер для чтения данных
	.EQU		_DRV_AM2301_BUFFER_HH			= 0x00
	.EQU		_DRV_AM2301_BUFFER_HL			= 0x01
	.EQU		_DRV_AM2301_BUFFER_TH			= 0x02
	.EQU		_DRV_AM2301_BUFFER_TL			= 0x03
	.EQU		_DRV_AM2301_BUFFER_PARITY		= 0x04
	.EQU	_DRV_AM2301_PORT						= 0x05			;1B - порт
	.EQU	_DRV_AM2301_IR_DRV					= 0x06			;1B - драйвер PCI или C5_IR_INTn
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
;IN: TEMP_H-ид драйвера PCINT или прерывание C5_IR_INTn
;(с выключенным 0x07 битом),TEMP_L-порт
;--------------------------------------------------------
	LDI ACCUM,_DRV_AM2301_RAM_SIZE
	MCALL C5_RAM_REALLOC
	STD Y+_DRV_AM2301_IR_DRV,TEMP_H
	STD Y+_DRV_AM2301_PORT,TEMP_L

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP_EH,TEMP_EL-значение влажности в SDNF формате
;TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH ACCUM
	PUSH FLAGS

	MCALL C5_RAM_OFFSET

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
	LDD TEMP_H,Y+_DRV_AM2301_IR_DRV
	SBRC TEMP_H,0x07
	RJMP _DRV_AM2301_PROC__RX_PCINT_ENABLE
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	PUSH_Y
	LDI_Y _DRV_AM2301_EVENT
	MCALL C5_EINT_ENABLE
	POP_Y
	RJMP _DRV_AM2301_PROC__RX_PCINT_INT_ENABLE_DONE
_DRV_AM2301_PROC__RX_PCINT_ENABLE:
	LDD ACCUM,Y+_DRV_AM2301_PORT
	PUSH_X
	LDI_X _DRV_AM2301_EVENT
	MOV FLAGS,PID
	MCALL C5_EXEC
	POP_X
_DRV_AM2301_PROC__RX_PCINT_INT_ENABLE_DONE:

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP_H,BYTE3(8/2)
	LDI TEMP_L,BYTE2(8/2)
	LDI TEMP,BYTE1(8/2)
	MCALL C5_WAIT_2MS

	LDD TEMP_H,Y+_DRV_AM2301_IR_DRV
	SBRC TEMP_H,0x07
	RJMP _DRV_AM2301_INT_DISABLE__PCINT
	MCALL C5_EINT_DISABLE
	RJMP _DRV_AM2301_INT_DISABLE__END
_DRV_AM2301_INT_DISABLE__PCINT:
	LDD ACCUM,Y+_DRV_AM2301_PORT
	LDI FLAGS,0xff
	MCALL C5_EXEC
_DRV_AM2301_INT_DISABLE__END:

	LDD TEMP,Y+_DRV_AM2301_OFFSET
	CPI TEMP,41
	LDI TEMP,DRV_RESULT_ABSENT
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
	LDI TEMP,DRV_RESULT_ERROR
	BRNE _DRV_AM2301_PROC__ERROR

	LDD TEMP_H,Y+_DRV_AM2301_BUFFER_HH
	LDD TEMP_L,Y+_DRV_AM2301_BUFFER_HL
	LDI TEMP,0x0a
	MCALL DIV16x8
	MOV TEMP_EH,TEMP_L
	MCALL MUL10
	MOV TEMP_EL,TEMP_L
	LDD TEMP_H,Y+_DRV_AM2301_BUFFER_TH
	MOV ACCUM,TEMP_H
	ANDI TEMP_H,0x7f
	LDD TEMP_L,Y+_DRV_AM2301_BUFFER_TL
	LDI TEMP,0x0a
	MCALL DIV16x8
	MOV TEMP_H,TEMP_L
	PUSH TEMP_H
	MCALL MUL10
	POP TEMP_H
	SBRC ACCUM,0x07
	ORI TEMP_L,0x80

	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_AM2301_PROC__END

_DRV_AM2301_PROC__ERROR:
	LDI TEMP_EL,0xff
	LDI TEMP_L,0xff
	RJMP _DRV_AM2301_PROC__END
_DRV_AM2301_PROC__ABSENT:
	LDI TEMP,DRV_RESULT_ABSENT
	LDI TEMP_EL,0xff
	LDI TEMP_L,0xff
	MCALL C5_DISPATCHER_UNLOCK
_DRV_AM2301_PROC__END:

	POP FLAGS
	POP ACCUM
	POP_Y
	POP_X
	RET


;--------------------------------------------------------
_DRV_AM2301_EVENT:
;--------------------------------------------------------
;Обработчик прерывания
;IN: FLAGS(бит 0)-состояние порта
;--------------------------------------------------------
	PUSH_T16
	PUSH TEMP
	PUSH ACCUM
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET

	LDD ACCUM,Y+_DRV_AM2301_TCNT
	MCALL C5_TCNT_GET
	STD Y+_DRV_AM2301_TCNT,TEMP

	LDD TEMP_H,Y+_DRV_AM2301_OFFSET
	CPI TEMP_H,0x00
	BREQ _DRV_AM2301_EVENT__NEXT
	CPI TEMP_H,41
	BRCC _DRV_AM2301_EVENT__END

	SUB TEMP,ACCUM
	CPI TEMP,60*(CORE_FREQ/8)
	BRCS _DRV_AM2301_EVENT__ROL
	CPI TEMP,110*(CORE_FREQ/8)
	BRCC _DRV_AM2301_EVENT__SET
	CLC
	RJMP _DRV_AM2301_EVENT__ROL
_DRV_AM2301_EVENT__SET:
	SEC
_DRV_AM2301_EVENT__ROL:
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
	POP_T16
	RET
.endif
