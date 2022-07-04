;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.03.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;28.10.2020	w5277c@gmail.com			Корректировка таймингов
;23.04.2021	w5277c@gmail.com			Багфиксинг и успешный первый тест
;04.07.2021	w5277c@gmail.com			Z->Y, оптимизация
;25.10.2021	w5277c@gmail.com			Добавлена поддержка кнопки с фиксацией(не тестировано)
;04.06.2022	konstantin@5277.ru		Добавлена поддержка сенсора и светодиода(не тестировано)
;07.06.2022	konstantin@5277.ru		Исправлена ошибка короткого нажатия после длительного удерживания(повтора) и 5-го
;												длинного нажатия, реализован новый функционал.
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер кнопок, понимает кнопку без фиксации, с фиксацией и сенсор.
;Поддерживает несколько разных кнопок одновременно.
;Может параллельно кнопки управлять светодиодом с резистором на 100КОм(подобран экпериментально, для яркости лучше
;выбрать синий)
;Запоминает 4 пордряд идущих нажатия, учитывает их длительность и долгое удреживание(флаг повтора)
;Поддерживает очередь, может вернуть последнее событие или ожидать событие.

;Режим сенсора вероятно будет работать только на высоких частотах МК, желательно не менее 16MHz
;С флагами и режимами нужно быть аккуратнее, к примеру режим LED не будет работать если у вас внешняя подтяжка.
;-----------------------------------------------------------------------------------------------------------------------
.message "TODO В режиме сенсора и включенном LED(с резистором в 100K) почти на порядок падает чуствительность сенсора)"
.message "TODO Реализовать STATELESS режим"
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_BUTTONS
.else
.set DEF__C5_DRV_BUTTONS = 1
.message "Included driver Buttons v0.5"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/ram/ram_extend.inc"
.include	"./mem/ram_fill.inc"
.include	"./core/wait.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/timer_stop.inc"
.include	"./core/timer_set_y.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_get.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BUTTONS_STRUCT_SIZE					= 0x05	;Размер структуры данных для каждой кнопки
	.EQU	_DRV_BUTTONS_QUEUE_MAX_SIZE				= 0x05	;Количество кнопок очереди для событий
	;---
	.EQU	_DRV_BUTTONS_TIMER_ID						= 0x00	;1B - ИД таймера
	.EQU	_DRV_BUTTONS_BUTTONS_QNT					= 0x01	;1B - Количество кнопок
	.EQU	_DRV_BUTTONS_CNTR								= 0x02	;1B - Счетчик опроса
	.EQU	_DRV_BUTTONS_QUEUE_SIZE						= 0x03	;1B - Кол-во элементов в очереди
	.EQU	_DRV_BUTTONS_QUEUE							= 0x04	;_DRV_BUTTONS_QUEUE_MAX_SIZE*2 - очередь событий [PORT,RESULT]
	.EQU	_DRV_BUTTONS_DATA								= _DRV_BUTTONS_QUEUE+(_DRV_BUTTONS_QUEUE_MAX_SIZE*2);x4B - Данные кнопок

	;---Структура данных по кнопкам---
	.EQU	_DRV_BUTTON_PORT								= 0x00	;Порт кнопки
	.EQU	_DRV_BUTTON_FLAGS								= 0x01	;Флаги
	.EQU	_DRV_BUTTON_CNTR_N_STATES					= 0x02	;Счетчик высоких состояний и состояние
	.EQU	_DRV_BUTTON_TIME								= 0x03	;Время отсчета
	.EQU	_DRV_BUTTON_RESULT							= 0x04	;Результат |rqqqssss|, r - повтор, q-кол-во нажатий(1-4), s-состояния(история 0-короткое/1-длинное)
	;---Тайминги---
	.EQU	_DRV_BUTTONS_TIMER_PERIOD					= 0x02	;Период таймера (x*0.002) (анализ 32мс)
	.EQU	_DRV_BUTTONS_MAX_CNTR						= 0x04	;Количество необходимых итераций перед анализом
	;---
	.EQU	_DRV_BUTTONS_SHORT_PRESS_MAX_TIME		= 200/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR);было 300
	.EQU	_DRV_BUTTONS_HOLD_TIME						= 600/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR);было 1000
	.EQU	_DRV_BUTTONS_TIMEOUT							= 400/(2*_DRV_BUTTONS_TIMER_PERIOD*_DRV_BUTTONS_MAX_CNTR);было 800
	;---Флаги---
	.EQU	_DRV_BUTTON_FL_LED_MODE						= 0x00	;Режим паралельно подключенного светодиода
	.EQU	_DRV_BUTTON_FL_LED_ON						= 0x01	;Светодиод включен
	.EQU	_DRV_BUTTON_FL_SENSOR						= 0x02	;Режим сенсора или кнопки с/без фиксации
	.EQU	_DRV_BUTTON_FL_FIX							= 0x03	;Кнопка с фиксацией-1, без фиксации-0
	.EQU	_DRV_BUTTON_FL_VCCBASE						= 0x04	;Общий плюс
	.EQU	_DRV_BUTTON_FL_STATELESS					= 0x05	;Мгновенная реакция в одно нажатие
	;---Состояния---
	.EQU	_DRV_BUTTON_ST_INIT							= 0x04	;Ожидание начала цикла опроса
	.EQU	_DRV_BUTTON_ST_FIRST_SCAN					= 0x05	;Первый опрос(нет предыдущего занчения)
	.EQU	_DRV_BUTTON_ST_RESERVED						= 0x06	;Зарезервировано на будущее
	.EQU	_DRV_BUTTON_ST_LAST_STATE					= 0x07	;Последнее состояние порта
	;---OPERATIONS---
	.EQU	DRV_BUTTONS_OP_ADD							= 0x00	;Добавляем кнопку без фиксации
	.EQU	DRV_BUTTONS_OP_ADD_FIX						= 0x01	;Добавляем кнопку с фиксацией
	.EQU	DRV_BUTTONS_OP_ADD_SENSOR					= 0x02	;Добавляем сенсор
	.EQU	DRV_BUTTONS_OP_ADD_CUSTOM					= 0x03	;Добавляем кнопку с заданными флагами
	.EQU	DRV_BUTTONS_OP_GET							= 0x04	;Считываем нажатие
	.EQU	DRV_BUTTONS_OP_WAIT							= 0x05	;Ожидаем нажатие
	.EQU	DRV_BUTTONS_OP_CLEAR							= 0x06	;Очищаем очередь нажатий
	.EQU	DRV_BUTTONS_OP_LED_ON						= 0x07	;Включаем светодид кнопки
	.EQU	DRV_BUTTONS_OP_LED_OFF						= 0x08	;Вылючаем светодид кнопки

	.EQU	_DRV_BUTTONS_RAM_SIZE						= 0x04+(_DRV_BUTTONS_QUEUE_MAX_SIZE*2)
