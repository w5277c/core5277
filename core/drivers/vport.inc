;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.10.2024	konstantin@5277.ru		Начало, в процессе разработки
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_ADD, DRV_OP_GET_PORT, DRV_OP_SET_PORT
;В главном блоке программы должен быть объявлен ID драйвера как DRV_VPORT_ID и количество виртуальных пртов VPORTS(до
;подгрузки библиотеки МК)
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер чипа расширения должен поддерживать процедуры: DRV_OP_GET_PORT и DRV_OP_SET_PORT, с параметрами:
;IN: TEMP_H-номер порта,TEMP_L-тип порта(VPORT_REG_...), FLAGS-пин-число бита, ACCUM-состояние порта(для set)
;OUT: ACCUM-состоянеи порта(для get), TEMP-DRV_RESULT_...
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__DRV_VPORT
.else
.set DEF__DRV_VPORT = 1
.message "Included driver VPORT v0.1"

	.include	"./mem/ram_fill.inc"
	.include	"./io/port_set_byte.inc"
	.include	"./io/port_mode_byte.inc"
;---CONSTANTS-----------------------------------------------
	.EQU	_DRV_VPORT_DRIVER_IDS					= 0x00							;ID драйвера для всех виртуальных портов
	.EQU	_DRV_VPORT_QUEUE_MAX					= _DRV_VPORT_DRIVER_IDS+VPORTS	;Максимальный размер элементов
	.EQU	_DRV_VPORT_QUEUE_SIZE					= _DRV_VPORT_QUEUE_MAX+0x01		;Текущий размер очереди в элементах
	.EQU	_DRV_VPORT_QUEUE						= _DRV_VPORT_QUEUE_SIZE+0x01	;Очередь заявок


	.EQU	_DRV_VPORT_ITEM_ADDR					= 0x00	;Аадрес на IO регистр
	.EQU	_DRV_VPORT_ITEM_PINNUM					= 0x02	;Номер пина
	.EQU	_DRV_VPORT_ITEM_OPCODE					= 0x03	;Код опреации
	.EQU	_DRV_VPORT_ITEM_SIZE					= 0x04	;Размер элемента

;-----------------------------------------------------------
DRV_VPORT_INIT:
;-----------------------------------------------------------
;Инициализация
;-----------------------------------------------------------
;IN-LOOP_CNTR-максимальный размер элементов в очереди
;-----------------------------------------------------------
	MOV TEMP,LOOP_CNTR
	LSL LOOP_CNTR
	LSL LOOP_CNTR
	LDI ACCUM,_DRV_VPORT_QUEUE
	ADD ACCUM,LOOP_CNTR
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_VPORT_QUEUE_MAX,TEMP
	STD Y+_DRV_VPORT_QUEUE_SIZE,C0x00

	MOVW XL,YL
	LDI TEMP,0xff
	LDI LOOP_CNTR,VPORTS
	MCALL RAM_FILL


	MCALL C5_READY
;-----------------------------------------------------------
;Основной код.
;-----------------------------------------------------------
;IN: FLAGS-код операции
;OUT: См. вызываемые процедуры
;-----------------------------------------------------------
	PUSH_Y
	PUSH TEMP_EH
	PUSH TEMP_EL

	MCALL C5_RAM_OFFSET

	MCALL C5_DISPATCHER_LOCK
	LDD TEMP_EH,Y+_DRV_VPORT_QUEUE_MAX
	LDD TEMP_EL,Y+_DRV_VPORT_QUEUE_SIZE
	CP TEMP_EH,TEMP_EL
	BRNE _DRV_VPORT__NO_OVERFLOW
	MCALL C5_DISPATCHER_UNLOCK
;TODO Выводить более информативно или вовсе убрать
.ifdef LOGGING_PORT											;Очередь переполнена
	PUSH TEMP
	LDI TEMP,'#'
	MCALL C5_OUT_CHAR
	POP TEMP
.endif
	LDI TEMP,DRV_RESULT_TRY_LATER
	RJMP _DRV_VPORT__END
_DRV_VPORT__NO_OVERFLOW:
	INC TEMP_EL
	STD Y+_DRV_VPORT_QUEUE_SIZE,TEMP_EL
	MCALL C5_DISPATCHER_UNLOCK

	CPI TEMP_EL,0x01
	BREQ _DRV_VPORT__SKIP_QUEUE
	SUBI TEMP_EL,0x02
	LSL TEMP_EL
	LSL TEMP_EL
	ADIW YL,VPORTS+0x02
	ADD YL,TEMP_EL
	ADC YH,C0x00
	STD Y+_DRV_VPORT_ITEM_ADDR+0x00,ZH
	STD Y+_DRV_VPORT_ITEM_ADDR+0x01,ZL
	STD Y+_DRV_VPORT_ITEM_PINNUM,ACCUM
	STD Y+_DRV_VPORT_ITEM_OPCODE,FLAGS
	RJMP _DRV_VPORT__END

_DRV_VPORT__SKIP_QUEUE:
	PUSH_Y
	CPI FLAGS,DRV_OP_ADD
	BRNE _DRV_VPORT__NO_OP_ADD
	MCALL _DRV_VPORT_OP_ADD
	RJMP _DRV_VPORT__DONE
_DRV_VPORT__NO_OP_ADD:
	CPI FLAGS,DRV_OP_GET_PORT
	BRNE _DRV_VPORT__NO_OP_GET
	MCALL _DRV_VPORT_OP_GET
	RJMP _DRV_VPORT__DONE
