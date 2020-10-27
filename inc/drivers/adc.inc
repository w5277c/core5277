;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_ADC_RAM_INIT_SIZE							= 0x03;Размер изначально используемой памяти
	.EQU	_DRV_ADC_STRUCT_SIZE								= 0x0b;Размер структуры данных для каждого элемента

	;---PRESCALLER---
	.EQU	DRV_ADC_PRESC_X1									= 0x00
	.EQU	DRV_ADC_PRESC_X2									= 0x01
	.EQU	DRV_ADC_PRESC_X4									= 0x02
	.EQU	DRV_ADC_PRESC_X8									= 0x03
	.EQU	DRV_ADC_PRESC_X16									= 0x04
	.EQU	DRV_ADC_PRESC_X32									= 0x05
	.EQU	DRV_ADC_PRESC_X64									= 0x06
	.EQU	DRV_ADC_PRESC_X128								= 0x07

	;---DATA---
	.EQU	_DRV_ADC_TIMER_ID									= 0x00;1B - ид таймера
	.EQU	_DRV_ADC_ITEMS_QNT								= 0x01;1B - количество элементов
	.EQU	_DRV_ADC_CURRENT_ITEM							= 0x02;1B - текущий элемент
	.EQU	_DRV_ADC_DATA										= 0x03;?B - блок данных

	;---Структура данных---
	.EQU	_DRV_ADC_PORT										= 0x00;1B - ADC порт
	.EQU	_DRV_ADC_TIMER_PERIOD							= 0x01;1B - период таймера
	.EQU	_DRV_ADC_ITER_QNT									= 0x02;1B - количество итераций (64 максимум)
	.EQU	_DRV_ADC_ITER_CNT									= 0x03;1B - счетчик итераций
	.EQU	_DRV_ADC_PRESCALLER								= 0x04;1B - делитель
	.EQU	_DRV_ADC_VREF										= 0x05;1B - опорное напряжение
	.EQU	_DRV_ADC_SUM										= 0x06;3B - сумма показаний
	.EQU	_DRV_ADC_RESULT									= 0x09;2B - результат (0xffff - нет результата, 15-бит - свежий результат)
	;---OPERATIONS---
	.EQU	DRV_ADC_OP_ADD										= 0x00
	.EQU	DRV_ADC_OP_GET										= 0x01
	.EQU	DRV_ADC_OP_WAIT									= 0x02

;--------------------------------------------------------
DRV_ADC_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	PUSH ACCUM
	;Выделяем память
	LDI ACCUM,_DRV_ADC_RAM_INIT_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	STD Z+_DRV_ADC_TIMER_ID,ACCUM
	LDI ACCUM,0x00
	STD Z+_DRV_ADC_ITEMS_QNT,ACCUM
	STD Z+_DRV_ADC_CURRENT_ITEM,ACCUM
.IFDEF ADCSRB
	STS ADCSRB,ACCUM
.ENDIF
	STS ADMUX,ACCUM
	STS ADCSRA,ACCUM

	;Подаю питание на ADC
	_C5_PRR_RESET PRSDC

	;Активируем прерывания
	LDI TEMP_H,high(_DRV_ADC_EVENT)
	LDI TEMP_L,low(_DRV_ADC_EVENT)
	LDI ACCUM,C5_IR_ADC
	MCALL C5_IR_VECTOR_SET

	;Завершаем инициализацию
	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, определяем тип операции.
;IN: FLAGS - код операции DRV_ADC_OP_GET...
;Остальные параметры см. в обработчике операций
;--------------------------------------------------------
	PUSH TEMP

	CPI FLAGS,DRV_ADC_OP_ADD
	BRNE PC+0x04
	MCALL _DRV_ADC_OP_ADD
	RJMP _DRV_ADC_PROC_END
	CPI FLAGS,DRV_ADC_OP_GET
	BRNE PC+0x04
	MCALL _DRV_ADC_OP_GET
	RJMP _DRV_ADC_PROC_END
	CPI FLAGS,DRV_ADC_OP_WAIT
	BRNE PC+0x04
	MCALL _DRV_ADC_OP_WAIT
	RJMP _DRV_ADC_PROC_END

