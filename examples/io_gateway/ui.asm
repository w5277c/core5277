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
	.SET	C5_DRIVERS_QNT							= 0
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 0	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x0000;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PC5	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_DBG
	.SET	INPUT_PORT								= PC4

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
		;---
	;Блок задач
	;---
	;Дополнительно
	.include "./core/ui/menu.inc"
	.include	"./core/ui/hex32_input.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
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

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

TASK__MENU:
	.db	"Select action:",0x00,"Action 1",0x00,"Action 2",0x00,"Action 3",0x00

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI_Y TASK__MENU|0x8000
	LDI ACCUM,0x03
	MCALL C5_MENU

	LDI TEMP_EH,0x12
	LDI TEMP_EL,0x34
	LDI TEMP_H,0x56
	LDI TEMP_L,0x78
	MCALL C5_HEX32_INPUT
	RJMP TASK

