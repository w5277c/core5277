
;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;15.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_ADC_ITERACTIONS_QNT						= 0x10;Количество опросов для усреднения показания
	.EQU	_DRV_ADC_RAM_INIT_SIZE							= 0x02;Размер изначально используемой памяти
	.EQU	_DRV_ADC_STRUCT_SIZE								= 0x07;Размер структуры данных для каждого элемента
	;---
	.EQU	_DRV_ADC_ITEMS_QNT								= 0x00;1B - количество элементов
	.EQU	_DRV_ADC_CURRENT_ITEM							= 0x01;1B - текущий элемент
	.EQU	_DRV_ADC_DATA										= 0x02;?B - блок данных

	;---Структура данных---
	.EQU	_DRV_ADC_PORT										= 0x00;1B - ADC порт
	.EQU	_DRV_ADC_COUNTER									= 0x01;1B - Счетчик
	.EQU	_DRV_ADC_SUM										= 0x02;3B - Сумма показаний
	.EQU	_DRV_ADC_RESULT									= 0x05;2B - Результат (0xffff - нет результата)
	;---OPERATIONS---
	.EQU	DRV_ADC_OP_ADD										= 0x00
	.EQU	DRV_ADC_OP_GET										= 0x01

;--------------------------------------------------------
DRV_ADC_INIT:
;--------------------------------------------------------
;Инициализация
;IN - ACCUM - опорное напряжение VREF(ADC_VREF_...)
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_ADC_RAM_INIT_SIZE
	MCALL CORE5277_RAM_REALLOC

	LDI TEMP,0x00
	STD Z+_DRV_ADC_ITEMS_QNT,TEMP
	STD Z+_DRV_ADC_CURRENT_ITEM,TEMP

.IFDEF ADCSRB
	CLR TEMP
	STS ADCSRB,TEMP													;Одноразовое конвертирование без автоматического повторения (ADATE=0)
.ENDIF
	LDI TEMP,(0<<ADLAR)												;(0<<ADLAR) - направление представления байт (right)
	OR TEMP,ACCUM
	STS ADMUX,TEMP
	LDI TEMP,(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(0<<ADEN)|(1<<ADSC)|(1<<ADIE)
																			;(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0) - делитель 128
																			;(0<<ADEN) - АЦП выключен
																			;(1<<ADSC) - Одноразовый старт конверитрования
																			;(1<<ADIE) - Разрешаем прерывание по окончанию кнвертирования
	STS ADCSRA,TEMP													;(0<<ADATE) - Выключен режим AutoTriggering

	;Подаю питание на ADC
	_CORE5277_PRR_RESET PRSDC

	;Активируем прерывания
	LDI TEMP_H,high(_DRV_ADC_EVENT)
	LDI TEMP_L,low(_DRV_ADC_EVENT)
	LDI ACCUM,CORE5277_IR_ADC
	MCALL CORE5277_IR_VECTOR_SET

	;Завершаем инициализацию
	MCALL CORE5277_READY
;--------------------------------------------------------
;Основной код, активируем порт.
;IN: ACCUM - ADC порт(ADC0-ADC7)
;FLAGS - тип операции
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL CORE5277_RAM_OFFSET

	CPI FLAGS,DRV_ADC_OP_ADD
	BRNE PC+0x04
	MCALL _DRV_ADC_OP_ADD
	RJMP _DRV_ADC_PROC_END
	CPI FLAGS,DRV_ADC_OP_GET
	BRNE PC+0x04
	MCALL _DRV_ADC_OP_GET

_DRV_BUTTONS_PROC_END:
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_ADC_OP_ADD:
;--------------------------------------------------------
;Добавляем порт
;IN: ACCUM - ADC порт(ADC0-ADC7)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_X

	;Увеличиваем размер выделенной памяти на _DRV_ADC_STRUCT_SIZE
	PUSH ACCUM
	LDI ACCUM,_DRV_ADC_STRUCT_SIZE
	MCALL CORE5277_RAM_EXTEND
	POP ACCUM
	PUSH_X

;TODO

	POP_X
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_ADC_OP_GET:
;--------------------------------------------------------
;Считываем показание
;OUT: TEMP_H, TEMP_L - показание(0xffff - нет показания)
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
