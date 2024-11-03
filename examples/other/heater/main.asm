;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:konstantin@5277.ru,
;по всем правовым вопросам обращайтесь на email.
;Взят, в качестве примера, из реального проекта.
;-----------------------------------------------------------------------------------------------------------------------
;Нагреватель оснащенный DS18B20, с коммуникацией по I2C(режим ведомого)
;-----------------------------------------------------------------------------------------------------------------------
;17.10.2024	konstantin@5277.ru		Начало(табуляция 3 символа), первые успешные тесты
;-----------------------------------------------------------------------------------------------------------------------

	.EQU	FW_VERSION										= 1		;Версия 0.1
	.EQU	CORE_FREQ										= 8		;Частота процессора
	.EQU	TIMER_C_ENABLE									= 0		;Использование аппаратного таймера(быстрая реализация)
	.SET	AVRA												= 0		;Использование сборщика AVRA(больше отладочной информации)
	.SET	REPORT_INCLUDES								= 0		;Информация при сборке о подключенных библиотеках

	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/attiny85.inc"							;Используемый микроконтроллер

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	I2C_ADDR											= 0x10	;Адрес устройства на шине I2C(TWI)

	;---PORT-CONSTANTS---
	.EQU	TEMP_PORT										= PB1		;Порт датчика температуры DS18B20
	.EQU	HEATER_PORT										= PB3		;Порт нагревательного элемента(высокий уровень=включен)

	;Идентификаторы драйверов(0-...)
	.EQU	PID_1WIRE										= 0|(1<<C5_PROCID_OPT_DRV)	;ИД драйвера шины 1wire
	.EQU	PID_DS18B20										= 1|(1<<C5_PROCID_OPT_DRV)	;ИД драйвера датчика температуры DS18B20
	.EQU	PID_I2C											= 2|(1<<C5_PROCID_OPT_DRV)	;ИД драйвера I2C(TWI) slave на базе USI

	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0		;ИД задачи
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	LOGGING_PORT									= PB4		;Порт логирования. Код логирования подгружается если порт задан
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC	;Уровень логирования, PNC-полный

	.SET	TS_MODE											= TS_MODE_NO	;Не использовать многозадачность, код диспетчера не подгружается
	.SET	C5_DRIVERS_QNT									= 3		;Количество используемых драйверов
	.SET	C5_TASKS_QNT									= 1		;Количество используемых задач
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us - частота основного таймера(норма=50)
	.SET	TIMERS											= 0		;Количество 8бит программных таймеров, 0-код не подгружается
	.SET	TIMERS16											= 0		;Количество 16бит программныйх таймеров, 0-код не подгружается

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"								;Подключаем ядро
	;---
	;Блок драйверов
	.INCLUDE	"./core/drivers/1wire_s.inc"						;Подключаем драйвер 1wire
	.INCLUDE	"./core/drivers/ds18b20.inc"						;Подключаем драйвер датчика температуры DS18B20
	.INCLUDE "./core/drivers/i2c_su.inc"						;Подключаем драйвер I2C(TWI) slave by USI

	.INCLUDE "./math/sdnf_cp.inc"									;Подключаем процедуру сравнения двух SDNF значений
	.INCLUDE "./io/port_set.inc"									;Подключаем процедуру установки порта относительно флага C
	.INCLUDE "./io/port_set_hi.inc"								;Подключаем процедуру установки порта в HI пложение
.ifdef LOGGING_PORT
	.INCLUDE "./core/io/out_sdnf.inc"							;Подключаем процедуру логирования SDNF значения
	.INCLUDE "./core/io/out_bytes.inc"							;Подключаем процедуру логирования блока байт
