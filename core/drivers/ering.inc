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
;Значение не записывается, если оказалось меньше предыдущего или равно.
;При старте необходимо вызвать DRV_OP_GET иначе не сможем проверить на меньше или равно.
;Период отсчитывается от времени записи значения.
;Вручную вызванный DRV_OP_FLUSH записывает в EEPROM безусловно.
;Запись в EEPROM ведется с младшего байта - для учета сброса МК во время записи. Иначе сброс при записи
;0x01FF->0x0200 мог привести к значению 0x02FF. А так мы получаем 0x0100 и вероятно выберем другое, близкое, ранее
;записанное значение.
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
	.EQU	_DRV_ERING_ST_READY								= 0x00;
	.EQU	_DRV_ERING_ST_NEW_DATA							= 0x01;

	;---DATA---
	.EQU	_DRV_ERING_STATE									= 0x00;1B - состояние
	.EQU	_DRV_ERING_ELEMENT_SIZE							= 0x01;1B - размер элемента(от 1 до 4 байт)
	.EQU	_DRV_ERING_RING_SIZE								= 0x02;1B - количество элементов в кольце
	.EQU	_DRV_ERING_EEPROM_ADDR							= 0x03;2B - адрес блока EEPROM
	.EQU	_DRV_ERING_ELEMENT_POS							= 0x05;1B - текущий элемент
	.EQU	_DRV_ERING_PERIOD									= 0x06;2B - период(в минутах) записи в EEPROM нового значения(0-выполнять запись сразу)
	.EQU	_DRV_ERING_UPTIME									= 0x08;5B - UPTIME последней записи в EEPROM
	.EQU	_DRV_ERING_VALUE									= 0x0d;4B - значение счетчика для записи в EEPROM
	.EQU	_DRV_ERING_TMP_VALUE								= 0x11;4B - временное хранение значения счетчика
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

	LDI ACCUM,_DRV_ERING_ST_READY
	STD Y+_DRV_ERING_STATE,ACCUM

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
	LDD TEMP,Y+_DRV_ERING_TMP_VALUE+0x00						;Полученное значение
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

	MOV TEMP,FLAGS
	POP ACCUM
	POP FLAGS
	POP LOOP_CNTR
	RET


;--------------------------------------------------------
_DRV_ERING_OP_STATUS:
;--------------------------------------------------------
;Получаем количество живых элементов
;OUT: TEMP_L-количество живых элементов
;TEMP-код результата DRV_RESULT_...
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

	LDI TEMP,DRV_RESULT_OK

	POP ACCUM
	POP TEMP
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_ERING_OP_SET:
;--------------------------------------------------------
;Запоминаем значение, для последующей записи в EEPROM
;IN: TEMP_EH/EL/H/L-значение,										;Значение заполняется начиная со старшего регистра
;TEMP-код результата DRV_RESULT_...								;т.е. значение длиной в байт будет храниться в регистре TEMP_EH
;--------------------------------------------------------
	PUSH ACCUM

	LDD ACCUM,Y+_DRV_ERING_RING_SIZE+0x00
	LDD TEMP,Y+_DRV_ERING_VALUE+0x00
	CP TEMP_EH,TEMP
	BRNE _DRV_ERING_OP_SET__UPDATE
	BRCS _DRV_ERING_OP_SET__ERROR
	CPI ACCUM,0x01
	BREQ _DRV_ERING_OP_SET__END

	LDD TEMP,Y+_DRV_ERING_VALUE+0x01
	CP TEMP_EL,TEMP
	BRNE _DRV_ERING_OP_SET__UPDATE
	BRCS _DRV_ERING_OP_SET__ERROR
	CPI ACCUM,0x02
	BREQ _DRV_ERING_OP_SET__END

	LDD TEMP,Y+_DRV_ERING_VALUE+0x02
	CP TEMP_H,TEMP
	BRNE _DRV_ERING_OP_SET__UPDATE
	BRCS _DRV_ERING_OP_SET__ERROR
	CPI ACCUM,0x03
	BREQ _DRV_ERING_OP_SET__END

	LDD TEMP,Y+_DRV_ERING_VALUE+0x03
	CP TEMP_L,TEMP
	BREQ _DRV_ERING_OP_SET__END
	BRCS _DRV_ERING_OP_SET__ERROR