;--------------------------------------------------------
DRV_BUTTONS_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_BUTTONS_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	STD Y+_DRV_BUTTONS_QUEUE_SIZE,C0x00
	STD Y+_DRV_BUTTONS_BUTTONS_QNT,C0x00
	STD Y+_DRV_BUTTONS_CNTR,C0x00

	LDI LOOP_CNTR,_DRV_BUTTONS_QUEUE_MAX_SIZE*2
	MOVW XL,YL
	ADIW XL,_DRV_BUTTONS_QUEUE
	LDI TEMP,0x00
	MCALL RAM_FILL

	;Инициализирую таймер с периодом в 0.002*_DRV_BUTTONS_TIMER_PERIOD
	MOV TEMP,ACCUM
	STD Y+_DRV_BUTTONS_TIMER_ID,TEMP
	LDI TEMP_H,high(DRV_BUTTONS_TIMER_EVENT)
	LDI TEMP_L,low(DRV_BUTTONS_TIMER_EVENT)
	LDI ACCUM,_DRV_BUTTONS_TIMER_PERIOD|0x80
	MCALL C5_TIMER_SET
	;Запускаю таймер
	MCALL C5_TIMER_START

	MCALL C5_READY
;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_BUTTONS_OP_ADD
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUTTONS_OP_ADD
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_ADD_FIX
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUTTONS_OP_ADD
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_ADD_SENSOR
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_BUTTONS_OP_ADD
	CPI FLAGS,DRV_BUTTONS_OP_ADD_CUSTOM
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_BUTTONS_OP_ADD

	CPI FLAGS,DRV_BUTTONS_OP_GET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUTTONS_OP_GET
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_WAIT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUTTONS_OP_WAIT
	RJMP _DRV_BUTTONS_PROC_END
	CPI FLAGS,DRV_BUTTONS_OP_CLEAR
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_BUTTONS_OP_CLEAR
	CPI FLAGS,DRV_BUTTONS_OP_LED_ON
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_BUTTONS_OP_LED
	CPI FLAGS,DRV_BUTTONS_OP_LED_OFF
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_BUTTONS_OP_LED