.endif
	.INCLUDE "./mem/eeprom_read_byte.inc"						;Подключаем процедуру чтения байта из EEPROM
	.INCLUDE "./mem/eeprom_write_byte.inc"						;Подключаем процедуру записи байта в EEPROM
	.INCLUDE "./mem/ram_fill.inc"									;Подключаем процедуру заполнения блока памяти(8бит)
	.INCLUDE "./mem/ram_copy.inc"									;Подключаем процедуру копирования блока памяти(8бит)
	.INCLUDE "./core/uptime_copy.inc"							;Подключаем процедуру записи UPTIME в память
	.INCLUDE "./conv/crc8_block.inc"								;Подключаем процедуру вычисления CRC8 для блока памяти(8бит)
	.INCLUDE "./conv/crc8_maxim.inc"								;Подключаем процедуру вычисления CRC8 maxim

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация светодиодов и других портов
	LDI ACCUM,HEATER_PORT											;Устанавливаем порт нагревателя в режим выхода с LO состоянием
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Инициализация ядра
	MCALL C5_INIT														;Инициализируем ядро

	LDI PID,PID_1WIRE													;Инициализируем драйвер 1wire
	LDI_Z DRV_1WIRE_INIT												;Указываем точку входа инициализации драйвера
	LDI ACCUM,TEMP_PORT												;Указываем порт 1wire
	MCALL C5_CREATE

	LDI PID,PID_DS18B20												;Инициализируем драйвер DS18B20
	LDI_Z DRV_DS18B20_INIT											;Указываем точку входа инициализации драйвера
	LDI ACCUM,PID_1WIRE												;Указваем драйвер 1wire
	MCALL C5_CREATE

	LDI PID,PID_I2C													;Инициализация драйвера I2C
	LDI_Z DRV_I2C_SU_INIT											;Указываем точку входа инициализации драйвера
	LDI TEMP_H,I2C_ADDR												;Указываем адрес устройства
	LDI TEMP_L,0x10													;Указываем размер буфера
	LDI XH,high(_TASK_I2C_HANDLER)								;Указываем точку входа в процедуру обработчика принятых данных
	LDI XL,low(_TASK_I2C_HANDLER)
	LDI FLAGS,PID_TASK												;Указываем ИД задачи процедуры обработчика принятых данных
	MCALL C5_CREATE

	LDI PID,PID_TASK													;Инициализация единственной задачи
	LDI_Z TASK_INIT													;Указываем точку входа инициализации задачи
	MCALL C5_CREATE

_MAIN__END:
	MJMP C5_START														;Запускаем ядро

	;---TASK-RAM-OFFSETS---											;Смещения данных в RAM(относительно начала выделенной памяти задачи)
	.EQU	TIMESTAMP										= 0x00	;5B-временная метка(uptime), обновляется при считывании значения температуры с DS18B20
	.EQU	CUR_TEMP											= 0x05	;2B-текущая температура(SDNF)
;	.EQU	TIMESTAMP										= 0x07	;5B-альтернативное значение(обеспечение атомарности)
;	.EQU	CUR_TEMP											= 0x0c	;2B-альтернативное значение(обеспечение атомарности)
	.EQU	I2C_THRESHOLD									= 0x0e	;2B-заданная температура(порог)(SDNF)
	.EQU	FLAG_DATA_BLOCK								= 0x10	;1B-Флаг для обеспечения атомарности блока данных(uptime+temp)
	.EQU	FLAG_THRESHOLD									= 0x11	;1B-Флаг для обеспечения атомарности значения заданной температуры(порога)

	;---TASK-EEPROM-OFFSETS---
	.EQU	EEPROM_PAGE										= EEPROM_PAGESIZE*0x00	;1B-номер актуальной страницы EEPROM(EEPROM_PAGESIZE всегда не менее 4Б)
	.EQU	EEPROM_P1_THRESHOLD							= EEPROM_PAGESIZE*0x01	;2B-заданная температура(порог)(SDNF)
	.EQU	EEPROM_P2_THRESHOLD							= EEPROM_PAGESIZE*0x02	;2B-альтернативное значение

