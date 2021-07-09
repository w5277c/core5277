;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.05.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.SET	CORE_FREQ										= 0x08
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.INCLUDE "./devices/atmega8.inc"

	;Важные, но не обязательные параметры ядра
;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.SET	REPORT_INCLUDES								= 0x01
	.EQU	TS_MODE											= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.EQU	OPT_MODE											= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA												= 0		;0-1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US
	.SET	TIMERS											= 4		;0-...
	.SET	LOGGING_PORT									= MOSI

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
	.EQU	SEGG_PORT										= PB2		;Порт управления катодом сегмента Q !!!
	.EQU	BATTERY_PORT									= PC5		;Порт для анализа уровня заряда батареи
	.EQU	BUTTON_PORT										= PD3		;Порт для кнопки		!!!
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
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	CLI
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация BUTTONS
	LDI PID,PID_BUTTONS_DRV
	LDI_Z DRV_BUTTONS_INIT
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE

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
	LDI YL,0xff
	LDI ACCUM,TID_7SEGLD
	LDI LOOP_CNTR,0x04
	LDI FLAGS,DRV_7SEGLD_MODE_HDIG_HSEG
	MCALL C5_CREATE
	;Устаналвиваем порты цифр
	LDI TEMP,PID_7SEGLD_DRV
	LDI ACCUM,DIG1_PORT
	LDI FLAGS,DRV_7SEGLD_OP_SET_PORT
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

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x05
	MCALL C5_RAM_REALLOC

	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_ADD
	LDI ACCUM,BUTTON_PORT|0x80
	MCALL C5_EXEC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0xff
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	MCALL _TASK_GET_BAT_V
	MCALL _TASK_LED_UPDATE
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00111000
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI TEMP,0x03
	MCALL C5_WAIT_1S

	MCALL _TASK_READ_LAST_VALUE
	CPI TEMP_H,0xff
	BREQ _TASK__NO_LAST_VALUE
	MCALL _TASK_LED_UPDATE
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00011100
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI TEMP,0x03
	MCALL C5_WAIT_1S

_TASK__NO_LAST_VALUE:
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0x00
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDI TEMP_L,0x03
	MCALL C5_EXEC

TASK_LOOP:
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

	LDI LOOP_CNTR,0x03
_TASK__LOOP2:
	LDI TEMP_L,0x00
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE
	PUSH ACCUM
	LDI ACCUM,0b00000000
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE

	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_GET
	MCALL C5_EXEC
	POP ACCUM
	CPI TEMP_H,0xff
	BREQ _TASK__LOOP2_NEXT
	;Проверка на короткое нажатие
	CPI TEMP_L,0b00010000
	BRNE _TASK_ITERATION__NO_SHORT_PRESS
	MCALL _TASK_WRITE_LAST_VALUE
	RJMP TASK
_TASK_ITERATION__NO_SHORT_PRESS:
	;Проверка на единичное длинное нажатие
	CPI TEMP_L,0b00010001											
	BRNE _TASK__LOOP2_NEXT
	MCALL _TASK_WRITE_LAST_VALUE
	;TODO POWER OFF

_TASK__LOOP2_NEXT:
	DEC LOOP_CNTR
	BRNE _TASK__LOOP2
	RJMP TASK_LOOP

TASK_COUNTER_UPDATE:
	PUSH_FT

	LDI TEMP,PID_COUNTER_DRV
	LDI FLAGS,DRV_COUNTER_OP_GET
	MCALL C5_EXEC
	MCALL _TASK_LED_UPDATE

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x64/2
	MCALL C5_WAIT_2MS

	POP_FT
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
	LDI TEMP_EH,ADC_VREF_2_56_CAP
	LDI TEMP_EL,DRV_ADC_PRESC_AUTO
	LDI TEMP_H,0x04
	LDI TEMP_L,0x85
	LDI_X 0x0000
	MCALL C5_EXEC

	LSR TEMP_H
	ROR TEMP_L
	LSR TEMP_H
	ROR TEMP_L
		
	POP ACCUM
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET

;--------------------------------------------------------
_TASK_LED_UPDATE:
;--------------------------------------------------------
;IN: TEMP_H/L-значение
;--------------------------------------------------------
	PUSH_Z
	PUSH_FT
	PUSH ACCUM

	MOVW ZL,YL
	MCALL NUM16_TO_STRF
	LDD TEMP,Y+0x02
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDD TEMP,Y+0x03
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	LDI TEMP,PID_7SEGLD_DRV
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDD TEMP,Y+0x04
	SUBI TEMP,0x30
	MCALL NUM_TO_7SEG
	MOV ACCUM,TEMP
	LDI TEMP,PID_7SEGLD_DRV
	LDI TEMP_L,0x03
	MCALL C5_EXEC

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
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP,PID_COUNTER_DRV
	LDI FLAGS,DRV_COUNTER_OP_GET
	MCALL C5_EXEC
	PUSH TEMP_L
	PUSH TEMP_H
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	POP TEMP
	MCALL EEPROM_WRITE_BYTE
	INC TEMP_L
	POP TEMP
	MCALL EEPROM_WRITE_BYTE

	POP TEMP_L
	POP TEMP_H
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