_DRV_VPORT__NO_OP_GET:
	CPI FLAGS,DRV_OP_SET_PORT
	BRNE _DRV_VPORT__DONE
	MCALL _DRV_VPORT_OP_SET

_DRV_VPORT__DONE:
	POP_Y
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP_EL,Y+_DRV_VPORT_QUEUE_SIZE
	DEC TEMP_EL
	STD Y+_DRV_VPORT_QUEUE_SIZE,TEMP_EL
	MCALL C5_DISPATCHER_UNLOCK
	CPI TEMP_EL,0x00
	BREQ _DRV_VPORT__END

	LDD LOOP_CNTR,Y+_DRV_VPORT_QUEUE_MAX
	ADIW YL,VPORTS+0x02
	LDD ZH,Y+_DRV_VPORT_ITEM_ADDR+0x00
	LDD ZL,Y+_DRV_VPORT_ITEM_ADDR+0x01
	LDD ACCUM,Y+_DRV_VPORT_ITEM_PINNUM
	LDD FLAGS,Y+_DRV_VPORT_ITEM_OPCODE
	;TODO подвинуть заявку
	PUSH_Z
	PUSH_X
	MOVW ZL,YL
	MOVW XL,YL
	ADIW XL,_DRV_VPORT_ITEM_SIZE
	DEC LOOP_CNTR
	LSL LOOP_CNTR
	LSL LOOP_CNTR
	MCALL RAM_COPY
	POP_X
	POP_Z
	SBIW YL,VPORTS+0x02
	RJMP _DRV_VPORT__SKIP_QUEUE

_DRV_VPORT__END:

	POP TEMP_EL
	POP TEMP_EH
	POP_Y
	RET

;-----------------------------------------------------------
_DRV_VPORT_OP_ADD:
;-----------------------------------------------------------
;Записываем ид драйвера
;IN: TEMP_H-порядковый номер виртуального порта(0-...,
;TEMP_L-ID драйвера
;-----------------------------------------------------------
	ADD YL,TEMP_H
	ADC YH,C0x00
	ST Y,TEMP_L
	RET

;-----------------------------------------------------------
_DRV_VPORT_OP_GET:
;-----------------------------------------------------------
;Получаем значение от драйвера и помещаем в RAM
;IN: ZL-адрес на IO регистр, ACCUM-номер пина
;OUT: TEMP-код результата DRV_RESULT_...
;-----------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH_T16
	PUSH FLAGS

	RCALL _DRV__VPORT__GET_PARAMS
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_GET__ERROR

	MOV FLAGS,ACCUM
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_VPORT_OP_GET__ERROR

	ST Z,ACCUM
	RJMP _DRV_VPORT_OP_GET__END

_DRV_VPORT_OP_GET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_VPORT_OP_GET__END:
	POP FLAGS
	POP_T16
	POP ACCUM
	POP_Z
	RET


;-----------------------------------------------------------
_DRV_VPORT_OP_SET:
;-----------------------------------------------------------
;Считываем значение с RAM и передаем его драйверу
;IN: Z-адрес, ACCUM-номер пина
;OUT: TEMP-код результата DRV_RESULT_...
;-----------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH_T16
	PUSH FLAGS

	RCALL _DRV__VPORT__GET_PARAMS
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_SET__ERROR

	;MOV FLAGS,ACCUM
	LD ACCUM,Z

	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ _DRV_VPORT_OP_SET__END

_DRV_VPORT_OP_SET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_VPORT_OP_SET__END:
	POP FLAGS
	POP_T16
	POP ACCUM
	POP_Z
	RET

;-----------------------------------------------------------
_DRV__VPORT__GET_PARAMS:
;-----------------------------------------------------------
;IN: Z-адрес на VPORT,Y-адрес на выделенную память
;OUT: TEMP-ID драйвера,TEMP_H-номер порта,TEMP_L-тип
;порта(VPORT_REG_...)
;-----------------------------------------------------------
	PUSH_Y
	PUSH_Z

	SUBI ZL,low(VPORTS_ADDR)
	SBCI ZH,high(VPORTS_ADDR)
	MOV TEMP_L,ZL

	MOV TEMP,ZL															;(X*5+X/2)>>4 (деление на 3, корректно до делимого 31 включительно, поэтому максимум 8 портов)
	LSL ZL
	LSL ZL
	ADD ZL,TEMP
	LSR TEMP
	ADD ZL,TEMP
	SWAP ZL
	ANDI ZL,0x0f

	MOV TEMP_H,ZL														;Номер порта
	ADD YL,ZL															;Смещение на ID драйвера
	ADC YH,C0x00
	LD TEMP,Y															;ID драйвера

	LSL ZL																;Номер порта *3
	ADD ZL,TEMP_H
	SUB TEMP_L,ZL														;Тип порта (0-PIN,1-DDR,2-PORT)

	POP_Z
	POP_Y
	RET

;-----------------------------------------------------------
VPORT_GET:
;-----------------------------------------------------------
;IN: Z-адрес, ACCUM-число бита
;-----------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,PID_VPORT_DRV
	LDI FLAGS,DRV_OP_GET_PORT
	MCALL C5_EXEC

	POP FLAGS
	POP TEMP
	RET

;-----------------------------------------------------------
VPORT_SET:
;-----------------------------------------------------------
;IN: Z-адрес, ACCUM-число бита
;-----------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,PID_VPORT_DRV
	LDI FLAGS,DRV_OP_SET_PORT
	MCALL C5_EXEC

	POP FLAGS
	POP TEMP
	RET
.endif
