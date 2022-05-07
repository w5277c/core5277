;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.05.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	FW_VERSION										= 1
	.SET	CORE_FREQ										= 8	;2-20
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.SET	AVRA												= 1	;0-1
	.SET	REPORT_INCLUDES								= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega8.inc"

	;Важные, но не обязательные параметры ядра
;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_EVENT		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.EQU	OPT_MODE											= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US
	.SET	TIMERS											= 4		;0-...
	.SET	LOGGING_PORT									= SCK

	;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/buttons.inc"
	.include	"./core/drivers/7segld.inc"
	.include	"./core/drivers/counter.inc"
	.include	"./core/drivers/adc.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	;---

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	DIG1_PORT										= PD0		;Порт управления анодом цифры 1
	.EQU	DIG2_PORT										= PD1		;Порт управления анодом цифры 2
	.EQU	DIG3_PORT										= PD2		;Порт управления анодом цифры 3
	.EQU	DIG4_PORT										= PB1		;Порт управления анодом цифры 4
	.EQU	SEGA_PORT										= PC4		;Порт управления катодом сегмента A
	.EQU	SEGB_PORT										= PC0		;Порт управления катодом сегмента B
	.EQU	SEGC_PORT										= PD4		;Порт управления катодом сегмента C
	.EQU	SEGD_PORT										= PC1		;Порт управления катодом сегмента D
	.EQU	SEGE_PORT										= PC2		;Порт управления катодом сегмента E
	.EQU	SEGF_PORT										= PC3		;Порт управления катодом сегмента F
	.EQU	SEGG_PORT										= PB2		;Порт управления катодом сегмента Q
	.EQU	SEGP_PORT										= PB6		;Порт управления катодом сегмента P
	.EQU	BATTERY_PORT									= PC5		;Порт для анализа уровня заряда батареи
	.EQU	BUTTON_PORT										= PD3		;Порт для кнопки
	.EQU	INT_PORT											= PB0		;Порт ввода

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_7SEGLD_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_COUNTER_DRV								= 2|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_DRV										= 3|(1<<C5_PROCID_OPT_DRV)

	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS										= 0
	.EQU	TID_7SEGLD										= 1
	.EQU	TID_COUNTER										= 2
	.EQU	TID_ADC											= 3
	;---

	;Дополнительно (по мере использования)
	.include "./mem/eeprom_read_byte.inc"
	.include "./mem/eeprom_write_byte.inc"
	.include	"./conv/num16_to_strf.inc"
	.include	"./conv/num_to_7seg.inc"
	.include	"./core/wait_1s.inc"
	.include "./core/uptime_copy.inc"
	.include "./core/uptime_delta.inc"
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация BUTTONS
	LDI PID,PID_BUTTONS_DRV
	LDI_Z DRV_BUTTONS_INIT
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_ADD
	LDI ACCUM,BUTTON_PORT|0x80
	MCALL C5_EXEC

	;Инициализация COUNTER
	LDI PID,PID_COUNTER_DRV
	LDI_Z DRV_COUNTER_INIT
	LDI TEMP_H,PB0|0x80
	LDI TEMP_L,TID_COUNTER
	MCALL C5_CREATE

	;Инициализация ADC(уровень напряжения батареи)
	LDI PID,PID_ADC_DRV
	LDI_Z DRV_ADC_INIT
	LDI ACCUM,TID_ADC
	MCALL C5_CREATE

	;Инициализация 7SEGLD
	LDI PID,PID_7SEGLD_DRV
	LDI_Z DRV_7SEGLD_INIT
	LDI TEMP_EH,SEGA_PORT
	LDI TEMP_EL,SEGB_PORT
	LDI TEMP_H,SEGC_PORT
	LDI TEMP_L,SEGD_PORT
	LDI XH,SEGE_PORT
	LDI XL,SEGF_PORT
	LDI YH,SEGG_PORT
	LDI YL,SEGP_PORT
	LDI ACCUM,TID_7SEGLD
	LDI LOOP_CNTR,0x04
	LDI FLAGS,DRV_7SEGLD_MODE_HDIG_HSEG
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	;Выделяем память под UPTIME,последнее значение счетчика и буфер для преобразования числа в строку
	LDI ACCUM,0x05+0x02+0x05
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	;Обновляем временную метку
	MCALL C5_UPTIME_COPY
	;Устанавливаем старое значение счетчика в 0(для отслеживания изменения значения счетчика)
	STD Y+0x05,C0x00
	STD Y+0x06,C0x00

	;Устаналвиваем и включаем порты цифр
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_PORT
	LDI ACCUM,DIG1_PORT
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI ACCUM,DIG2_PORT
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDI ACCUM,DIG3_PORT
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDI ACCUM,DIG4_PORT
	LDI TEMP_L,0x03
	MCALL C5_EXEC

	;Получаем текущее напряжение батареи и выводим на дисплей
	MCALL _TASK_GET_BAT_V
	LDI LOOP_CNTR,0b00000100
	MCALL _TASK_LED_UPDATE
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00111000
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	;Ждем секунду
	LDI TEMP,0x01
	MCALL C5_WAIT_1S

	;Считываем последнее значение счетчика, и выводи на дисплей(если есть значение)
	MCALL _TASK_READ_LAST_VALUE
	CPI TEMP_H,0xff
	BREQ _TASK__NO_LAST_VALUE
	LDI LOOP_CNTR,0x00
	MCALL _TASK_LED_UPDATE
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00011000
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	;Ждем 2 секунды
	LDI TEMP,0x02
	MCALL C5_WAIT_1S

