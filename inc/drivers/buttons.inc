;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;02.03.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать обработчики для переключаетеля и сенсора
;TODO реализовать очередь и процедуру ее чтения(с ожиданием нажатия кнопки и без ожидания)
;---CONSTANTS--------------------------------------------
	.EQU	DRV_BUTTONS__STRUCT_SIZE						= 0x05;Размер структуры данных для каждой кнопки
	;---
	.EQU	DRV_BUTTONS__VAR_BUTTONS_QNT					= 0x00;1b - Количество кнопок
	.EQU	DRV_BUTTONS__HANDLER_OFFSET					= 0x01;2b - Адрес обработчика кнопок(0x0000 - не задан, использую очередь)
	.EQU	DRV_BUTTONS__VAR_MODE							= 0x03;1b - Режим работы
	.EQU	DRV_BUTTONS__VAR_CNTR							= 0x04;1b - Счтетчик опроса(1ед = 10мс)
	.EQU	DRV_BUTTONS__VAR_RESULT_QUEUE					= 0x05;8b - Очередь резултатов кнопок

	;---Режим работы---
	.EQU	DRV_BUTTONS__MODE_BUTTONS						= 0x00;
	.EQU	DRV_BUTTONS__MODE_SWITCHES						= 0x01;
	.EQU	DRV_BUTTONS__MODE_SENSORS						= 0x02;
	;---
	.EQU	DRV_BUTTONS__SHORT_PRESS_MAX_TIME			= 0x30;0.480сек
	.EQU	DRV_BUTTONS__HOLD_TIME							= 0x60;0.960сек
	.EQU	DRV_BUTTONS__TIMEOUT								= 0x24;0.360сек
	;---
	.EQU	DRV_BUTTONS__MAX_CNTR							= 0x08;Количество необходимых итераций перед анализом
	;---Структура данных по кнопкам---
	.EQU	DRV_BUTTONS__PORT									= 0x00;Порт (PORTB,PORTC,PORTD)
	.EQU	DRV_BUTTONS__PIN									= 0x01;Пин (1,2,4,8,16,32,64,128)
	.EQU	DRV_BUTTONS__CNTR_N_FLAGS						= 0x02;Счетчик высоких состояний и флаги(старший ниббл)
	.EQU	DRV_BUTTONS__TIME									= 0x03;Время отсчета
	.EQU	DRV_BUTTONS__RESULT								= 0x04;Результат |rqqqssss|, r - повтор, q-кол-во нажатий(1-4), s-состояния(история 0-короткое/1-длинное)

	;---Флаги---
	.EQU	DRV_BUTTONS__FL_INIT								= 0x04;Только что добавлено(ожидание начала цикла опроса)
	.EQU	DRV_BUTTONS__FL_FIRST_SCAN						= 0x05;Первый опрос(нет предыдущего занчения)
	.EQU	DRV_BUTTONS__FL_HOLD								= 0x06;Удерживание
	.EQU	DRV_BUTTONS__FL_STATE							= 0x07;Текущее состояние порта

;--------------------------------------------------------
DRV_BUTTONS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - количество обрабатываемых кнопок,
;TEMP - режим работы
;TEMP_H,TEMP_L - адрес обработчика
;--------------------------------------------------------
	MOV r0,ACCUM														;DRV_BUTTONS__STRUCT_SIZE
	LSL r0
	LSL r0
	ADD r0,ACCUM

	PUSH TEMP
	;Устанавливаю количество кнопок
	LDI TEMP,DRV_BUTTONS__VAR_BUTTONS_QNT
	MCALL CORE5277_VAR_SET
	POP TEMP
	;Устанавливаю режим работы
	MOV ACCUM,TEMP
	LDI TEMP,DRV_BUTTONS__VAR_MODE
	MCALL CORE5277_VAR_SET
	;Устанавливаю адрес обработчика кнопок
	LDI TEMP,DRV_BUTTONS__HANDLER_OFFSET
	MCALL CORE5277_VAR_SET16

	;Инициализирую таймер (0.001s)
	LDS TEMP,PRR
	ANDI TEMP,~(1<<PRTIM2)											;Подпитываю TIMER2
	STS PRR,TEMP
	LDI TEMP,(1<<WGM21)|(0<<WGM20)
	STS TCCR2A,TEMP
	LDI TEMP,(1<<CS22)|(0<<CS21)|(0<<CS20)						;/64 - for 16MHz
	STS TCCR2B,TEMP
	CLR TEMP
	STS TCNT2,TEMP
	LDI TEMP,0xf9														;0xf9 - 250(16MHz)
	STS OCR2A,TEMP
	LDI TEMP,(1<<OCF2A)
	OUT TIFR2,TEMP
	LDI TEMP,(1<<OCIE2A)|(1<<TOIE2)
	STS TIMSK2,TEMP
	;Устанавливаем вектор прерывания
	LDI TEMP,CORE5277_IR_TIMER2_COMPA
	LDI TEMP_H,high(DRV_BUTTONS_TIMER_EVENT)
	LDI TEMP_L,low(DRV_BUTTONS_TIMER_EVENT)
	MCALL CORE5277_IR_VECTOR_SET

	LDI ACCUM,(1<<CORE5277_MODEBITS_DRV_BSEM)
	MCALL CORE5277_DRV_READY
