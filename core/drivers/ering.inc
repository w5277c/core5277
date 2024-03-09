;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.02.2024	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Позволяет хранить архивные данные(типа счетчиков) с различным периодом записи
;-----------------------------------------------------------------------------------------------------------------------
;Использует кольцевой буфер для EEPROM.
;Каждая запись состоит из значения и инвертированного значения
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_GET, DRV_OP_SET, DRV_OP_STATUS, DRV_OP_EVENT, DRV_OP_FLUSH


.ifdef DEF__C5_DRV_ERING
.else
.set DEF__C5_DRV_ERING = 1
.message "Included driver ERING v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/uptime_copy.inc"
.include "./core/utils/time/uptime_mldelta.inc"
.include	"./math/mul8x8.inc"
.include	"./mem/eeprom_read_byte.inc"
.include	"./mem/eeprom_write_byte.inc"

;---CONSTANTS--------------------------------------------
	;---DATA---
	.EQU	_DRV_ERING_ELEMENT_SIZE							= 0x00;1B - размер элемента(от 1 до 4 байт)
	.EQU	_DRV_ERING_RING_SIZE								= 0x01;1B - количество элементов в кольце
	.EQU	_DRV_ERING_EEPROM_ADDR							= 0x02;2B - адрес блока EEPROM
	.EQU	_DRV_ERING_ELEMENT_POS							= 0x04;1B - текущий элемент
	.EQU	_DRV_ERING_PERIOD									= 0x05;2B - период(в минутах) записи в EEPROM нового значения(0-выполнять запись сразу)
	.EQU	_DRV_ERING_UPTIME									= 0x07;5B - UPTIME последней записи в EEPROM
	.EQU	_DRV_ERING_VALUE									= 0x0c;4B - значение счетчика для записи в EEPROM
	.EQU	_DRV_ERING_TMP_VALUE								= 0x10;4B - временное хранение значения счетчика
	.EQU	_DRV_ERING_RAM_SIZE								= _DRV_ERING_TMP_VALUE+0x04
;--------------------------------------------------------
DRV_ERING_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_EH-размер элемента(1-4), TEMP_EL-кол-во эле-
;ментов в кольце, TEMP_H,L-адрес в EEPROM, X-период запи-
;си в минутах
;--------------------------------------------------------
	LDI ACCUM,_DRV_ERING_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_ERING_ELEMENT_SIZE,TEMP_EH
	STD Y+_DRV_ERING_RING_SIZE,TEMP_EL
	STD Y+_DRV_ERING_EEPROM_ADDR+0x00,TEMP_H
	STD Y+_DRV_ERING_EEPROM_ADDR+0x01,TEMP_L
	STD Y+_DRV_ERING_PERIOD+0x00,XH
	STD Y+_DRV_ERING_PERIOD+0x01,XL
	STD Y+_DRV_ERING_ELEMENT_POS,C0x00
	STD Y+_DRV_ERING_UPTIME+0x00,C0x00
	STD Y+_DRV_ERING_UPTIME+0x01,C0x00
	STD Y+_DRV_ERING_UPTIME+0x02,C0x00
	STD Y+_DRV_ERING_UPTIME+0x03,C0x00
	STD Y+_DRV_ERING_UPTIME+0x04,C0x00

	ADIW YL,_DRV_ERING_UPTIME
	MCALL C5_UPTIME_COPY

	MCALL C5_READY

;--------------------------------------------------------
;Основной код.
;--------------------------------------------------------
;IN: FLAGS-код операции
;TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_OP_GET
	BRNE _DRV_ERING__NO_OP_GET
	MCALL _DRV_ERING_OP_GET
	RJMP _DRV_ERING__END
_DRV_ERING__NO_OP_GET:
	CPI FLAGS,DRV_OP_SET
	BRNE _DRV_ERING__NO_OP_SET
	MCALL _DRV_ERING_OP_SET
	RJMP _DRV_ERING__END
