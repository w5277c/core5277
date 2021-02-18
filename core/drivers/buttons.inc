;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.03.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;28.10.2020  w5277c@gmail.com        Корректировка таймингов
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_BUTTONS
.else
.set DEF__C5_DRV_BUTTONS = 1
.message "Included driver Buttons v0.1"

;TODO реализовать обработчики для переключаетеля и сенсора
;TODO Фиксируется 5-ое длинное нажатие (вероятно с потерей первого значения) из 4 допустимых


.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/ram/ram_extend.inc"
.include	"./mem/ram_fill8.inc"
.include	"./core/wait.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BUTTONS_STRUCT_SIZE						= 0x04;Размер структуры данных для каждой кнопки
	.EQU	_DRV_BUTTONS_QUEUE_MAX_SIZE					= 0x05;Количество кнопок очереди для событий
	;---
	.EQU	_DRV_BUTTONS_BUTTONS_QNT						= 0x00;1B - Количество кнопок
	.EQU	_DRV_BUTTONS_CNTR									= 0x01;1B - Счетчик опроса
	.EQU	_DRV_BUTTONS_QUEUE_SIZE							= 0x02;1B - Кол-во элементов в очереди
	.EQU	_DRV_BUTTONS_QUEUE								= 0x03;_DRV_BUTTONS_QUEUE_MAX_SIZE*2 - очередь событий [PORT,RESULT]
	.EQU	_DRV_BUTTONS_DATA									= _DRV_BUTTONS_QUEUE+(_DRV_BUTTONS_QUEUE_MAX_SIZE*2);x4B - Данные кнопок

	;---Структура данных по кнопкам---
	.EQU	_DRV_BUTTONS_PORT									= 0x00;Порт кнопки
	.EQU	_DRV_BUTTONS_CNTR_N_FLAGS						= 0x01;Счетчик высоких состояний и флаги(старший ниббл)
	.EQU	_DRV_BUTTONS_TIME									= 0x02;Время отсчета
	.EQU	_DRV_BUTTONS_RESULT								= 0x03;Результат |rqqqssss|, r - повтор, q-кол-во нажатий(1-4), s-состояния(история 0-короткое/1-длинное)
	;---Тайминги---
	.EQU	_DRV_BUTTONS_TIMER_PERIOD						= 0x02;Период таймера (x*0.002) (анализ 32мс)
	.EQU	_DRV_BUTTONS_MAX_CNTR							= 0x08;Количество необходимых итераций перед анализом
	;---
	.EQU	_DRV_BUTTONS_SHORT_PRESS_MAX_TIME			= 300/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR)
	.EQU	_DRV_BUTTONS_HOLD_TIME							= 1000/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR)
	.EQU	_DRV_BUTTONS_TIMEOUT								= 400/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR)
	;---Флаги---
	.EQU	_DRV_BUTTONS_FL_INIT								= 0x04;Только что добавлено(ожидание начала цикла опроса)
	.EQU	_DRV_BUTTONS_FL_FIRST_SCAN						= 0x05;Первый опрос(нет предыдущего занчения)
	.EQU	_DRV_BUTTONS_FL_HOLD								= 0x06;Удерживание
	.EQU	_DRV_BUTTONS_FL_STATE							= 0x07;Текущее состояние порта
	;---OPERATIONS---
	.EQU	DRV_BUTTONS_OP_ADD								= 0x00
	.EQU	DRV_BUTTONS_OP_GET								= 0x01
	.EQU	DRV_BUTTONS_OP_WAIT								= 0x02

	.EQU	_DRV_BUTTONS_RAM_SIZE							= 0x03+(_DRV_BUTTONS_QUEUE_MAX_SIZE*2)
;--------------------------------------------------------
DRV_BUTTONS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH_X
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	PUSH ACCUM
	LDI ACCUM,_DRV_BUTTONS_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	LDI TEMP,0x00
	STD Z+_DRV_BUTTONS_QUEUE_SIZE,TEMP
	STD Z+_DRV_BUTTONS_BUTTONS_QNT,TEMP
	STD Z+_DRV_BUTTONS_CNTR,TEMP

	LDI LOOP_CNTR,_DRV_BUTTONS_QUEUE_MAX_SIZE*2
	CLR XH
	LDI XL,_DRV_BUTTONS_QUEUE
	ADD XL,ZL
	ADC XH,ZH
	MCALL RAM_FILL8

	;Инициализирую таймер с периодом в 0.002*_DRV_BUTTONS_TIMER_PERIOD
	MOV TEMP,ACCUM
	LDI TEMP_H,high(DRV_BUTTONS_TIMER_EVENT)
	LDI TEMP_L,low(DRV_BUTTONS_TIMER_EVENT)
	LDI ACCUM,_DRV_BUTTONS_TIMER_PERIOD|0x80
	MCALL C5_TIMER_SET
	;Запускаю таймер
	MCALL C5_TIMER_START

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	POP_X
	POP_Z
	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS - тип операции
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_BUTTONS_OP_ADD
	BRNE PC+0x04
	MCALL _DRV_BUTTONS_OP_ADD
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_GET
	BRNE PC+0x04
	MCALL _DRV_BUTTONS_OP_GET
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_WAIT
	BRNE PC+0x04
	MCALL _DRV_BUTTONS_OP_WAIT

