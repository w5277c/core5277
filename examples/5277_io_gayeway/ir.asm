;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.06.2021	w5277c@gmail.com			Начало
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
	.SET	TS_MODE									= TS_MODE_TIME
	.SET	C5_DRIVERS_QNT							= 1
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x0000;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PC4	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_DBG

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/ir.inc"
		;---
	;Блок задач
	;---
	;Дополнительно

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_IR_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_IR									= 0
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

	;Инициализация IR
	LDI PID,PID_IR_DRV
	LDI ZH,high(DRV_IR_INIT)
	LDI ZL,low(DRV_IR_INIT)
	LDI TEMP_H,PD3
	LDI TEMP_L,PB3
	LDI TEMP_EH,C5_IR_INT1
	LDI TEMP_EL,PB2
	LDI ACCUM,TID_IR
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:

	LDI ACCUM,0x10
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI TEMP,PID_IR_DRV
	MOVW YL,ZL
	MOVW XL,ZL
	LDI TEMP_EH,0x00*0x08
	LDI TEMP_EL,0x10*0x08
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	CPI TEMP_H,DRV_IR_READY
	BRNE TASK
	CPI TEMP_L,0x00
	BREQ TASK

	MOV TEMP,TEMP_L
	MCALL C5_LOG_BYTE
	LDI TEMP,':'
	MCALL C5_LOG_CHAR
	LDI TEMP,0x10
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
	RJMP TASK