_DRV_ERING__NO_OP_SET:
	CPI FLAGS,DRV_OP_EVENT
	BRNE _DRV_ERING__NO_OP_EVENT
	MCALL _DRV_ERING_OP_EVENT
	RJMP _DRV_ERING__END
_DRV_ERING__NO_OP_EVENT:
	CPI FLAGS,DRV_OP_FLUSH
	BRNE _DRV_ERING__NO_OP_FLUSH
	MCALL _DRV_ERING_OP_FLUSH
	RJMP _DRV_ERING__END
_DRV_ERING__NO_OP_FLUSH:
	CPI FLAGS,DRV_OP_STATUS
	BRNE _DRV_ERING__NO_OP_STATUS
	MCALL _DRV_ERING_OP_STATUS
	RJMP _DRV_ERING__END
_DRV_ERING__NO_OP_STATUS:

	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_ERING__END:

	POP_Y
	RET

;--------------------------------------------------------
_DRV_ERING_OP_GET:													;TODO надо проверить
;--------------------------------------------------------
;Получаем последнее значение
;OUT: TEMP_EH/EL/H/L-значение,									;Значение заполняется начиная со старшего регистра,
;TEMP-код результата DRV_RESULT_...								;т.е. значение длиной в байт будет храниться в регистре TEMP_EH
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH FLAGS
	PUSH ACCUM

	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00

	LDI FLAGS,DRV_RESULT_ERROR

	LDD LOOP_CNTR,Y+_DRV_ERING_RING_SIZE
	LDI ACCUM,0x00

_DRV_ERING_OP_GET__LOOP:
	MCALL _DRV_ERING_READ_VALUE
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_ERING_OP_GET__NEXT
	LDI FLAGS,DRV_RESULT_OK
	LDD TEMP,Y+_DRV_ERING_TMP_VALUE+0x00	//Полученное значение
	CP TEMP_EH,TEMP
	BRCS _DRV_ERING_OP_GET__VALUE_UPDATE
	BRNE _DRV_ERING_OP_GET__NEXT
	LDD TEMP,Y+_DRV_ERING_TMP_VALUE+0x01
	CP TEMP_EL,TEMP
	BRCS _DRV_ERING_OP_GET__VALUE_UPDATE
	BRNE _DRV_ERING_OP_GET__NEXT
	LDD TEMP,Y+_DRV_ERING_TMP_VALUE+0x02
	CP TEMP_H,TEMP
	BRCS _DRV_ERING_OP_GET__VALUE_UPDATE
	BRNE _DRV_ERING_OP_GET__NEXT
	LDD TEMP,Y+_DRV_ERING_TMP_VALUE+0x03
	CP TEMP_L,TEMP
	BRCS _DRV_ERING_OP_GET__VALUE_UPDATE
	BRNE _DRV_ERING_OP_GET__NEXT
_DRV_ERING_OP_GET__VALUE_UPDATE:
	LDD TEMP_EH,Y+_DRV_ERING_TMP_VALUE+0x00
	LDD TEMP_EL,Y+_DRV_ERING_TMP_VALUE+0x01
	LDD TEMP_H,Y+_DRV_ERING_TMP_VALUE+0x02
	LDD TEMP_L,Y+_DRV_ERING_TMP_VALUE+0x03

_DRV_ERING_OP_GET__NEXT:
	INC ACCUM
	DEC LOOP_CNTR
	BRNE _DRV_ERING_OP_GET__LOOP

	POP ACCUM
	POP FLAGS
	POP LOOP_CNTR
	RET


;--------------------------------------------------------
_DRV_ERING_OP_STATUS:
;--------------------------------------------------------
;Получаем количество живых элементов
;OUT: TEMP_L-количество живых элементов
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH ACCUM

	LDD LOOP_CNTR,Y+_DRV_ERING_RING_SIZE
	LDI ACCUM,0x00
	LDI TEMP_L,0x00

