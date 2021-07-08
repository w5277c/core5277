;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.04.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

;BUILD: avra -I core5277 "eraser2.asm"
;FLASH: avrdude -p m328 -c avrispmkII -U flash:w:main.hex
;FLASH: avrdude -p m16 -c avrispmkII -U flash:w:main.hex

	.SET	CORE_FREQ								= 16	;Max: 8-ATMega16, 10-ATMega382

	.INCLUDE "./devices/atmega328.inc"
	.SET	REPORT_INCLUDES						= 0x01

	;Важные, но не обязательные параметры ядра
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	C5_DRIVERS_QNT							= 1
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE								= 0x0000;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PC5	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_DBG
	.SET	INPUT_PORT								= PC4

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/buttons.inc"
		;---
	;Блок задач
	;---
	;Дополнительно
	.include "./core/ui/menu.inc"
	.include	"./core/ui/hex32_input.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV						= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS								= 0
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
	LDI ZH,high(DRV_BUTTONS_INIT)
	LDI ZL,low(DRV_BUTTONS_INIT)
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI TEMP,PID_BUTTONS_DRV
	LDI ACCUM,PC1|(0x80)												;Включаем подтяжку к 5в
	LDI FLAGS,DRV_BUTTONS_OP_ADD
	MCALL C5_EXEC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_WAIT
	MCALL C5_EXEC
	MCALL C5_LOG_WORD
	RJMP TASK