_DRV_BUTTONS_PROC_END:
	POP TEMP
	POP_Y
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_ADD:
;--------------------------------------------------------
;Добавляем кнопку
;IN: ACCUM-порт кнопки(7-бит-включать подтяжку)
;FLAGS-тип кнопки(DRV_BUTTONS_OP_ADD/FIX/SENSOR/CUST)
;TEMP_H-флаги(для режима ADD_CUSTOM),
;TEMP_L-порт нагрузки(1МОм) для сенсора или порт
;конденсатора для остальных режимов
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_X
	PUSH_Z

	;Увеличиваем размер выделенной памяти на _DRV_BUTTONS_STRUCT_SIZE
	PUSH ACCUM
	LDI ACCUM,_DRV_BUTTONS_STRUCT_SIZE
	MCALL C5_RAM_EXTEND
	LDD TEMP,Y+_DRV_BUTTONS_TIMER_ID
	MCALL C5_TIMER_SET_Y
	MOVW ZL,XL
	POP ACCUM

	LDD TEMP,Y+_DRV_BUTTONS_BUTTONS_QNT
	INC TEMP
	STD Y+_DRV_BUTTONS_BUTTONS_QNT,TEMP

	;Сбрасываем флаг подтяжки
	MOV TEMP,ACCUM
	ANDI ACCUM,0x7f
	STD Z+_DRV_BUTTON_PORT,ACCUM
	;Порт на вход
	MCALL PORT_MODE_IN
	SBRS TEMP,0x07
	RJMP PC+0x01+_MCALL_SIZE
	;Включаем подтяжку, если необходимо
	MCALL PORT_SET_HI

	MOV ACCUM,TEMP_L
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Записываем данные кнопки
	ANDI TEMP_H,0x0f
	LDI TEMP,0x00
	CPI FLAGS,DRV_BUTTONS_OP_ADD_CUSTOM
	BRNE PC+0x02
	OR TEMP,TEMP_H
	CPI FLAGS,DRV_BUTTONS_OP_ADD_FIX
	BRNE PC+0x02
	ORI TEMP,(1<<_DRV_BUTTON_FL_FIX)
	CPI FLAGS,DRV_BUTTONS_OP_ADD_SENSOR
	BRNE PC+0x02
	ORI TEMP,(1<<_DRV_BUTTON_FL_SENSOR)

	STD Z+_DRV_BUTTON_FLAGS,TEMP
	LDI TEMP,(1<<_DRV_BUTTON_ST_INIT)
	STD Z+_DRV_BUTTON_CNTR_N_STATES,TEMP
	STD Z+_DRV_BUTTON_TIME,C0x00
	STD Z+_DRV_BUTTON_RESULT,C0x00

	POP_Z
	POP_X
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_WAIT:
;--------------------------------------------------------
;Ожидаем нажатие
;OUT: TEMP_H-порт, TEMP_L-результат
;--------------------------------------------------------
_DRV_BUTTONS_OP_WAIT__LOOP:
	LDD TEMP,Y+_DRV_BUTTONS_QUEUE_SIZE
	CPI TEMP,0x00
	BRNE _DRV_BUTTONS_OP_GET
	MCALL C5_WAIT
	RJMP _DRV_BUTTONS_OP_WAIT__LOOP

;--------------------------------------------------------
_DRV_BUTTONS_OP_GET:
;--------------------------------------------------------
;Считываем нажатие
;OUT: TEMP_H-порт(0xff-нет нажатий),
;TEMP_L-результат
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	LDI TEMP_H,0xff
	;Декрементируем размер очереди
	LDS ACCUM,SREG
	CLI
	LDD LOOP_CNTR,Y+_DRV_BUTTONS_QUEUE_SIZE
	CPI LOOP_CNTR,0x00
	BREQ _DRV_BUTTONS_PULL_QUEUE__END
	DEC LOOP_CNTR
	STD Y+_DRV_BUTTONS_QUEUE_SIZE,LOOP_CNTR

	LDI XH,0x00
	LDI XL,_DRV_BUTTONS_QUEUE
	LSL LOOP_CNTR
	ADD XL,LOOP_CNTR
	ADD XL,YL
	ADC XH,YH
	LD TEMP_H,X+
	LD TEMP_L,X
	;Если очередь не пуста, то сдвигаем данные очереди
	CPI LOOP_CNTR,0x00
	BREQ _DRV_BUTTONS_PULL_QUEUE__END
	LDI XH,0x00
	LDI XL,_DRV_BUTTONS_QUEUE+0x02
	ADD XL,YL
	ADC XH,YH
	MOVW ZL,YL
	ADIW ZL,_DRV_BUTTONS_QUEUE
	MCALL RAM_COPY