_TASK__NO_LAST_VALUE:
	;Выводим нули - начало отсчета
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0x00
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDI TEMP_L,0x03
	MCALL C5_EXEC

	;Очищаем очередь кнопки
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_CLEAR
	MCALL C5_EXEC

TASK_LOOP:
	;Получаем напряжение батареи
	LDI ACCUM,0b10000000
	MCALL _TASK_GET_BAT_V
	CPI TEMP_H,0x00
	BRNE PC+0x07
	CPI TEMP_L,100
	BRCC PC+0x05
	LDI ACCUM,0b00000010
	CPI TEMP_L,80
	BRCC PC+0x02
	LDI ACCUM,0b00010000

	;32 итераций без опроса батареи
	LDI LOOP_CNTR,0x20
_TASK__LOOP2:
	;Отображаем заряд батареи на дисплее
	LDI TEMP_L,0x00
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE
	PUSH ACCUM
	LDI TEMP_L,0x00
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00000000
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE

	;Считываем очередь нажатий кнопки
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_GET
	MCALL C5_EXEC
	POP ACCUM
	;Следующая итерация если нажатий не было
	CPI TEMP_H,0xff
	BREQ _TASK__LOOP2_NEXT
	;Обновляем временную метку
	MCALL C5_UPTIME_COPY
	;Проверка на единичное короткое нажатие
	CPI TEMP_L,0b00010000
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _TASK_SHUTDOWN
	RJMP TASK
_TASK_ITERATION__NO_SHORT_PRESS:
	;Проверка на единичное длинное нажатие
	CPI TEMP_L,0b00010001
	BRNE _TASK__LOOP2_NEXT
_TASK_ITERATION__POWER_OFF:
	MCALL _TASK_SHUTDOWN
	;Переходим в Idle mode
	LDS TEMP,GICR
	ORI TEMP,(1<<INT1)
	STS GICR,TEMP
	LDI TEMP,(1<<SM2)|(1<<SM1)|(0<<SM0)|(1<<SE)|(0<<ISC11)|(0<<ISC10)
	STS MCUCR,TEMP
	SLEEP
	LDS TEMP,GICR
	ANDI TEMP,low(~(1<<INT1))
	STS GICR,TEMP
	RJMP TASK

_TASK__LOOP2_NEXT:
	DEC LOOP_CNTR
	BRNE _TASK__LOOP2
	;Проверяем на 3 мин. таймаут
	MCALL C5_UPTIME_DELTA
	CPI TEMP_EH,0x00
	BRNE _TASK_ITERATION__POWER_OFF
	CPI TEMP_EL,0x00
	BRNE _TASK_ITERATION__POWER_OFF
	CPI TEMP_H,0x01
	BRCS _TASK__NO_TIMEOUT
	BRNE _TASK_ITERATION__POWER_OFF
	CPI TEMP_L,0x5f
	BRCS _TASK__NO_TIMEOUT
	BRNE _TASK_ITERATION__POWER_OFF
	CPI TEMP,0x90
	BRCS _TASK__NO_TIMEOUT
	BRNE _TASK_ITERATION__POWER_OFF
_TASK__NO_TIMEOUT:
	RJMP TASK_LOOP

