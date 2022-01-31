;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;31.01.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ								= 16
	.EQU	TIMER_C_ENABLE							= 1	;0-1
	.SET	AVRA										= 0	;0-1
	.SET	REPORT_INCLUDES						= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega328.inc"

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_TIME
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= SCK	;PA0-PC7
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.INCLUDE "./core/drivers/i2c_h.inc"
	.INCLUDE "/core/drivers/ds3231.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include "./core/io/out_uptime.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;---
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0|(1<<C5_PROCID_OPT_TIMER)
	;---
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	
	;Инициализация задачи тестирования(отработка по таймеру раз в секунду)
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(1000/2)>>0x10 & 0xff
	LDI TEMP_L,(1000/2)>>0x08 & 0xff
	LDI TEMP,  (1000/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

_TASK_STR1:
	.db "Started at:",0x00
_TASK_STR2:
	.db ", wait done at:",0x00

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x12														;xx:xx:xx xx.xx.xx0
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK_LOOP:
	LDI_Z _TASK_STR1|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_UPTIME

	;MCALL C5_OUT_TASKDUMP

	LDI TEMP_H,(1100/2)>>0x10 & 0xff
	LDI TEMP_L,(1100/2)>>0x08 & 0xff
	LDI TEMP,  (1100/2)>>0x00 & 0xff
	MCALL C5_WAIT_2MS

	LDI_Z _TASK_STR2|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_UPTIME
	MCALL C5_OUT_CR

	;MCALL C5_OUT_TASKDUMP

	;Засыпаем до следующего события от таймера
	MCALL C5_SUSPEND
	RJMP TASK_LOOP