_DRV_ERING_OP_STATUS__LOOP:
	MCALL _DRV_ERING_READ_VALUE
	CPI TEMP,DRV_RESULT_OK
	BRNE PC+0x02
	INC TEMP_L
	INC ACCUM
	DEC LOOP_CNTR
	BRNE _DRV_ERING_OP_STATUS__LOOP

	POP ACCUM
	POP TEMP
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_ERING_OP_SET:
;--------------------------------------------------------
;Запоминаем значение, для последующей записис в EEPROM
;IN: TEMP_EH/EL/H/L-значение,										;Значение заполняется начиная со старшего регистра
;--------------------------------------------------------;т.е. значение длиной в байт будет храниться в регистре TEMP_EH
	STD Y+_DRV_ERING_VALUE+0x00,TEMP_EH
	STD Y+_DRV_ERING_VALUE+0x01,TEMP_EL
	STD Y+_DRV_ERING_VALUE+0x02,TEMP_H
	STD Y+_DRV_ERING_VALUE+0x03,TEMP_L
	RET

;--------------------------------------------------------
_DRV_ERING_OP_EVENT:
;--------------------------------------------------------
;Проверяем истечение периода и выполняем запись в EEPROM
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_T16

	ADIW YL,_DRV_ERING_UPTIME
	MCALL C5_UPTIME_MLDELTA
	SBIW YL,_DRV_ERING_UPTIME
	LDD TEMP,Y+_DRV_ERING_PERIOD+0x00
	CP TEMP,TEMP_H
	BRCS _DRV_ERING_OP_EVENT__TIMEOUT
	BRNE _DRV_ERING_OP_EVENT__END
	LDD TEMP,Y+_DRV_ERING_PERIOD+0x01
	CP TEMP,TEMP_L
	BRCS _DRV_ERING_OP_EVENT__TIMEOUT
	BRNE _DRV_ERING_OP_EVENT__END
_DRV_ERING_OP_EVENT__TIMEOUT:
	MCALL _DRV_ERING_OP_FLUSH
_DRV_ERING_OP_EVENT__END:

	POP_T16
	RET

;--------------------------------------------------------
_DRV_ERING_OP_FLUSH:
;--------------------------------------------------------
;Выполняем запись в EEPROM
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_T32
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH FLAGS

	ADIW YL,_DRV_ERING_UPTIME
	MCALL C5_UPTIME_COPY
	SBIW YL,_DRV_ERING_UPTIME

	LDD FLAGS,Y+_DRV_ERING_RING_SIZE
	LDD LOOP_CNTR,Y+_DRV_ERING_RING_SIZE
	LDD ACCUM,Y+_DRV_ERING_ELEMENT_POS
_DRV_ERING_OP_FLUSH__LOOP:
	MCALL _DRV_ERING_WRITE_VALUE
	INC ACCUM
	CP ACCUM,FLAGS
	BRNE PC+0x02
	LDI ACCUM,0x00
	STD Y+_DRV_ERING_ELEMENT_POS,ACCUM

	CPI TEMP,DRV_RESULT_OK
	BREQ _DRV_ERING_OP_FLUSH__END
_DRV_ERING_OP_FLUSH__LOOP_NEXT:
	DEC LOOP_CNTR
	BRNE _DRV_ERING_OP_FLUSH__LOOP

	LDI TEMP,DRV_RESULT_ERROR
_DRV_ERING_OP_FLUSH__END:

	POP FLAGS
	POP ACCUM
	POP LOOP_CNTR
	POP_T32
	RET

;--------------------------------------------------------
_DRV_ERING_OFFSET:
;--------------------------------------------------------
;Получаем вдрес на текущий элемент
;IN: ACCUM-позиция
;OUT: TEMP_H/L - EEPROM адрес
;--------------------------------------------------------
	PUSH TEMP

	MOV TEMP_L,ACCUM
	LDD TEMP,Y+_DRV_ERING_ELEMENT_SIZE
	LSL TEMP																;Каждое значение состоит из двух блоков: не инвертированный и инвертированный
	MCALL MUL8X8
	LDD TEMP,Y+_DRV_ERING_EEPROM_ADDR+0x01
	ADD TEMP_L,TEMP
	LDD TEMP,Y+_DRV_ERING_EEPROM_ADDR+0x00
	ADC TEMP_H,TEMP

	POP TEMP
	RET

