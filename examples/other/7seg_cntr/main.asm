;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.05.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.SET	CORE_FREQ										= 0x08	;Max: 8-ATMega16, 10-ATMega382
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
	.SET	C5_DRIVERS_QNT									= 5
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS											= 5		;0-...
	;.SET	LOGGING_PORT									= MOSI
	;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/buttons.inc"
	.include	"./core/drivers/beeper.inc"
	.include	"./core/drivers/7segld.inc"
	.include	"./core/drivers/counter.inc"
	.include	"./core/drivers/adc.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	;---
	.include "./core/log/log_ramdump.inc"

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
	.EQU	BEEPER_PORT										= PD6		;Порт бипера

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_BEEPER_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_7SEGLD_DRV									= 2|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_COUNTER_DRV								= 3|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_DRV										= 4|(1<<C5_PROCID_OPT_DRV)

	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS										= 0
	.EQU	TID_BEEPER										= 1
	.EQU	TID_7SEGLD										= 2
	.EQU	TID_COUNTER										= 3
	.EQU	TID_ADC											= 4
	;---

	;Дополнительно (по мере использования)
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"
	.include "./io/port_invert.inc"
	.include "./core/wait_2ms.inc"
	.include	"./conv/num16_to_strf.inc"
	.include	"./conv/num_to_7seg.inc"
	.include	"./core/wait_1s.inc"
	;---

BEEPER_SND1:
	.db N64,H3,H4,H5,E,0x00
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

	;Инициализация BEEPER
	LDI PID,PID_BEEPER_DRV
	LDI_Z DRV_BEEPER_INIT
	LDI ACCUM,BEEPER_PORT
	LDI FLAGS,TID_BEEPER
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

;--------------------------------------------------------
TASK_START_SND:
	.db N64T,F5,E,0x00
;--------------------------------------------------------;Задача
TASK__INIT:

	LDI ACCUM,0x05
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	MCALL _TASK_GET_BAT_V
	MCALL _TASK_LED_UPDATE

	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b00111000
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	LDI TEMP,0x03
	MCALL C5_WAIT_1S

;Большое потребление тока, устройство перезагружается
;	MCALL _TASK_DISPLAY_OFF
;	LDI TEMP,PID_BEEPER_DRV
;	LDI_Z TASK_START_SND|0x8000
;	MCALL C5_EXEC

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
	CPI TEMP_L,130
	BRCC PC+0x05
	LDI ACCUM,0b00000010
	CPI TEMP_L,110
	BRCC PC+0x02
	LDI ACCUM,0b00010000

	LDI LOOP_CNTR,0x03
_TASK__LOOP2:
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE
	PUSH ACCUM
	LDI ACCUM,0b00000000
	MCALL C5_EXEC
	MCALL TASK_COUNTER_UPDATE
	POP ACCUM
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
;OUT: TEMP_H/L-Напряжение батареи в сотых вольта
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH ACCUM

	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0xff
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	LDI TEMP,PID_ADC_DRV
	LDI ACCUM,ADC5
	LDI TEMP_EH,ADC_VREF_AVCC
	LDI TEMP_EL,DRV_ADC_PRESC_AUTO
	LDI TEMP_H,0x04
	LDI TEMP_L,0x85
	LDI_X 0x0000
	MCALL C5_EXEC
	
	LDI TEMP,0x0a
	MCALL MUL16X8
	LDI TEMP,31
	MCALL DIV16X8

	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0xff
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	POP TEMP_L
	POP TEMP_H
	
	POP ACCUM
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET

;--------------------------------------------------------
_TASK_LED_UPDATE:
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
_TASK_DISPLAY_OFF:
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS
	PUSH ACCUM
	PUSH TEMP_L
	
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

	POP TEMP_L
	POP ACCUM
	POP FLAGS
	POP TEMP
	RET
