;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2022	konstantin@5277.ru			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер сенсора, опрос каждыйе 100мс(одного порта), порты отпрашиваются по очереди
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_SENSORS
.else
.set DEF__C5_DRV_SENSORS = 1
.message "Included driver software SENSORS v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./mem/ram_fill.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SENSORS_QUEUE_MAX_SIZE				= 0x05	;Количество кнопок очереди для событий

	.EQU	_DRV_SENSORS_QNT								= 0x00	;1B - Количество портов
	.EQU	_DRV_SENSORS_CNTR								= 0x01	;1B - Счетчик опроса
	.EQU	_DRV_SENSORS_QUEUE_SIZE						= 0x02	;1B - Кол-во элементов в очереди
	.EQU	_DRV_SENSORS_QUEUE							= 0x03	;_DRV_SENSORS_QUEUE_MAX_SIZE - очередь событий [PORT]
	.EQU	_DRV_SENSORS_PORTS							= _DRV_SENSORS_QUEUE+_DRV_SENSORS_QUEUE_MAX_SIZE	;xB - Порты(1 байт на порт)

	;---OPERATIONS---
	.EQU	DRV_SENSORS_OP_SET							= 0x00	;Задаем порт
	.EQU	DRV_SENSORS_OP_GET							= 0x01	;Считываем нажатие
	.EQU	DRV_SENSORS_OP_WAIT							= 0x02	;Ожидаем нажатие
	.EQU	DRV_SENSORS_OP_CLEAR							= 0x03	;Очищаем очередь нажатий

;--------------------------------------------------------
DRV_SENSORS_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: LOOP_CNTR-количество портов, FLAGS-ид таймера
;--------------------------------------------------------
	;Выделяем память
	MOV ACCUM,LOOP_CNTR
	SUBI ACCUM,(0x100-_DRV_SENSORS_PORTS)
	MCALL C5_RAM_REALLOC
	;Записываем длину
	STD Y+_DRV_SENSORS_QNT,LOOP_CNTR
	STD Y+_DRV_SENSORS_CNTR,C0x00
	;Заполняем
	MOVW XL,YL
	ADIW XL,_DRV_SENSORS_QUEUE
	SUBI ACCUM,(_DRV_SENSORS_QUEUE)
	MOV LOOP_CNTR,ACCUM
	LDI TEMP,0xff
	MCALL RAM_FILL

	;Настраиваем и запускаем таймер
	MOV TEMP,FLAGS
	LDI ACCUM,0x32|0x80												;Каждые 100мс
	LDI TEMP_H,high(_DRV_SENSORS_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_SENSORS_TIMER_EVENT)
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	;Завершаем инициализацию
	MCALL C5_READY


;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_SENSORS_OP_SET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SENSORS_OP_SET
	RJMP _DRV_SENSORS_PROC_END
	CPI FLAGS,DRV_SENSORS_OP_GET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SENSORS_OP_GET
	RJMP _DRV_SENSORS_PROC_END
	CPI FLAGS,DRV_SENSORS_OP_WAIT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SENSORS_OP_WAIT
	RJMP _DRV_SENSORS_PROC_END
	CPI FLAGS,DRV_SENSORS_OP_CLEAR
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SENSORS_OP_CLEAR
	RJMP _DRV_SENSORS_PROC_END

_DRV_SENSORS_PROC_END:
	POP_Y
	RET

;--------------------------------------------------------
_DRV_SENSORS_OP_SET:
;--------------------------------------------------------
;Добавляем порт
;IN: ACCUM-порт кнопки, LOOP_CNTR-номер сенсора
;--------------------------------------------------------
	ADIW YL,_DRV_SENSORS_PORTS
	ADD YL,LOOP_CNTR
	ADC YH,C0x00
	ST Y,ACCUM
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	RET


;--------------------------------------------------------
_DRV_SENSORS_OP_WAIT:
;--------------------------------------------------------
;Ожидаем нажатие
;OUT: TEMP_H-порт
;--------------------------------------------------------
_DRV_SENSORS_OP_WAIT__LOOP:
	LDD TEMP_H,Y+_DRV_SENSORS_QUEUE_SIZE
	CPI TEMP_H,0x00
	BRNE _DRV_SENSORS_OP_GET
	MCALL C5_WAIT
	RJMP _DRV_SENSORS_OP_WAIT__LOOP