;--------------------------------------------------------
;Основной код, отсуствует
;--------------------------------------------------------
	RET

;--------------------------------------------------------
DRV_BUTTONS_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру(анализ портов)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP,DRV_BUTTONS__VAR_BUTTONS_QNT
	MCALL CORE5277_VAR_GET
	MOV LOOP_CNTR,ACCUM

	;Инкрементирую счетчик опроса
	LDI TEMP,DRV_BUTTONS__VAR_CNTR
	MCALL CORE5277_VAR_GET
	INC ACCUM
	MCALL CORE5277_VAR_SET

	MCALL CORE5277_DRV_GET_DATAOFFSET
	;Читаем состояние портов для каждой кнопки
DRV_BUTTONS_TIMER_EVENT__READ_LOOP:
	LDD TEMP_H,Z+DRV_BUTTONS__PORT
	LDD TEMP_L,Z+DRV_BUTTONS__PIN
	MCALL CORE5277_PORTS__GET
	BRCS DRV_BUTTONS_TIMER_EVENT__LO_STATE
	;Инкрементирую счетчик
	LDD TEMP_H,Z+DRV_BUTTONS__CNTR_N_FLAGS
	MOV TEMP_L,TEMP_H
	ANDI TEMP_H,0xf0
	ANDI TEMP_L,0x0f
	INC TEMP_L
	OR TEMP_H,TEMP_L
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP_H
DRV_BUTTONS_TIMER_EVENT__LO_STATE:
	CPI ACCUM,DRV_BUTTONS__MAX_CNTR
	BRNE PC+0x03
	MCALL _DRV_BUTTONS_ANALYZE

	LDI TEMP,DRV_BUTTONS__STRUCT_SIZE
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	DEC LOOP_CNTR
	BRNE DRV_BUTTONS_TIMER_EVENT__READ_LOOP

	;Сбрасываю счетчик, если отсчет завершен
	CPI ACCUM,DRV_BUTTONS__MAX_CNTR
	BRNE PC+0x05
	LDI TEMP,DRV_BUTTONS__VAR_CNTR
	CLR ACCUM
	MCALL CORE5277_VAR_SET

DRV_BUTTONS_TIMER_EVENT__END:
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUTTONS_ANALYZE:
;--------------------------------------------------------
;Анализ кнопки
;IN: LOOP_CNTR - ид кнопки(1-n),
;Z - смещение на структуру кнопки
;--------------------------------------------------------
	PUSH_Z
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH ACCUM

	;Проверяю на только что добавленную кнопку
	LDD TEMP_H,Z+DRV_BUTTONS__CNTR_N_FLAGS
	;Запоминаем предыдущее состояние порта в T
	BST TEMP_H,DRV_BUTTONS__FL_STATE
	MOV TEMP_L,TEMP_H
	;Сбрасываем счетчик и состояние в 0
	ANDI TEMP_H,low(~((1<<DRV_BUTTONS__FL_STATE)|0x0f))
	ANDI TEMP_L,0x0f
	LDI TEMP,0x04
	CP TEMP,TEMP_L
	BRCC PC+0x02
	;Устанавливаем состояние в 1
	ORI TEMP_H,(1<<DRV_BUTTONS__FL_STATE)
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP_H