TASK_INIT:																;Инициализация задачи
	LDI ACCUM,0x12														;Выделяем память(18B)
	MCALL C5_RAM_REALLOC

	MOVW XL,YL															;Заполняем всю выделенную память 0xff
	LDI TEMP,0xff
	LDI LOOP_CNTR,0x12
	MCALL RAM_FILL

	MCALL C5_READY														;Инициализация задачи завершена, дальнейший код выплняется после запуска ядра
;--------------------------------------------------------

	LDI_Z 0x1900														;В Z храним заданный уровень температуры(SDNF формат), изначально:25С

	LDI TEMP_H,0x00													;Считываем из EEPROM номер актуального блока
	LDI TEMP_L,EEPROM_PAGE
	MCALL EEPROM_READ_BYTE
	MOV FLAGS,TEMP
	CPI FLAGS,0x02														;Проверяем на корректность значения номера блока
	BRCS PC+0x03
	LDI FLAGS,0x00														;Далее храним в FLAGS номер активной страницы EEPROM
	RJMP _TASK__EEPROM_READ_SKIP									;>=2? Не корректно, игнорируем чтение EEPROM

	LDI TEMP_L,EEPROM_P1_THRESHOLD								;Устанавливаем адрес на актуальные данные
	SBRC FLAGS,0x00
	LDI TEMP_L,EEPROM_P2_THRESHOLD

	MCALL EEPROM_READ_BYTE											;Читаем из EEPROM заданный уровень температуры(целая часть)
	MOV ZH,TEMP
	INC TEMP_L
	MCALL EEPROM_READ_BYTE											;Читаем из EEPROM заданный уровень температуры(сотые)
	MOV ZL,TEMP

_TASK__EEPROM_READ_SKIP:
	STD Y+I2C_THRESHOLD+0x00,ZH									;Записываем порог(Z) в I2C блок, для исключения ложного срабатывания. I2C пока не запущен
	STD Y+I2C_THRESHOLD+0x01,ZL

	LDI TEMP,PID_I2C													;Запуск I2C, теперь возможнна отработка обработчика принятых данных от I2C, что может влиять на данные в памяти задачи
	MCALL C5_EXEC

	LDI XL,0xff															;В X храним текущую температуру, изначально null
_TASK__LOOP:															;Начало бесконечного цикла
	LDD TEMP_H,Y+FLAG_THRESHOLD 									;Считываем текущее значение флага записи порога
	LDD TEMP_EH,Y+I2C_THRESHOLD+0x00								;Cчитываем в TEMP_EH/EL значение I2C порога
	LDD TEMP_EL,Y+I2C_THRESHOLD+0x01
	LDD TEMP_L,Y+FLAG_THRESHOLD 									;Еще раз считываем текущее значение флага записи порога
	EOR TEMP_H,TEMP_L													;XOR, выявляем разницу в первом бите(т.е. факт изменения I2C порога во время считывания)
	SBRC TEMP_H,0x00
	RJMP _TASK_INIT__EEPROM_WRITE_SKIP							;Если нарушена атомарность, то игнорируенм данное чтение. Корректное получим на следующей итерации

	CP ZH,TEMP_EH														;Проверяем на измененме порога
	BRNE _TASK_INIT__EEPROM_WRITE
	CP ZL,TEMP_EL
	BREQ _TASK_INIT__EEPROM_WRITE_SKIP

_TASK_INIT__EEPROM_WRITE:											;Порог изменен, записываем его в EEPROM
	LDI TEMP_H,0x00
	LDI TEMP_L,EEPROM_P1_THRESHOLD								;Устанавливаем адрес на не актуальные данные
	COM FLAGS															;Для этого меняем страницу
	ANDI FLAGS,0x01
	BREQ PC+0x02
	LDI TEMP_L,EEPROM_P2_THRESHOLD

	MOV TEMP,TEMP_EH													;Записываем целую часть порога
	MCALL EEPROM_WRITE_BYTE
	INC TEMP_L
	MOV TEMP,TEMP_EL													;Записываем сотые порога
	MCALL EEPROM_WRITE_BYTE
	LDI TEMP_L,EEPROM_PAGE											;Актуализируем страницу
	MOV TEMP,FLAGS
	MCALL EEPROM_WRITE_BYTE

	MOV ZH,TEMP_EH														;Фиксируем новый порог в Z
	MOV ZL,TEMP_EL