_DRV_BUTTONS_PROC_END:
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_ADD:
;--------------------------------------------------------
;Добавляем кнопку
;IN: ACCUM - порт кнопки(7-бит - включать подтяжку)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_Y

	;Увеличиваем размер выделенной памяти на _DRV_BUTTONS_STRUCT_SIZE
	PUSH ACCUM
	LDI ACCUM,_DRV_BUTTONS_STRUCT_SIZE
	MCALL C5_RAM_EXTEND
	POP ACCUM

	LDD TEMP,Z+_DRV_BUTTONS_BUTTONS_QNT
	INC TEMP
	STD Z+_DRV_BUTTONS_BUTTONS_QNT,TEMP

	;Сбрасываем флаг подтяжки
	MOV TEMP,ACCUM
	ANDI ACCUM,0x7f
	STD Y+_DRV_BUTTONS_PORT,ACCUM
	;Порт на вход
	MCALL PORT_MODE_IN
	SBRS TEMP,0x07
	RJMP PC+0x03
	;Включаем подтяжку, если необходимо
	MCALL PORT_SET_HI

	;Записываем данные кнопки
	LDI TEMP,(1<<_DRV_BUTTONS_FL_INIT)
	STD Y+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP
	LDI TEMP,0x00
	STD Y+_DRV_BUTTONS_TIME,TEMP
	STD Y+_DRV_BUTTONS_RESULT,TEMP

	POP_Y
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_WAIT:
;--------------------------------------------------------
;Ожидаем нажатие
;OUT: TEMP_H - порт, TEMP_L - результат
;--------------------------------------------------------
_DRV_BUTTONS_OP_WAIT__LOOP:
	LDD TEMP,Z+_DRV_BUTTONS_QUEUE_SIZE
	CPI TEMP,0x00
	BRNE _DRV_BUTTONS_OP_GET
	MCALL C5_WAIT
	RJMP _DRV_BUTTONS_OP_WAIT__LOOP

;--------------------------------------------------------
_DRV_BUTTONS_OP_GET:
;--------------------------------------------------------
;Считываем нажатие
;OUT: TEMP_H - порт(0xff - нет нажатий),
;TEMP_L - результат
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_X

	LDI TEMP_H,0xff
	;Декрементируем размер очереди
	LDS ACCUM,SREG
	CLI
	LDD LOOP_CNTR,Z+_DRV_BUTTONS_QUEUE_SIZE
	CPI LOOP_CNTR,0x00
	BREQ _DRV_BUTTONS_PULL_QUEUE__END
	DEC LOOP_CNTR
	STD Z+_DRV_BUTTONS_QUEUE_SIZE,LOOP_CNTR

	LDI XH,0x00
	LDI XL,_DRV_BUTTONS_QUEUE
	LSL LOOP_CNTR
	ADD XL,LOOP_CNTR
	ADD XL,ZL
	ADC XH,ZH
	LD TEMP_H,X+
	LD TEMP_L,X
	;Если очередь не пуста, то сдвигаем данные очереди
	CPI LOOP_CNTR,0x00
	BREQ _DRV_BUTTONS_PULL_QUEUE__END
	LDI XH,0x00
	LDI XL,_DRV_BUTTONS_QUEUE+0x02
	ADD XL,ZL
	ADC XH,ZH
	PUSH_Z
	ADIW ZL,_DRV_BUTTONS_QUEUE
	MCALL RAM_COPY8
	POP_Z
_DRV_BUTTONS_PULL_QUEUE__END:
	STS SREG,ACCUM

	POP_X
	POP LOOP_CNTR
	POP ACCUM
	RET

;--------------------------------------------------------
DRV_BUTTONS_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру(анализ портов)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L

	MCALL C5_RAM_OFFSET
	LDD LOOP_CNTR,Z+_DRV_BUTTONS_BUTTONS_QNT

	;Инкрементирую счетчик опроса
	LDD TEMP,Z+_DRV_BUTTONS_CNTR
	CPI TEMP,_DRV_BUTTONS_MAX_CNTR
	BREQ DRV_BUTTONS_TIMER_EVENT__ANALYZE
	INC TEMP
	STD Z+_DRV_BUTTONS_CNTR,TEMP

	;Смещаемся на данные первой кнопки
	ADIW ZL,_DRV_BUTTONS_DATA

	;Читаем состояние портов для каждой кнопки