_DRV_BUTTONS_PROC_END:
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_ADC_OP_ADD:
;--------------------------------------------------------
;Добавляем порт
;IN: ACCUM - ADC порт(ADC0-ADC7)
;FLAGS - опорное напряжение(VREF), LOOP_CNTR - PRESCALLER,
;TEMP_H - кол-во итераций для вычисления среднего,
;TEMP_L - период таймера для итераций
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z

	;Увеличиваем размер выделенной памяти на _DRV_ADC_STRUCT_SIZE
	PUSH ACCUM
	LDI ACCUM,_DRV_ADC_STRUCT_SIZE
	MCALL C5_RAM_EXTEND
	POP ACCUM

	STD Y+_DRV_ADC_PORT,ACCUM
	STD Y+_DRV_ADC_TIMER_PERIOD,TEMP_L
	STD Y+_DRV_ADC_ITER_QNT,TEMP_H
	STD Y+_DRV_ADC_PRESCALLER,LOOP_CNTR
	STD Y+_DRV_ADC_VREF,FLAGS
	LDI TEMP,0x00
	STD Y+_DRV_ADC_ITER_CNTR,TEMP
	LDI TEMP,0xff
	STD Y+_DRV_ADC_RESULT+0x00,TEMP
	STD Y+_DRV_ADC_RESULT+0x01,TEMP

	LDD TEMP,Z+_DRV_ADC_ITEMS_QNT
	INC TEMP
	STD Z+_DRV_ADC_ITEMS_QNT,TEMP
	CPI TEMP,0x01
	BRNE _DRV_ADC_OP_ADD__NOT_FIRST
	_MCALL _DRV_ADC_START
_DRV_ADC_OP_ADD__NOT_FIRST:

	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_ADC_OP_WAIT:
;--------------------------------------------------------
;Считываем новое показание
;IN: ACCUM - ADC порт(ADC0-ADC7)
;OUT: TEMP_H, TEMP_L - показание(0xffff - нет показания)
;--------------------------------------------------------
_DRV_ADC_OP_WAIT__LOOP:
	MCALL _DRV_ADC_OP_GET
	BRCS _DRV_ADC_OP_WAIT__END
	MCALL C5_WAIT
	RJMP _DRV_ADC_OP_WAIT__LOOP
_DRV_ADC_OP_WAIT__END:
	RET

;--------------------------------------------------------
_DRV_ADC_OP_GET:
;--------------------------------------------------------
;Считываем показание
;IN: ACCUM - ADC порт(ADC0-ADC7)
;OUT: TEMP_H, TEMP_L - показание(0xffff - нет показания)
;флаг C = HI - свежий результат
;--------------------------------------------------------
	PUSH_Z

	LDI TEMP_H,0xff
	LDI TEMP_L,0xff

	MCALL C5_RAM_OFFSET
	LDD LOOP_CNTR,Z+_DRV_ADC_ITEMS_QNT
	ADIW ZL,_DRV_ADC_DATA

_DRV_ADC_OP_GET__LOOP:
	LDD TEMP,Z+_DRV_ADC_PORT
	CP TEMP,ACCUM
	BRNE _DRV_ADC_OP_GET__NEXT

	LDS TEMP,SREG
	CLI
	LDD TEMP_H,Z+_DRV_ADC_RESULT+0x00
	LDD TEMP_L,Z+_DRV_ADC_RESULT+0x01
	STS SREG,TEMP
	MOV TEMP,TEMP_H
	ANDI TEMP_H,0x7f
	LSL TEMP
	RJMP _DRV_ADC_OP_GET__END

_DRV_ADC_OP_GET__NEXT:
	ADIW ZL,_DRV_ADC_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE _DRV_ADC_OP_GET__LOOP

_DRV_ADC_OP_GET__END:
	POP_Z
	RET


;--------------------------------------------------------
_DRV_ADC_START:
;--------------------------------------------------------
;Запускаем ADC
;IN: Y - адрес на блок данных элемента
;--------------------------------------------------------

;TODO

	RET


;--------------------------------------------------------
_DRV_ADC_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик по таймеру
;--------------------------------------------------------

;TODO

	RET
;--------------------------------------------------------
_DRV_ADC_EVENT:
;--------------------------------------------------------
;Обработчик перывания по завершенному вычислению ADC
;--------------------------------------------------------

;TODO

	RET