_TASK_INIT__EEPROM_WRITE_SKIP:
	LDI ACCUM,HEATER_PORT
	CPI XL,0xFF
	BREQ PC+0x03+_MCALL_SIZE										;Флаг C сброшен, если null
	CPI ZL,0xFF
	BREQ PC+0x01+_MCALL_SIZE										;Флаг C сброшен, если null
	MCALL SDNF_CP														;Проверяем разницу температур
	MCALL PORT_SET														;Выключаем нагрев, если null или текущая температура выше порога, если ниже - включаем

	LDD TEMP,Y+FLAG_DATA_BLOCK										;Выбираем блок данных в зависимости от флага(атомарность)
	PUSH_Y
	SBRS TEMP,0x00
	ADIW YL,0x07

	LDI TEMP,PID_DS18B20												;Вызываю драйвер DS18B20 для получения значения температуры в SDNF формате
	MCALL C5_EXEC
	MCALL C5_UPTIME_COPY												;Записываем текущй UPTIME
.ifdef LOGGING_PORT													;Логируем
	PUSH_T16
	MCALL C5_OUT_BYTE													;Результат работы драйвера
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_SDNF													;Текущая температура
	LDI TEMP,'-'
	MCALL C5_OUT_CHAR
	LDI TEMP,'>'
	MCALL C5_OUT_CHAR
	MOV TEMP_H,ZH														;Порог
	MOV TEMP_L,ZL
	MCALL C5_OUT_SDNF
	MCALL C5_OUT_CR													;Новая строка
	POP_T16
.endif
	STD Y+CUR_TEMP+0x00,TEMP_H										;Фиксируем полученую температуру
	STD Y+CUR_TEMP+0x01,TEMP_L
	MOV XH,TEMP_H
	MOV XL,TEMP_L

	POP_Y
	LDD TEMP,Y+FLAG_DATA_BLOCK										;Меняем блок памяти на только что заполненный
	INC TEMP
	STD Y+FLAG_DATA_BLOCK,TEMP

_TASK__END:																;Конец цикла, выдерживаем паузу в 1с(плюс, где-то 1s была затрачена на опрос DS18B20)
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI TEMP,0xf4
	MCALL C5_WAIT_2MS													;1s
	RJMP _TASK__LOOP

;--------------------------------------------------------
;Параметры обработчика:
;IN: X-буфер, PID-ид задачи, LOOP-CNTR-размер данных в
;буфере
;OUT: LOOP_CNTR-размер данных ответа в буфере
;--------------------------------------------------------
_TASK_I2C_HANDLER:
	PUSH ACCUM
	PUSH_Y
	PUSH TEMP

	MCALL C5_RAM_OFFSET												;Получаем в Y начало блока выделенной памяти задачи

.ifdef LOGGING_PORT													;Логируем
.if LOGGING_LEVEL	== LOGGING_LVL_PNC
	PUSH TEMP
	MOV TEMP,LOOP_CNTR												;Размер запроса
	MCALL C5_OUT_BYTE
	LDI TEMP,'<'
	MCALL C5_OUT_CHAR
	MOVW ZL,XL
	MOV TEMP,LOOP_CNTR
	MCALL C5_OUT_BYTES												;Блок запроса
	MCALL C5_OUT_CR													;Новая строка
	POP TEMP
