;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.10.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	TIMER_C_ENABLE							= 0	;0-1
	.INCLUDE "./devices/atmega328.inc"

	.SET	TS_MODE									= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	OPT_MODE									= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA										= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/1wire_s.inc"
	.INCLUDE "./core/drivers/ds18b20.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_char.inc"
	.include	"./core/io/out_sdnf.inc"
	.include	"./core/io/out_romstr.inc"
	.include	"./core/io/out_cr.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_1WIRE_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_DS18B20_DRV						= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация 1wire
	LDI PID,PID_1WIRE_DRV
	LDI_Z DRV_1WIRE_INIT
	LDI ACCUM,PC1
	MCALL C5_CREATE

	;Инициализация DS18B20
	LDI PID,PID_DS18B20_DRV
	LDI_Z DRV_DS18B20_INIT
	LDI ACCUM,PID_1WIRE_DRV
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	TASK__LOGSTR_ERROR:
	.db   "error",0x0d,0x0a,0x00

TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP_H,BYTE3(1000/2)
	LDI TEMP_L,BYTE2(1000/2)
	LDI TEMP,BYTE1(1000/2)
	MCALL C5_WAIT_2MS											;Ждем 1000мс

	LDI TEMP,PID_DS18B20_DRV
	MCALL C5_EXEC
	CPI TEMP_L,0xff
	BREQ TASK__ERROR

	MCALL C5_OUT_SDNF
	MCALL C5_OUT_CR
	RJMP TASK__INFINITE_LOOP

TASK__ERROR:
	C5_OUT_ROMSTR TASK__LOGSTR_ERROR
	RJMP TASK__INFINITE_LOOP