;TODO Нужно учитывать, что кнопка может быть нажата до запуса МК, не обрабатываем пока не отпустят

	;Проверяем на только что добавленную кнопку
	SBRS TEMP_H,DRV_BUTTONS__FL_INIT
	RJMP _DRV_BUTTONS_ANALYZE__NOT_INIT
	CLR TEMP_H
	STD Z+DRV_BUTTONS__TIME,TEMP_H
	STD Z+DRV_BUTTONS__RESULT,TEMP_H
	ORI TEMP_H,(1<<DRV_BUTTONS__FL_FIRST_SCAN)
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP_H
	RJMP _DRV_BUTTONS_ANALYZE__END

_DRV_BUTTONS_ANALYZE__NOT_INIT:
	;Проверяем на первое сканирование
	SBRS TEMP_H,DRV_BUTTONS__FL_FIRST_SCAN
	RJMP _DRV_BUTTONS_ANALYZE__NOT_FIRST_SCAN
	;Если кнопка нажата, то остаемся в режиме первого сканирования
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_ANALYZE__END
	;Иначе, конпка не нажата, сбрасываем флаг первого сканирования
	CLR TEMP_H
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP_H
	RJMP _DRV_BUTTONS_ANALYZE__END

_DRV_BUTTONS_ANALYZE__NOT_FIRST_SCAN:
	;Флаг T хранит старое значение, флаг C хранит новое(true-нажата)
	ROL TEMP
	BLD TEMP_L,0x00
	;Записываем в флаг T новое состояние
	BST TEMP,0x00
	EOR TEMP,TEMP_L
	SBRC TEMP,0x00
	RJMP _DRV_BUTTONS_ANALYZE__STATE_CHANGED

	;Увеличиваем время нахождения в одном и том-же состоянии
	LDD TEMP_L,Z+DRV_BUTTONS__TIME
	CPI TEMP_L,0xff
	BREQ PC+0x02
	INC TEMP_L
	STD Z+DRV_BUTTONS__TIME,TEMP_L

	;Проверка на таймауты
	;Считываем результат
	LDD TEMP_H,Z+DRV_BUTTONS__RESULT
	;Переходим, если кнопка нажата
	BRTS _DRV_BUTTONS_ANALYZE__BUTTON_PRESSED
	;Кнопка не нажата, проверяем таймаут если нажатия были
	MOV TEMP,TEMP_H
	ANDI TEMP,0b01110000
	;Если нажатий не было, выходим
	BRNE PC+0x02
	RJMP _DRV_BUTTONS_ANALYZE__END
	CPI TEMP_L,DRV_BUTTONS__TIMEOUT
	;Таймаут не достигнут
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_ANALYZE__END

	;Достигнут таймаут, заполняем переменные и вызываем обработчик
	MOV ACCUM,LOOP_CNTR
	PUSH TEMP_H
	LDI TEMP,DRV_BUTTONS__HANDLER_OFFSET
	MCALL CORE5277_VAR_GET16
	POP TEMP
	PUSH_Z
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	ICALL
	POP_Z
	CLR TEMP
	STD Z+DRV_BUTTONS__TIME,TEMP
	STD Z+DRV_BUTTONS__RESULT,TEMP
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP
	RJMP _DRV_BUTTONS_ANALYZE__END

_DRV_BUTTONS_ANALYZE__BUTTON_PRESSED:
	CPI TEMP_L,DRV_BUTTONS__HOLD_TIME
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_ANALYZE__END
	;Пропускаем блок, если r уже установлен
	SBRC TEMP_H,0x07
	RJMP _DRV_BUTTONS_ANALYZE__LONG_PRESS_REPEATE
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