;--------------------------------------------------------
_DRV_SENSORS_OP_GET:
;--------------------------------------------------------
;Считываем нажатие
;OUT: TEMP_H-порт(0xff-нет нажатий)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	LDI TEMP_H,0xff
	;Декрементируем размер очереди
	LDS ACCUM,SREG
	CLI
	LDD LOOP_CNTR,Y+_DRV_SENSORS_QUEUE_SIZE
	CPI LOOP_CNTR,0x00
	BREQ _DRV_SENSORS_PULL_QUEUE__END
	DEC LOOP_CNTR
	STD Y+_DRV_SENSORS_QUEUE_SIZE,LOOP_CNTR

	LDI XH,0x00
	LDI XL,_DRV_SENSORS_QUEUE
	ADD XL,LOOP_CNTR
	ADD XL,YL
	ADC XH,YH
	LD TEMP_H,X

	;Если очередь не пуста, то сдвигаем данные очереди
	CPI LOOP_CNTR,0x00
	BREQ _DRV_SENSORS_PULL_QUEUE__END
	LDI XH,0x00
	LDI XL,_DRV_SENSORS_QUEUE+0x01
	ADD XL,YL
	ADC XH,YH
	MOVW ZL,YL
	ADIW ZL,_DRV_SENSORS_QUEUE
	MCALL RAM_COPY
_DRV_SENSORS_PULL_QUEUE__END:
	STS SREG,ACCUM

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_SENSORS_OP_CLEAR:
;--------------------------------------------------------
;Очищаем очередь нажатий
;--------------------------------------------------------
	STD Y+_DRV_SENSORS_QUEUE_SIZE,C0x00
	RET

;--------------------------------------------------------
_DRV_SENSORS_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик таймера
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH_X
	PUSH ACCUM
	PUSH FLAGS
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	LDD FLAGS,Y+_DRV_SENSORS_QNT
	CPI FLAGS,0x00
	BREQ _DRV_SENSORS_TIMER_EVENT__END

	LDD LOOP_CNTR,Y+_DRV_SENSORS_CNTR
	MOVW ZL,YL
	ADIW ZL,_DRV_SENSORS_PORTS
	ADD ZL,LOOP_CNTR
	ADC ZH,C0x00
	LD ACCUM,Z
	CPI ACCUM,0xff
	BREQ _DRV_SENSORS_TIMER_EVENT__NEXT

	;Опрашиваем сенсор
	PUSH ACCUM
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,0x10
	MCALL PORT_OFFSETS
	CLR ZH
	MOV ZL,TEMP_H
	CLR XH
	MOV XL,TEMP
	MOV TEMP,ACCUM
	COM TEMP
	;Выключаем подтяжку
	LD TEMP_L,Z
	AND TEMP_L,TEMP
	ST Z,TEMP_L
	;Отсчитываем время проверяя порт
DRV_SENSORS_TIMER_EVENT__DETECT_LOOP:
	LD TEMP,X
	AND TEMP,ACCUM
	BREQ DRV_SENSORS_TIMER_EVENT__DETECT_DONE
	DEC LOOP_CNTR
	BRNE DRV_SENSORS_TIMER_EVENT__DETECT_LOOP
DRV_SENSORS_TIMER_EVENT__DETECT_DONE:
	;Включаем подтяжку
	LD TEMP_L,Z
	OR TEMP_L,ACCUM
	ST Z,TEMP_L

	POP ACCUM
	CPI LOOP_CNTR,0x00
	POP LOOP_CNTR
	BREQ _DRV_SENSORS_TIMER_EVENT__NEXT

	;Записываем в очередь, если было срабатывание
	LDD ZL,Y+_DRV_SENSORS_QUEUE_SIZE
	CPI ZL,_DRV_SENSORS_QUEUE_MAX_SIZE
	BREQ _DRV_SENSORS_TIMER_EVENT__NEXT							;Очередь заполнена
	INC ZL
	CLR ZH
	ADD ZL,YL
	ADC ZH,YH
	ADIW ZL,_DRV_SENSORS_QUEUE
	ST Z,ACCUM

_DRV_SENSORS_TIMER_EVENT__NEXT:
	INC LOOP_CNTR
	CP LOOP_CNTR,FLAGS
	BRNE PC+0x02
	CLR LOOP_CNTR
	STD Y+_DRV_SENSORS_CNTR,LOOP_CNTR

_DRV_SENSORS_TIMER_EVENT__END:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	POP FLAGS
	POP ACCUM
	POP_X
	RET
.endif