_DRV_ERING_OP_SET__UPDATE:
	LDI TEMP,DRV_RESULT_OK
	STD Y+_DRV_ERING_VALUE+0x00,TEMP_EH
	STD Y+_DRV_ERING_VALUE+0x01,TEMP_EL
	STD Y+_DRV_ERING_VALUE+0x02,TEMP_H
	STD Y+_DRV_ERING_VALUE+0x03,TEMP_L

	LDI ACCUM,_DRV_ERING_ST_NEW_DATA
	STD Y+_DRV_ERING_STATE,ACCUM
	RJMP _DRV_ERING_OP_SET__END
_DRV_ERING_OP_SET__ERROR:
	LDI TEMP,DRV_RESULT_ERROR										;Новое значение оказалось меньше уже записанного

_DRV_ERING_OP_SET__END:
	POP ACCUM
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
	BRNE _DRV_ERING_OP_EVENT__TRY_LATER
	LDD TEMP,Y+_DRV_ERING_PERIOD+0x01
	CP TEMP,TEMP_L
	BRCS _DRV_ERING_OP_EVENT__TIMEOUT
	BRNE _DRV_ERING_OP_EVENT__TRY_LATER
_DRV_ERING_OP_EVENT__TIMEOUT:
	LDD TEMP_L,Y+_DRV_ERING_STATE
	CPI TEMP_L,_DRV_ERING_ST_NEW_DATA
	BRNE _DRV_ERING_OP_EVENT__SENSELESS
	MCALL _DRV_ERING_OP_FLUSH
	RJMP _DRV_ERING_OP_EVENT__END
_DRV_ERING_OP_EVENT__TRY_LATER:
	LDI TEMP,DRV_RESULT_TRY_LATER
	RJMP _DRV_ERING_OP_EVENT__END
_DRV_ERING_OP_EVENT__SENSELESS:
	LDI TEMP,DRV_RESULT_SENSELESS

_DRV_ERING_OP_EVENT__END:
	POP_T16
	RET

;--------------------------------------------------------
_DRV_ERING_OP_FLUSH:
;--------------------------------------------------------
;Выполняем запись в EEPROM
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH FLAGS

	LDD FLAGS,Y+_DRV_ERING_RING_SIZE
	MOV LOOP_CNTR,FLAGS
	LDD ACCUM,Y+_DRV_ERING_ELEMENT_POS
_DRV_ERING_OP_FLUSH__LOOP:
	MCALL _DRV_ERING_WRITE_VALUE
	INC ACCUM
	CP ACCUM,FLAGS
	BRNE PC+0x02
	LDI ACCUM,0x00
	STD Y+_DRV_ERING_ELEMENT_POS,ACCUM

	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_ERING_OP_FLUSH__LOOP_NEXT
	LDI ACCUM,_DRV_ERING_ST_NEW_DATA
	STD Y+_DRV_ERING_STATE,ACCUM
	ADIW YL,_DRV_ERING_UPTIME
	MCALL C5_UPTIME_COPY
	RJMP _DRV_ERING_OP_FLUSH__END
_DRV_ERING_OP_FLUSH__LOOP_NEXT:
	DEC LOOP_CNTR
	BRNE _DRV_ERING_OP_FLUSH__LOOP

	LDI TEMP,DRV_RESULT_ERROR
_DRV_ERING_OP_FLUSH__END:

	POP FLAGS
	POP ACCUM
	POP LOOP_CNTR
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
	PUSH_Z
	PUSH_T16
	PUSH FLAGS
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI FLAGS,0x01

	LDD LOOP_CNTR,Y+_DRV_ERING_ELEMENT_SIZE
	MCALL _DRV_ERING_OFFSET
	MOVW ZL,YL
	ADIW ZL,_DRV_ERING_VALUE
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
_DRV_ERING_WRITE_VALUE__WLOOP:
	LD TEMP,-Z
	MCALL EEPROM_WRITE_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	COM TEMP
	MCALL EEPROM_WRITE_BYTE
	ADD TEMP_L,FLAGS
	ADC TEMP_H,C0x00
	DEC LOOP_CNTR
	BRNE _DRV_ERING_WRITE_VALUE__WLOOP

	LDD LOOP_CNTR,Y+_DRV_ERING_ELEMENT_SIZE
	MCALL _DRV_ERING_OFFSET
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
_DRV_ERING_WRITE_VALUE__RLOOP:
	LD ACCUM,-Z
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
	POP_Z
	RET
.endif