_DRV_BUTTONS_ANALYZE__LONG_PRESS_REPEATE:
	MOV ACCUM,LOOP_CNTR
	PUSH TEMP_H
	LDI TEMP,DRV_BUTTONS__HANDLER_OFFSET
	MCALL CORE5277_VAR_GET16
	POP TEMP
	PUSH_Z
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	ICALL
	POP_Z
	CLR TEMP
	STD Z+DRV_BUTTONS__TIME,TEMP
	;Сбраываем, оставляя бит состояния порта включенным
	LDI TEMP,(1<<DRV_BUTTONS__FL_STATE)
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP
	;Не сбрасываем результат, для следующего события длинного нажатия(если кнопку так и не отпустили)
	RJMP _DRV_BUTTONS_ANALYZE__END

_DRV_BUTTONS_ANALYZE__STATE_CHANGED:
	;Считываю и сбрасываю счетчик нахождения в одном состоянии
	LDD TEMP_L,Z+DRV_BUTTONS__TIME
	CLR TEMP
	STD Z+DRV_BUTTONS__TIME,TEMP
	;Если кнопку нажали, то просто выходим
	BRTS _DRV_BUTTONS_ANALYZE__END

	;Получаем результат
	LDD TEMP_H,Z+DRV_BUTTONS__RESULT
	;Получаем количество нажатых кнопок
	MOV TEMP,TEMP_H
	SWAP TEMP
	ANDI TEMP,0x0f

	;Проверяем на максимальное занчение
	CPI TEMP,0x04
	;Ничего не делаем, если достигнут максимум
	BREQ _DRV_BUTTONS_ANALYZE__END
	;Увеличиваем счетчик нажатий
	INC TEMP
	;Добавяем навое нажатие в результат
	PUSH TEMP
	LDI TEMP,DRV_BUTTONS__SHORT_PRESS_MAX_TIME
	CP TEMP,TEMP_L
	POP TEMP
	ROL TEMP_H
	ANDI TEMP_H,0b00001111
	SWAP TEMP
	OR TEMP_H,TEMP
	STD Z+DRV_BUTTONS__RESULT,TEMP_H
	RJMP _DRV_BUTTONS_ANALYZE__END

_DRV_BUTTONS_ANALYZE__END:
	POP ACCUM
	POP TEMP
	POP LOOP_CNTR
	POP_Z
	RET


;--------------------------------------------------------
DRV_BUTTONS_SET_BUTTON:
;--------------------------------------------------------
;Установка параметров кнопки
;IN: PID, ACCUM - ID(1-20),
;TEMP_H - порт, TEMP_L - пин
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_Z
	MCALL CORE5277_RAM_OFFSET
	DEC ACCUM
	MOV TEMP,ACCUM
	LSL ACCUM															;DRV_BUTTONS__STRUCT_SIZE
	LSL ACCUM
	ADD ACCUM,TEMP
	ADD ZL,ACCUM
	CLR TEMP
	ADC ZH,TEMP
	STD Z+DRV_BUTTONS__PORT,TEMP_H
	STD Z+DRV_BUTTONS__PIN,TEMP_L
	LDI TEMP,0x00|(1<<DRV_BUTTONS__FL_INIT)
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP
	POP_Z
	POP ACCUM
	RET


;--------------------------------------------------------
DRV_BUTTONS_SET_MODE:
;--------------------------------------------------------
;Установка режима работы кнопок
;IN: PID, ACCUM - режим
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_Z
	PUSH TEMP

	;Сброс состояний и счетчиков
	;Получаем количество кнопок
	LDI TEMP,DRV_BUTTONS__VAR_BUTTONS_QNT
	MCALL CORE5277_VAR_GET
	MOV LOOP_CNTR,ACCUM
	MCALL CORE5277_DRV_GET_DATAOFFSET
DRV_BUTTONS_SET_MODE__LOOP:
	LDI TEMP,0x00|(1<<DRV_BUTTONS__FL_INIT)
	STD Z+DRV_BUTTONS__CNTR_N_FLAGS,TEMP
	LDI TEMP,DRV_BUTTONS__STRUCT_SIZE
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	DEC LOOP_CNTR
	BRNE DRV_BUTTONS_SET_MODE__LOOP

	;Запись режима
	POP ACCUM
	LDI TEMP,DRV_BUTTONS__VAR_MODE
	MCALL CORE5277_VAR_SET

	POP_Z
	POP LOOP_CNTR
	POP ACCUM
	RET