_DRV_BUTTONS_PULL_QUEUE__END:
	STS SREG,ACCUM

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_LED:
;--------------------------------------------------------
;Управляем светодиодом
;IN: ACCUM-порт кнопки
;FLAGS-тип команды(DRV_BUTTONS_OP_LED_ON/OFF)
;--------------------------------------------------------
	PUSH LOOP_CNTR

	LDD LOOP_CNTR,Y+_DRV_BUTTONS_BUTTONS_QNT
	CPI LOOP_CNTR,0x00
	BRNE PC+0x02
	RJMP _DRV_BUTTONS_OP_LED__END

	ADIW YL,_DRV_BUTTONS_DATA
	;Ищем нужный порт кнопки и задаем флаг _DRV_BUTTONS_FL_LED_ON
_DRV_BUTTONS_OP_LED__LOOP:
	LDD TEMP,Y+_DRV_BUTTON_PORT
	CP TEMP,ACCUM
	BRNE _DRV_BUTTONS_OP_LED__NEXT

	LDD TEMP,Y+_DRV_BUTTON_FLAGS
	CPI FLAGS,DRV_BUTTONS_OP_LED_ON
	BRNE PC+0x02
	ORI TEMP,(1<<_DRV_BUTTON_FL_LED_ON)
	CPI FLAGS,DRV_BUTTONS_OP_LED_OFF
	BRNE PC+0x02
	ANDI TEMP,~(1<<_DRV_BUTTON_FL_LED_ON)
	STD Y+_DRV_BUTTON_FLAGS,TEMP
	RJMP _DRV_BUTTONS_OP_LED__END
_DRV_BUTTONS_OP_LED__NEXT:
	ADIW YL,_DRV_BUTTONS_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE _DRV_BUTTONS_OP_LED__LOOP
_DRV_BUTTONS_OP_LED__END:

	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_BUTTONS_OP_CLEAR:
;--------------------------------------------------------
;Очищаем очередь нажатий
;--------------------------------------------------------
	STD Y+_DRV_BUTTONS_QUEUE_SIZE,C0x00
	RET

;--------------------------------------------------------
DRV_BUTTONS_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру(анализ портов)
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH FLAGS

	LDD LOOP_CNTR,Y+_DRV_BUTTONS_BUTTONS_QNT
	CPI LOOP_CNTR,0x00
	BRNE PC+0x02
	RJMP DRV_BUTTONS_TIMER_EVENT__END

	;Инкрементирую счетчик опроса
	LDD TEMP,Y+_DRV_BUTTONS_CNTR
	CPI TEMP,_DRV_BUTTONS_MAX_CNTR
	BRNE PC+0x02
	RJMP DRV_BUTTONS_TIMER_EVENT__ANALYZE
	INC TEMP
	STD Y+_DRV_BUTTONS_CNTR,TEMP

	;Смещаемся на данные первой кнопки
	ADIW YL,_DRV_BUTTONS_DATA

	;Читаем состояние портов для каждой кнопки
DRV_BUTTONS_TIMER_EVENT__READ_LOOP:
	LDD ACCUM,Y+_DRV_BUTTON_PORT
	LDD TEMP_H,Y+_DRV_BUTTON_FLAGS
.MESSAGE "TODO Реализация сенсора. Хм, светодиод может выступать как нагрузочный резистор?"
	MCALL PORT_SET_HI

	SBRS TEMP_H,_DRV_BUTTON_FL_SENSOR
	RJMP DRV_BUTTONS_TIMER_EVENT__GET_BUTTON_STATE


	PUSH LOOP_CNTR
	PUSH TEMP_H
	LDI LOOP_CNTR,0x10
	MCALL PORT_OFFSETS
	MOV ZH,C0x00
	MOV ZL,TEMP_H
	MOV TEMP_H,ACCUM
	COM TEMP_H
	LDS FLAGS,SREG
	CLI
	;Выключаем подтяжку
	LD TEMP_L,Z
	AND TEMP_L,TEMP_H
	ST Z,TEMP_L
	MOV ZL,TEMP