DRV_BUTTONS_TIMER_EVENT__READ_LOOP:
	LDD ACCUM,Z+_DRV_BUTTONS_PORT
	MCALL PORT_GET
	BRCS DRV_BUTTONS_TIMER_EVENT__LO_STATE
	;Инкрементирую счетчик
	LDD TEMP,Z+_DRV_BUTTONS_CNTR_N_FLAGS
	MOV TEMP_L,TEMP
	ANDI TEMP,0xf0
	ANDI TEMP_L,0x0f
	INC TEMP_L
	OR TEMP,TEMP_L
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP
DRV_BUTTONS_TIMER_EVENT__LO_STATE:

	ADIW ZL,_DRV_BUTTONS_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE DRV_BUTTONS_TIMER_EVENT__READ_LOOP
	RJMP DRV_BUTTONS_TIMER_EVENT__END

DRV_BUTTONS_TIMER_EVENT__ANALYZE:
	;Сбрасываю счетчик, если отсчет завершен
	LDI TEMP,0x00
	STD Z+_DRV_BUTTONS_CNTR,TEMP

	;Запоминаем Z в Y, для доступа к основным параметрам типа _DRV_BUTTONS_QUEUE_SIZE
	MOV YH,ZH
	MOV YL,ZL

	;Смещаемся на данные первой кнопки
	ADIW ZL,_DRV_BUTTONS_DATA

DRV_BUTTONS_TIMER_EVENT__ANALYZE_LOOP:
	;Проверяю на только что добавленную кнопку
	LDD TEMP_H,Z+_DRV_BUTTONS_CNTR_N_FLAGS
	;Считываем предыдущее состояние порта в T
	BST TEMP_H,_DRV_BUTTONS_FL_STATE
	MOV TEMP_L,TEMP_H
	;Сбрасываем счетчик и состояние в 0
	ANDI TEMP_H,low(~((1<<_DRV_BUTTONS_FL_STATE)|0x0f))
	ANDI TEMP_L,0x0f
	LDI TEMP,_DRV_BUTTONS_MAX_CNTR/2
	CP TEMP,TEMP_L
	BRCC PC+0x02
	;Устанавливаем состояние в 1
	ORI TEMP_H,(1<<_DRV_BUTTONS_FL_STATE)
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP_H