;--------------------------------------------------------
_DRV_ERING_READ_VALUE:
;--------------------------------------------------------
;Записываем значение в _DRV_ERING_TMP_VALUE
;IN: ACCUM-позиция
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Z
	PUSH FLAGS
	PUSH LOOP_CNTR
	PUSH_T16
	PUSH ACCUM

	STD Y+_DRV_ERING_TMP_VALUE+0x00,C0x00
	STD Y+_DRV_ERING_TMP_VALUE+0x01,C0x00
	STD Y+_DRV_ERING_TMP_VALUE+0x02,C0x00
	STD Y+_DRV_ERING_TMP_VALUE+0x03,C0x00

	MOVW ZL,YL															;В Z смещение на временное значение счетчика
	ADIW ZL,_DRV_ERING_TMP_VALUE

	LDI FLAGS,0x01
	MCALL _DRV_ERING_OFFSET
	LDD LOOP_CNTR,Y+_DRV_ERING_ELEMENT_SIZE
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
_DRV_ERING_READ_VALUE__LOOP:
	MCALL EEPROM_READ_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	MOV ACCUM,TEMP
	MCALL EEPROM_READ_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	COM TEMP
	CP TEMP,ACCUM
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ERROR
	RJMP _DRV_ERING_READ_VALUE__END
	ST -Z,ACCUM
	DEC LOOP_CNTR
	BRNE _DRV_ERING_READ_VALUE__LOOP
	LDI TEMP,DRV_RESULT_OK

_DRV_ERING_READ_VALUE__END:
	POP ACCUM
	POP_T16
	POP LOOP_CNTR
	POP FLAGS
	POP_Z
	RET


;--------------------------------------------------------
_DRV_ERING_WRITE_VALUE:
;--------------------------------------------------------
;Записываем значение в EEPROM и проверяем
;IN: ACCUM-позиция, TEMP_EH/EL/H/L-значение
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Y
	PUSH_T16
	PUSH FLAGS
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI FLAGS,0x01

	LDD LOOP_CNTR,Y+_DRV_ERING_ELEMENT_SIZE
	MCALL _DRV_ERING_OFFSET
	PUSH_Y
	ADIW YL,_DRV_ERING_VALUE
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
_DRV_ERING_WRITE_VALUE__WLOOP:
	LD TEMP,-Y
	MCALL EEPROM_WRITE_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	COM TEMP
	MCALL EEPROM_WRITE_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	DEC LOOP_CNTR
	BRNE _DRV_ERING_WRITE_VALUE__WLOOP

	POP_Y
	LDD LOOP_CNTR,Y+_DRV_ERING_ELEMENT_SIZE
	MCALL _DRV_ERING_OFFSET
	ADIW YL,_DRV_ERING_VALUE
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
_DRV_ERING_WRITE_VALUE__RLOOP:
	LD ACCUM,-Y
	MCALL EEPROM_READ_BYTE
	CP TEMP,ACCUM
	BRNE _DRV_ERING_WRITE_VALUE__ERROR
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	MCALL EEPROM_READ_BYTE
	COM ACCUM
	CP TEMP,ACCUM
	BRNE _DRV_ERING_WRITE_VALUE__ERROR
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	DEC LOOP_CNTR
	BRNE _DRV_ERING_WRITE_VALUE__RLOOP

	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_ERING_WRITE_VALUE__END
_DRV_ERING_WRITE_VALUE__ERROR:
	LDI TEMP,DRV_RESULT_ERROR

_DRV_ERING_WRITE_VALUE__END:
	POP LOOP_CNTR
	POP ACCUM
	POP FLAGS
	POP_T16
	POP_Y
	RET
.endif