DRV_BUTTONS_TIMER_EVENT__SENSOR_LOOP:
	LD TEMP,Z
	AND TEMP,ACCUM
	BREQ DRV_BUTTONS_TIMER_EVENT__SENSOR_DONE
	DEC LOOP_CNTR
	BRNE DRV_BUTTONS_TIMER_EVENT__SENSOR_LOOP
DRV_BUTTONS_TIMER_EVENT__SENSOR_DONE:
	STS SREG,FLAGS
	SUBI LOOP_CNTR,0x01
	POP TEMP_H
	POP LOOP_CNTR
	RJMP DRV_BUTTONS_TIMER_EVENT__GET_BUTTON_STATE+_MCALL_SIZE

DRV_BUTTONS_TIMER_EVENT__GET_BUTTON_STATE:
	MCALL PORT_GET
	LDI TEMP_L,0x00
	ROL TEMP_L
	SBRC TEMP_H,_DRV_BUTTON_FL_VCCBASE
	SUBI TEMP_L,0x01
	BRNE DRV_BUTTONS_TIMER_EVENT__LO_STATE
	;Инкрементирую счетчик
	LDD TEMP,Y+_DRV_BUTTON_CNTR_N_STATES
	MOV ACCUM,TEMP
	ANDI TEMP,0xf8
	ANDI ACCUM,0x07
	INC ACCUM
	OR TEMP,ACCUM
	STD Y+_DRV_BUTTON_CNTR_N_STATES,TEMP
DRV_BUTTONS_TIMER_EVENT__LO_STATE:
	;Выключаем подтяжку если режим светодиода и он выключен
	SBRS TEMP_H,_DRV_BUTTON_FL_LED_MODE
	RJMP PC+0x05+_MCALL_SIZE
	LDD ACCUM,Y+_DRV_BUTTON_PORT
	CLC
	SBRC TEMP_H,_DRV_BUTTON_FL_LED_ON
	SEC
	MCALL PORT_SET

	ADIW YL,_DRV_BUTTONS_STRUCT_SIZE
	DEC LOOP_CNTR
	BRNE DRV_BUTTONS_TIMER_EVENT__READ_LOOP
	RJMP DRV_BUTTONS_TIMER_EVENT__END

DRV_BUTTONS_TIMER_EVENT__ANALYZE:
	;Останавливаю таймер, пока не выполню анализ
	LDD TEMP,Y+_DRV_BUTTONS_TIMER_ID
	MCALL C5_TIMER_STOP
	;Разрешаю прерывания, так как процедура медленная
	LDS TEMP,SREG
	PUSH TEMP
	SEI

	PUSH_X

	;Сбрасываю счетчик, т.к. отсчет завершен
	LDI TEMP,0x00
	STD Y+_DRV_BUTTONS_CNTR,TEMP

	MOVW ZL,YL
	;Смещаемся на данные первой кнопки
	ADIW ZL,_DRV_BUTTONS_DATA



DRV_BUTTONS_TIMER_EVENT__ANALYZE_LOOP:
	;Считываю флаги и счетчик с состояниями
	LDD FLAGS,Z+_DRV_BUTTON_FLAGS
	LDD TEMP_H,Z+_DRV_BUTTON_CNTR_N_STATES
	;Считываем предыдущее состояние порта в ACCUM
	CLR ACCUM
	SBRC TEMP_H,_DRV_BUTTON_ST_LAST_STATE
	ORI ACCUM,(1<<_DRV_BUTTON_ST_LAST_STATE)
	MOV TEMP_L,TEMP_H
	;Сбрасываем счетчик и состояние в 0
	ANDI TEMP_H,low(~((1<<_DRV_BUTTON_ST_LAST_STATE)|0x07))
	ANDI TEMP_L,0x07
	LDI TEMP,_DRV_BUTTONS_MAX_CNTR/2
	CP TEMP,TEMP_L
	BRCC PC+0x02
	;Устанавливаем состояние в 1
	ORI TEMP_H,(1<<_DRV_BUTTON_ST_LAST_STATE)
	STD Z+_DRV_BUTTON_CNTR_N_STATES,TEMP_H

