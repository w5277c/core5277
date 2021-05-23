;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.05.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.SET	CORE_FREQ										= 0x08	;Max: 8-ATMega16, 10-ATMega382

	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega8.inc"
	.SET	REPORT_INCLUDES								= 0x01

	;Важные, но не обязательные параметры ядра
	.SET	AVRA												= 0	;0-1
	.SET	REALTIME											= 0	;0-1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50NS
	.SET	C5_DRIVERS_QNT									= 3
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS											= 3	;0-4
	;.SET	LOGGING_PORT									= MISO
	;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/buttons.inc"
	.include	"./core/drivers/beeper.inc"
	.include	"./core/drivers/7segld.inc"
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
	.EQU	DIG4_PORT										= PB6		;Порт управления анодом цифры 4
	.EQU	SEGA_PORT										= PC4		;Порт управления катодом сегмента A
	.EQU	SEGB_PORT										= PC0		;Порт управления катодом сегмента B
	.EQU	SEGC_PORT										= PD3		;Порт управления катодом сегмента C
	.EQU	SEGD_PORT										= PC1		;Порт управления катодом сегмента D
	.EQU	SEGE_PORT										= PC2		;Порт управления катодом сегмента E
	.EQU	SEGF_PORT										= PC3		;Порт управления катодом сегмента F
	.EQU	SEGG_PORT										= PD4		;Порт управления катодом сегмента Q
	.EQU	BATTERY_PORT									= PC5		;Порт для анализа уровня заряда батареи
	.EQU	BUTTON_PORT										= PB1		;Порт для кнопки
	.EQU	INT_PORT											= PB0		;Порт ввода
	.EQU	BEEPER_PORT										= PD5		;Порт бипера (PD5/PD6/PD7)

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_BEEPER_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_7SEGLD_DRV									= 2|(1<<C5_PROCID_OPT_DRV)

	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS										= 0
	.EQU	TID_BEEPER										= 1
	.EQU	TID_7SEGLD										= 2
	;---

	;Дополнительно (по мере использования)
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"
	.include "./io/port_invert.inc"
	.include "./core/wait_2ms.inc"
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
	LDI ZH,high(DRV_BUTTONS_INIT)
	LDI ZL,low(DRV_BUTTONS_INIT)
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE

	;Инициализация BEEPER
	LDI PID,PID_BEEPER_DRV
	LDI ZH,high(DRV_BEEPER_INIT)
	LDI ZL,low(DRV_BEEPER_INIT)
	LDI ACCUM,BEEPER_PORT
	LDI FLAGS,TID_BEEPER
	MCALL C5_CREATE

	;Инициализация 7SEGLD
	LDI PID,PID_7SEGLD_DRV
	LDI ZH,high(DRV_7SEGLD_INIT)
	LDI ZL,low(DRV_7SEGLD_INIT)
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
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

SBI DDRB,MISO&0x0f

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK:

	LDI TEMP,PID_7SEGLD_DRV
	LDI FLAGS,DRV_7SEGLD_OP_SET_VAL
	LDI ACCUM,0b01100000
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	LDI ACCUM,0b11011010
	LDI TEMP_L,0x02
	MCALL C5_EXEC
	LDI ACCUM,0b11110010
	LDI TEMP_L,0x03
	MCALL C5_EXEC

	LDI TEMP_L,0x00
TASK_LOOP:
	LDI ACCUM,0b10000000
	MCALL C5_EXEC
	MCALL TASK_WAIT

	LDI ACCUM,0b01000000
	MCALL C5_EXEC
	MCALL TASK_WAIT

	LDI ACCUM,0b00000010
	MCALL C5_EXEC
	MCALL TASK_WAIT

	LDI ACCUM,0b00000100
	MCALL C5_EXEC
	MCALL TASK_WAIT

	RJMP TASK_LOOP

TASK_WAIT:
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32/2
	MCALL C5_WAIT_2MS

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