.endif
.endif

	CPI LOOP_CNTR,0x00												;Возвращаем ошибку, если данных нет
	BREQ _TASK_I2C_HANDLER__ERROR

	LD ACCUM,X															;Получаем код операции DRV_OP_...
	CPI ACCUM,DRV_OP_GET
	BRNE _TASK_I2C_HANDLER__NO_GET

	PUSH_Y
	LDD TEMP,Y+FLAG_DATA_BLOCK										;Выбираем блок данных в зависимости от флага(атомарность)
	SBRC TEMP,0x00
	ADIW YL,0x07

	LDI ACCUM,DRV_RESULT_OK											;Подготавливаем блок данных для ответа
	ST X,ACCUM
	PUSH_X
	MOVW ZL,XL
	ADIW ZL,0x01
	MOVW XL,YL
	LDI LOOP_CNTR,0x07
	MCALL RAM_COPY														;Копируем блок данных(UPTIME,TEMP) в буфер I2C
	POP_X
	ADIW XL,0x08
	POP_Y
	LDD TEMP,Y+I2C_THRESHOLD+0x00									;Копируем порог
	ST X+,TEMP
	LDD TEMP,Y+I2C_THRESHOLD+0x01
	ST X,TEMP
	SBIW XL,0x09
	LDI LOOP_CNTR,0x0a												;Сформировано 10 байт данных
	RJMP _TASK_I2C_HANDLER__END

_TASK_I2C_HANDLER__NO_GET:
	CPI ACCUM,DRV_OP_SET
	BRNE _TASK_I2C_HANDLER__NO_SET

	CPI LOOP_CNTR,0x04												;Возвращаем ошибку, если размер данных не равен 4
	BRNE _TASK_I2C_HANDLER__ERROR

	LDI ACCUM,0x00														;Получен новый порог
	LDI LOOP_CNTR,0x03
	LDI_Z CRC8_MAXIM													;Ожидаем 3 байта, считаем CRC8
	MCALL CRC8_BLOCK
	ADIW XL,0x01
	LD ZH,X+																;Получаем значение порога
	LD ZL,X+
	LD LOOP_CNTR,X														;Получаем значение CRC
	CP LOOP_CNTR,ACCUM												;Сравниваем вычисленный и полученный CRC
	BRNE _TASK_I2C_HANDLER__ERROR									;Отвечаем ошибкой, если не сошлись CRC
	STD Y+I2C_THRESHOLD+0x00,ZH									;Фиксируем новое значение порога
	STD Y+I2C_THRESHOLD+0x01,ZL
	LDD ACCUM,Y+FLAG_THRESHOLD										;Инвертируем бит-сообщаем о модифкации слова
	INC ACCUM
	STD Y+FLAG_THRESHOLD,ACCUM
	SBIW XL,0x03														;Восстанавливаем начальное значение X

	LDI ACCUM,DRV_RESULT_OK											;Успешное выполнение
	ST X,ACCUM
	LDI LOOP_CNTR,0x01												;Ответ размером в 1Б
	RJMP _TASK_I2C_HANDLER__END
_TASK_I2C_HANDLER__NO_SET:
_TASK_I2C_HANDLER__ERROR:
	LDI ACCUM,DRV_RESULT_ERROR										;Ошибка выполнения
	ST X,ACCUM
	LDI LOOP_CNTR,0x01												;Ответ размером в 1Б

_TASK_I2C_HANDLER__END:
	LDI ACCUM,0x00														;Вычисляем CRC8
	LDI_Z CRC8_MAXIM
	MCALL CRC8_BLOCK
	PUSH_X
	ADD XL,LOOP_CNTR
	ADC XH,C0x00
	ST X,ACCUM															;Записываем CRC
	INC LOOP_CNTR														;Инкрементируем размер ответа
	POP_X

.ifdef LOGGING_PORT													;Логируем
.if LOGGING_LEVEL	== LOGGING_LVL_PNC
	MOV TEMP,LOOP_CNTR
	MCALL C5_OUT_BYTE													;Длину ответа
	LDI TEMP,'>'
	MCALL C5_OUT_CHAR
	MOVW ZL,XL
	MOV TEMP,LOOP_CNTR
	MCALL C5_OUT_BYTES												;Блок ответа
	MCALL C5_OUT_CR													;Новая строка
.endif
.endif

	POP TEMP
	POP_Y
	POP ACCUM
	RET