;TODO Нужно учитывать, что кнопка может быть нажата до запуса МК, не обрабатываем пока не отпустят

	;Проверяем на только что добавленную кнопку
	SBRS TEMP_H,_DRV_BUTTON_ST_INIT
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_INIT
	STD Z+_DRV_BUTTON_TIME,C0x00
	STD Z+_DRV_BUTTON_RESULT,C0x00
	LDI TEMP_H,(1<<_DRV_BUTTON_ST_FIRST_SCAN)
	STD Z+_DRV_BUTTON_CNTR_N_STATES,TEMP_H
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_INIT:
	;Проверяем на первое сканирование
	SBRS TEMP_H,_DRV_BUTTON_ST_FIRST_SCAN
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_FIRST_SCAN
	;В режиме FIX и STATELESS не нужно ждать когда кнопку разомкнут
	SBRC FLAGS,_DRV_BUTTON_FL_FIX
	RJMP PC+0x03
	SBRS FLAGS,_DRV_BUTTON_FL_STATELESS
	RJMP PC+0x04
	ANDI TEMP_H,(1<<_DRV_BUTTON_ST_LAST_STATE)
	STD Z+_DRV_BUTTON_CNTR_N_STATES,TEMP_H
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_FIRST_SCAN
	;Если кнопка нажата, то остаемся в режиме первого сканирования
	SBRC TEMP_H,_DRV_BUTTON_ST_LAST_STATE
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Иначе, конпка не нажата, сбрасываем флаг первого сканирования
	STD Z+_DRV_BUTTON_CNTR_N_STATES,C0x00
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NOT_FIRST_SCAN:
	;_DRV_BUTTON_ST_STATE бит ACCUM хранит старое значение, в TEMP_H хранит новое(true-нажата)
	EOR ACCUM,TEMP_H
	;FIX или STATELESS? - просто добавляем результат в очередь
	SBRC FLAGS,_DRV_BUTTON_FL_FIX
	RJMP PC+0x03
	SBRS FLAGS,_DRV_BUTTON_FL_STATELESS
