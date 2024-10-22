;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.10.2024	konstantin@5277.ru			Начало, в процессе разработки
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_ADD, DRV_OP_GET, DRV_OP_SET
;В главном блоке программы должен быть объявлен ID драйвера как DRV_VPORT_ID и количество виртуальных пртов VPORTS(до
;подгрузки библиотеки МК)

.ifdef DEF__DRV_VPORT
.else
.set DEF__DRV_VPORT = 1
.message "Included driver VPORT v0.1"

;---CONSTANTS--------------------------------------------
	;---DATA---
	.EQU	_DRV_VPORT_DRIVER_IDS							= VPORT		;ID драйвера для всех виртуальных портов
	.EQU	_DRV_VPORT_RAM_SIZE								= _DRV_VPORT_DRIVER_IDS+VPORT


;--------------------------------------------------------
DRV_VPORT_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
	LDI ACCUM,_DRV_VPORT_RAM_SIZE
	MCALL C5_RAM_REALLOC

	MOVW XL,YL
	LDI TEMP,0xff
	LDI LOOP_CNTR,_DRV_VPORT_RAM_SIZE
	MCALL RAM_FILL

	MCALL C5_READY

;--------------------------------------------------------
;Основной код.
;--------------------------------------------------------
;IN: FLAGS-код операции
;TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_OP_ADD
	BRNE _DRV_VPORT__NO_OP_ADD
	MCALL _DRV_VPORT_OP_ADD
	RJMP _DRV_VPORT__END
_DRV_VPORT__NO_OP_ADD:
	CPI FLAGS,DRV_OP_GET
	BRNE _DRV_VPORT__NO_OP_GET
	MCALL _DRV_VPORT_OP_GET
	RJMP _DRV_VPORT__END
_DRV_VPORT__NO_OP_GET:
	CPI FLAGS,DRV_OP_SET
	BRNE _DRV_VPORT__NO_OP_SET
	MCALL _DRV_VPORT_OP_SET
	RJMP _DRV_VPORT__END
_DRV_VPORT__NO_OP_SET:
	CPI FLAGS,DRV_OP_SET_MODE
	BRNE _DRV_VPORT__NO_OP_SET
	MCALL _DRV_VPORT_OP_SET
	RJMP _DRV_VPORT__END
_DRV_VPORT__NO_OP_SET_MODE:
	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_VPORT__END:

	POP_Y
	RET

;--------------------------------------------------------
_DRV_VPORT_OP_ADD:
;--------------------------------------------------------
;Записываем ид драйвера
;IN: TEMP-ID драйвера, ACCUM-начальный порт,
;LOOP_CNTR-количество портов данного драйвера
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM

	SUBI ACCUM,PORTS_QUANTITY
	ADD YL,ACCUM
	ADC YH,C0x00
	STD Y+_DRV_VPORT_DRIVER_IDS,TEMP

	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_VPORT_OP_GET:
;--------------------------------------------------------
;Получаем значение от драйвера и помещаем в IO регистр
;IN: ZL-адрес на IO регистр, LOOP_CNTR-номер порта
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH ZH
	PUSH ACCUM

	RCALL _DRV__VPORT_GET_DRV_ID
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_GET__ERROR

	MCALL C5_DRV_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_VPORT_OP_GET__ERROR

	LDI ZH,0x00
	ST Z,ACCUM
	RJMP _DRV_VPORT_OP_GET__END

_DRV_VPORT_OP_GET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_VPORT_OP_GET__END:
	POP ACCUM
	POP ZH
	RET


;--------------------------------------------------------
_DRV_VPORT_OP_SET:
;--------------------------------------------------------
;Считываем значение с IO регистра и передаем его драйверу
;IN: ZL-адрес на IO регистр, LOOP_CNTR-номер порта
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH ZH
	PUSH ACCUM

	RCALL _DRV__VPORT_GET_DRV_ID
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_SET__ERROR

	LDI ZH,0x00
	LD ACCUM,Z

	MCALL C5_DRV_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ _DRV_VPORT_OP_GET__END

_DRV_VPORT_OP_GET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_VPORT_OP_GET__END:
	POP ACCUM
	POP ZH
	RET

;--------------------------------------------------------
_DRV__VPORT_GET_DRV_ID:
;--------------------------------------------------------
;IN: ZL-адрес на IO регистр
;OUT: TEMP-ID драйвера
;--------------------------------------------------------
	PUSH LOOP_CNTR

	LDI TEMP,VPORTS_ADDR
	LDI LOOP_CNTR,VPORTS
_VPORT_GET_DRV_ID__LOOP:
	CP TEMP,ZL
	BREQ _VPORT_GET_DRV_ID__END
	INC TEMP
	CP TEMP,ZL
	BREQ _VPORT_GET_DRV_ID__END
	INC TEMP
	CP TEMP,ZL
	BREQ _VPORT_GET_DRV_ID__END
	INC TEMP

	DEC LOOP_CNTR
	BRNE _VPORT_GET_DRV_ID__LOOP
	LDI TEMP,0xff

_VPORT_GET_DRV_ID__END:
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
VPORT_OFFSETS:
;--------------------------------------------------------
;Возвращаем адреса для PORTx, DDRx и PINx,
;а также пин в виде числа(1,2,4,8,16,32,64,128)
;IN: ACCUM-сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: TEMP_H-адрес регистра PORTx,
;TEMP_L-адрес регистра DDRx,
;TEMP-адрес регистра PINx
;ACCUM-пин в виде числа
;--------------------------------------------------------
	PUSH ACCUM
	SWAP ACCUM
	ANDI ACCUM,0x0f
	SUBI ACCUM,PORTS_QUANTITY
	MOV TEMP_H,ACCUM
	LSL ACCUM
	ADD TEMP_H,ACCUM
	LDI ACCUM,VPORTS_ADDR
	ADD TEMP_H,ACCUM
	MOV TEMP_L,TEMP_H
	INC TEMP_L
	MOV TEMP,TEMP_L
	INC TEMP
	POP TEMP
	ANDI TEMP,0x0f
	MCALL BITNUM_TO_NUM
	MOV ACCUM,TEMP
	RET

;--------------------------------------------------------
VPORT_GET:
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,DRV_VPORT_ID
;TODO добавить номер порта, как узнать, где начало портов данного драйвера?
	LDI FLAGS,DRV_OP_GET
	MCALL C5_DRV_EXEC

	POP FLAGS
	POP TEMP
	RET

;--------------------------------------------------------
VPORT_SET:
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,DRV_VPORT_ID
;TODO добавить номер порта относительно драйвера и тип операции GET,SET,SET_MODE
	MCALL C5_DRV_EXEC

	POP FLAGS
	POP TEMP
	RET
.endif