;--------------------------------------------------------
TASK_COUNTER_UPDATE:
;--------------------------------------------------------
;Получаем значение счетчика и выводим на экран,
;выдерживаем паузу
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR

	LDI TEMP,PID_COUNTER_DRV
	LDI FLAGS,DRV_COUNTER_OP_GET
	MCALL C5_EXEC
	LDD TEMP,Y+0x05
	CP TEMP,TEMP_H
	BRNE PC+0x04
	LDD TEMP,Y+0x06
	CP TEMP,TEMP_L
	BREQ PC+0x01+_MCALL_SIZE+0x02
	;Счетчик изменился, обновляем временную метку
	MCALL C5_UPTIME_COPY
	STD Y+0x05,TEMP_H
	STD Y+0x06,TEMP_L

	LDI LOOP_CNTR,0x00
	MCALL _TASK_LED_UPDATE

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32/2
	MCALL C5_WAIT_2MS

	POP LOOP_CNTR
	POP_FT
	RET

;--------------------------------------------------------
_TASK_SHUTDOWN:
;--------------------------------------------------------
;Заверение работы(сброс или выключение)
;--------------------------------------------------------
	;Выключаем дисплей
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDI TEMP_L,0x03
	MCALL C5_EXEC

	LDI TEMP,PID_COUNTER_DRV
	LDI FLAGS,DRV_COUNTER_OP_GET
	MCALL C5_EXEC

	CPI TEMP_H,0x00
	BRNE PC+0x03
	CPI TEMP_L,0x00
	BREQ PC+0x01+_MCALL_SIZE
	MCALL _TASK_WRITE_LAST_VALUE

	LDI TEMP,PID_COUNTER_DRV
	LDI FLAGS,DRV_COUNTER_OP_RESET
	MCALL C5_EXEC

	RET

;--------------------------------------------------------
_TASK_GET_BAT_V:
;--------------------------------------------------------
;OUT: TEMP_H/L-напряжение батареи в сотых вольта
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH ACCUM

	LDI TEMP,PID_ADC_DRV
	LDI ACCUM,ADC5
	LDI TEMP_EH,ADC_VREF_AVCC
	LDI TEMP_EL,DRV_ADC_PRESC_AUTO
	LDI TEMP_H,0x04
	LDI TEMP_L,0x85
	LDI_X 0x0000
	MCALL C5_EXEC

	LDI TEMP,10
	MCALL MUL16X8
	LDI TEMP,31
	MCALL DIV16X8

	POP ACCUM
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET

;--------------------------------------------------------
_TASK_LED_UPDATE:
;--------------------------------------------------------
;IN: TEMP_H/L-значение,LOOP_CNTR-точки
;--------------------------------------------------------
	PUSH_Z
	PUSH_FT
	PUSH ACCUM
	PUSH FLAGS

	MOVW ZL,YL
	ADIW ZL,0x07
	MCALL NUM16_TO_STRF
	LDD TEMP,Y+0x09
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	SBRC LOOP_CNTR,0x02
	ORI ACCUM,0x01
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDD TEMP,Y+0x0a
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	SBRC LOOP_CNTR,0x01
	ORI ACCUM,0x01
	LDI TEMP,PID_7SEGLD_DRV
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDD TEMP,Y+0x0b
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	SBRC LOOP_CNTR,0x00
	ORI ACCUM,0x01
	LDI TEMP,PID_7SEGLD_DRV
	LDI TEMP_L,0x03
	MCALL C5_EXEC

	POP FLAGS
	POP ACCUM
	POP_FT
	POP_Z
	RET

;--------------------------------------------------------
_TASK_WRITE_LAST_VALUE:
;--------------------------------------------------------
;Сохраняем в EEPROM значение счетчика
;IN: TEMP_H/L-значение
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH TEMP_H

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	POP TEMP
	MCALL EEPROM_WRITE_BYTE
	INC TEMP_L
	POP TEMP
	MCALL EEPROM_WRITE_BYTE

	POP TEMP
	RET

;--------------------------------------------------------
_TASK_READ_LAST_VALUE:
;--------------------------------------------------------
;Чтение из EEPROM последнее значение
;OUT: TEMP_H/L-значение
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL EEPROM_READ_BYTE
	PUSH TEMP
	INC TEMP_L
	MCALL EEPROM_READ_BYTE
	MOV TEMP_L,TEMP
	POP TEMP_H
	POP TEMP
	RET