.MESSAGE "TODO Похоже для STATELESS нужна доработка, добавление должно быть только при нажатии"
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_FOR_WO_FIX
	SBRS ACCUM,_DRV_BUTTON_ST_LAST_STATE
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Добавяем результат
	LDI FLAGS,0x10
	MCALL _DRV_BUTTONS__QUEUE_ADD
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_FOR_WO_FIX:
	;Далее флаги не используются, FLAGS можно использовать для RESULT
	LDD FLAGS,Z+_DRV_BUTTON_RESULT
	;Переходим если состояние кнопки изменилось
	SBRC ACCUM,_DRV_BUTTON_ST_LAST_STATE
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_STATE_CHANGED

	;Увеличиваем время нахождения в одном и том-же состоянии
	LDD TEMP_L,Z+_DRV_BUTTON_TIME
	CPI TEMP_L,0xff
	BREQ PC+0x03
	INC TEMP_L
	STD Z+_DRV_BUTTON_TIME,TEMP_L

	;Проверка на таймауты
	;Переходим, если кнопка, как и ранее, нажата
	SBRC TEMP_H,_DRV_BUTTON_ST_LAST_STATE
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_BUTTON_PRESSED
	;Кнопка, как и ранее, не нажата, проверяем таймаут если вообще были нажатия
	MOV TEMP,FLAGS
	ANDI TEMP,0b01110000
	;Если нажатий не было, проверяем следующую кнопку
	BRNE PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	CPI TEMP_L,_DRV_BUTTONS_TIMEOUT
	;Таймаут не достигнут
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Тамаут достигнут, действия пользователя окончены, помещаем результат в очередь
	MCALL _DRV_BUTTONS__QUEUE_ADD
	STD Z+_DRV_BUTTON_RESULT,C0x00
	STD Z+_DRV_BUTTON_CNTR_N_STATES,C0x00
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_BUTTON_PRESSED:
	;Кнопка, как и ранее, нажата
	;Проверяем на длинное нажатие, иначе переходим на следующую кнопку
	CPI TEMP_L,_DRV_BUTTONS_HOLD_TIME
	BRCC PC+0x02
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Пропускаем блок, если r в RESULT уже установлен
	SBRC FLAGS,0x07
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_LONG_PRESS_REPEATE
	;Заполняем переменные и вызываем обработчик
	MOV TEMP,FLAGS
	;Инкрементируем количество нажатий
	SWAP TEMP
	ANDI TEMP,0b00000111
	INC TEMP
	SWAP TEMP
	;Добавляем длинное нажатие
	SEC
	ROL FLAGS
	ANDI FLAGS,0b00001111
	OR FLAGS,TEMP
	;Включаем r в RESULT(повтор)
	ORI FLAGS,0b10000000
	STD Z+_DRV_BUTTON_RESULT,FLAGS

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_LONG_PRESS_REPEATE:
	MCALL _DRV_BUTTONS__QUEUE_ADD
	;Сбраываем, оставляя бит состояния порта включенным
	LDI TEMP,(1<<_DRV_BUTTON_ST_LAST_STATE)
	STD Z+_DRV_BUTTON_CNTR_N_STATES,TEMP
	;Не сбрасываем результат, для следующего события длинного нажатия(если кнопку так и не отпустили)
	;Но сбрасываем счетчик, чтобы не уперся в 0xff при удерживании
	STD Z+_DRV_BUTTON_TIME,C0x00
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_STATE_CHANGED:
	;r в RESULT включен? Значит было длительное нажатие(сбрасываем состояние),
	;пользователь просто отпустил кнопку(т.е. не последующее короткое нажатие)
	SBRS FLAGS,0x07
	RJMP PC+0x03
	STD Z+_DRV_BUTTON_CNTR_N_STATES,C0x00
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

	;Считываю и сбрасываю счетчик нахождения в одном состоянии
	LDD TEMP_L,Z+_DRV_BUTTON_TIME
	STD Z+_DRV_BUTTON_TIME,C0x00
	;Если кнопку нажали, то просто выходим
	SBRC TEMP_H,_DRV_BUTTON_ST_LAST_STATE
	RJMP _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT

	;Проверяем количество нажатых кнопок
	MOV TEMP,FLAGS
	SWAP TEMP
	ANDI TEMP,0x07
	;Проверяем на максимальное занчение
	CPI TEMP,0x04
	;Ничего не делаем, если достигнут максимум
	BREQ _DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT
	;Увеличиваем счетчик нажатий
	INC TEMP
	;Добавяем новое нажатие в результат
	LDI ACCUM,_DRV_BUTTONS_SHORT_PRESS_MAX_TIME
	CP ACCUM,TEMP_L
	ROL FLAGS
	ANDI FLAGS,0b00001111
	SWAP TEMP
	OR FLAGS,TEMP
	STD Z+_DRV_BUTTON_RESULT,FLAGS

_DRV_BUTTONS_TIMER_EVENT__ANALYZE_NEXT:
	ADIW ZL,_DRV_BUTTONS_STRUCT_SIZE
	DEC LOOP_CNTR
	BREQ PC+0x02
	RJMP DRV_BUTTONS_TIMER_EVENT__ANALYZE_LOOP
DRV_BUTTONS_TIMER_EVENT__ANALYZE_END:
	;Заново запускаю таймер
	LDD TEMP,Y+_DRV_BUTTONS_TIMER_ID
	MCALL C5_TIMER_START

	POP_X
	POP TEMP
	STS SREG,TEMP
DRV_BUTTONS_TIMER_EVENT__END:
	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUTTONS__QUEUE_ADD:
;--------------------------------------------------------
;Добавляем элемент в очередь
;IN: Y-Адрес основных переменных, Z-адрес данных
;кнопки, FLAGS-результат обработки кнопки
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH_X

	;Достигнут таймаут, добавляем событие в очередь
	LDD TEMP,Y+_DRV_BUTTONS_QUEUE_SIZE
	CPI TEMP,_DRV_BUTTONS_QUEUE_MAX_SIZE
	;Пропускаем добавление, если очередь заполнена
	BREQ _DRV_BUTTONS__QUEUE_ADD_END

	;Вычисляем адрес очереди
	MOVW XL,YL
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
	LDD TEMP,Z+_DRV_BUTTON_PORT
	ST X+,TEMP															;Порт кнопки
	ST X,FLAGS															;Результат
	STS SREG,TEMP_L
	;Очищаю данные кнопки
	CLR TEMP
	STD Z+_DRV_BUTTON_TIME,TEMP
_DRV_BUTTONS__QUEUE_ADD_END:
	POP_X
	POP TEMP_L
	POP TEMP
	RET
.endif
