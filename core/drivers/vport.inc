;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.10.2024	konstantin@5277.ru			Начало, в процессе разработки
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_ADD, DRV_OP_GET, DRV_OP_SET
;В главном блоке программы должен быть объявлен ID драйвера как DRV_VPORT_ID и количество виртуальных пртов VPORTS(до
;подгрузки библиотеки МК)
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер чипа расширения должен поддерживать процедуры: DRV_OP_GET и DRV_OP_SET, с параметрами:
;IN: TEMP_H-номер порта,TEMP_L-тип порта, FLAGS-число бита, ACCUM-состояние порта
;OUT: ACCUM-состоянеи порта, TEMP-DRV_RESULT_...
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__DRV_VPORT
.else
.set DEF__DRV_VPORT = 1
.message "Included driver VPORT v0.1"

;---CONSTANTS--------------------------------------------
	;---DATA---
	.EQU	_DRV_VPORT_DRIVER_IDS							= VPORTS		;ID драйвера для всех виртуальных портов
	.EQU	_DRV_VPORT_RAM_SIZE								= _DRV_VPORT_DRIVER_IDS+VPORTS


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
;OUT: См. вызываемые процедуры
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
;IN: TEMP_H-порядковый номер виртуального порта(0-...,
;TEMP_L-ID драйвера
;--------------------------------------------------------
	ADD YL,TEMP_H
	ADC YH,C0x00
	ST Y,TEMP_L
	RET

;--------------------------------------------------------
_DRV_VPORT_OP_GET:
;--------------------------------------------------------
;Получаем значение от драйвера и помещаем в IO регистр
;IN: ZL-адрес на IO регистр, LOOP_CNTR-номер порта
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH_T16
	PUSH FLAGS

	RCALL _DRV__VPORT__GET_PARAMS
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_GET__ERROR

	MOV FLAGS,ACCUM
	MCALL C5_DRV_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_VPORT_OP_GET__ERROR

	LDI ZH,0x00
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


;--------------------------------------------------------
_DRV_VPORT_OP_SET:
;--------------------------------------------------------
;Считываем значение с IO регистра и передаем его драйверу
;IN: ZL-адрес на IO регистр
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH_T16
	PUSH FLAGS

	RCALL _DRV__VPORT__GET_PARAMS
	CPI TEMP,0xff
	BREQ _DRV_VPORT_OP_SET__ERROR

	MOV FLAGS,ACCUM
	LDI ZH,0x00
	LD ACCUM,Z

	MCALL C5_DRV_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ _DRV_VPORT_OP_GET__END

_DRV_VPORT_OP_GET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_VPORT_OP_GET__END:
	POP FLAGS
	POP_T16
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_DRV__VPORT__GET_PARAMS:
;--------------------------------------------------------
;IN: ZL-адрес на IO регистр,Y-адрес на выделенную память
;OUT: TEMP-ID драйвера,TEMP_H-номер порта,TEMP_L-тип
;порта
;--------------------------------------------------------
	PUSH_Y
	PUSH ZL

	SUBI ZL,VPORTS_ADDR
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
	SUB TEMP_L,ZL														;Тип порта (0-PORT,1-DDR,2-PIN)

	POP ZL
	POP_Y
	RET

;--------------------------------------------------------
VPORT_GET:
;--------------------------------------------------------
;IN: ZL-адрес на IO регистр,ACCUM-число бита
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,DRV_VPORT_ID
	LDI FLAGS,DRV_OP_GET
	MCALL C5_DRV_EXEC

	POP FLAGS
	POP TEMP
	RET

;--------------------------------------------------------
VPORT_SET:
;--------------------------------------------------------
;IN: ZL-адрес на IO регистр,ACCUM-число бита
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS

	LDI TEMP,DRV_VPORT_ID
	LDI FLAGS,DRV_OP_SET
	MCALL C5_DRV_EXEC

	POP FLAGS
	POP TEMP
	RET
.endif