;TODO Нужно учитывать, что кнопка может быть нажата до запуса МК, не обрабатываем пока не отпустят

	;Проверяем на только что добавленную кнопку
	SBRS TEMP_H,_DRV_BUTTONS_FL_INIT
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_INIT
	CLR TEMP_H
	STD Z+_DRV_BUTTONS_TIME,TEMP_H
	STD Z+_DRV_BUTTONS_RESULT,TEMP_H
	ORI TEMP_H,(1<<_DRV_BUTTONS_FL_FIRST_SCAN)
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP_H
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_INIT:
	;Проверяем на первое сканирование
	SBRS TEMP_H,_DRV_BUTTONS_FL_FIRST_SCAN
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_FIRST_SCAN
	;Если кнопка нажата, то остаемся в режиме первого сканирования
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Иначе, конпка не нажата, сбрасываем флаг первого сканирования
	CLR TEMP_H
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP_H
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_FIRST_SCAN:
	;Флаг T хранит старое значение, флаг C хранит новое(true-нажата)
	ROL TEMP
	BLD TEMP_L,0x00
	;Записываем в флаг T новое состояние
	BST TEMP,0x00
	EOR TEMP,TEMP_L
	SBRC TEMP,0x00
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_STATE_CHANGED

	;Увеличиваем время нахождения в одном и том-же состоянии
	LDD TEMP_L,Z+_DRV_BUTTONS_TIME
	CPI TEMP_L,0xff
	BREQ PC+0x02
	INC TEMP_L
	STD Z+_DRV_BUTTONS_TIME,TEMP_L

	;Проверка на таймауты
	;Считываем результат
	LDD TEMP_H,Z+_DRV_BUTTONS_RESULT
	;Переходим, если кнопка нажата
	BRTS _DRV_BUTTONS_TIMER_EVENT__ANALYZE_BUTTON_PRESSED
	;Кнопка не нажата, проверяем таймаут если нажатия были
	MOV TEMP,TEMP_H
	ANDI TEMP,0b01110000
	;Если нажатий не было, проверяем следующую кнопку
	BRNE PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	CPI TEMP_L,_DRV_BUTTONS_TIMEOUT
	;Таймаут не достигнут
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	MCALL _DRV_BUTTONS__QUEUE_ADD
	LDI TEMP,0x00
	STD Z+_DRV_BUTTONS_RESULT,TEMP
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_BUTTON_PRESSED:
	CPI TEMP_L,_DRV_BUTTONS_HOLD_TIME
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Пропускаем блок, если r уже установлен
	SBRC TEMP_H,0x07
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_LONG_PRESS_REPEATE
	;Заполняем переменные и вызываем обработчик
	MOV TEMP,TEMP_H
	;Инкрементируем количество нажатий
	SWAP TEMP
	ANDI TEMP,0b00000111
	INC TEMP
	SWAP TEMP
	;Добавляем длинное нажатие
	SEC
	ROL TEMP_H
	ANDI TEMP_H,0b00001111
	OR TEMP_H,TEMP
	;Включаем r(повтор)
	ORI TEMP_H,0b10000000

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_LONG_PRESS_REPEATE:
	MCALL _DRV_BUTTONS__QUEUE_ADD
	;Сбраываем, оставляя бит состояния порта включенным
	LDI TEMP,(1<<_DRV_BUTTONS_FL_STATE)
	STD Z+_DRV_BUTTONS_CNTR_N_FLAGS,TEMP
	;Не сбрасываем результат, для следующего события длинного нажатия(если кнопку так и не отпустили)
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_STATE_CHANGED:
	;Считываю и сбрасываю счетчик нахождения в одном состоянии
	LDD TEMP_L,Z+_DRV_BUTTONS_TIME
	CLR TEMP
	STD Z+_DRV_BUTTONS_TIME,TEMP
	;Если кнопку нажали, то просто выходим
	BRTS _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

	;Получаем результат
	LDD TEMP_H,Z+_DRV_BUTTONS_RESULT
	;Получаем количество нажатых кнопок
	MOV TEMP,TEMP_H
	SWAP TEMP
	ANDI TEMP,0x0f

	;Проверяем на максимальное занчение
	CPI TEMP,0x04
	;Ничего не делаем, если достигнут максимум
	BREQ _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Увеличиваем счетчик нажатий
	INC TEMP
	;Добавяем новое нажатие в результат
	PUSH TEMP
	LDI TEMP,_DRV_BUTTONS_SHORT_PRESS_MAX_TIME
	CP TEMP,TEMP_L
	POP TEMP
	ROL TEMP_H
	ANDI TEMP_H,0b00001111
	SWAP TEMP
	OR TEMP_H,TEMP
	STD Z+_DRV_BUTTONS_RESULT,TEMP_H

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT:
	ADIW ZL,_DRV_BUTTONS_STRUCT_SIZE
	DEC LOOP_CNTR
	BREQ PC+0x02
	RJMP DRV_BUTTONS_TIMER_EVENT__ANALYZE_LOOP

DRV_BUTTONS_TIMER_EVENT__END:
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUTTONS__QUEUE_ADD:
;--------------------------------------------------------
;Добавляем элемент в очередь
;IN: Y - Адрес основных переменных, Z - адрес данных
;кнопки, TEMP_H - результат обработки кнопки
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L

	;Достигнут таймаут, добавляем событие в очередь
	LDD TEMP,Y+_DRV_BUTTONS_QUEUE_SIZE
	CPI TEMP,_DRV_BUTTONS_QUEUE_MAX_SIZE
	;Пропускаем добавление, если очередь заполнена
	BREQ _DRV_BUTTONS__QUEUE_ADD_END

	;Вычисляем адрес очереди
	MOV XH,YH
	MOV XL,YL
	ADIW XL,_DRV_BUTTONS_QUEUE

	LDS TEMP_L,SREG
	CLI
	LDD TEMP,Y+_DRV_BUTTONS_QUEUE_SIZE
	;Вычисляю смещение в очереди
	PUSH TEMP
	LSL TEMP
	ADD XL,TEMP
	LDI TEMP,0x00
	ADC XH,TEMP
	POP TEMP
	INC TEMP
	STD Y+_DRV_BUTTONS_QUEUE_SIZE,TEMP
	;Записываю кнопку в очередь
	LDD TEMP,Z+_DRV_BUTTONS_PORT
	ST X+,TEMP															;Порт кнопки
	ST X,TEMP_H															;Результат
	STS SREG,TEMP_L
	;Очищаю данные кнопки
	CLR TEMP
	STD Z+_DRV_BUTTONS_TIME,TEMP
_DRV_BUTTONS__QUEUE_ADD_END:
	POP TEMP_L
	POP TEMP
	RET
.endif
