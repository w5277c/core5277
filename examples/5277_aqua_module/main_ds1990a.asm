;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.09.2020  w5277c@gmail.com			Начало
;28.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/1wire_s.inc"
	.INCLUDE "./core/drivers/ds1990a.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_bytes.inc"
	.include	"./core/io/out_romstr.inc"
	.include	"./core/io/out_cr.inc"
	.include	"./core/wait_2ms.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_1WIRE_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_DS1990A_DRV						= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация 1WIRE
	LDI PID,PID_1WIRE_DRV
	LDI ZH,high(DRV_1WIRE_INIT)
	LDI ZL,low(DRV_1WIRE_INIT)
	LDI ACCUM,PC1
	MCALL C5_CREATE

	;Инициализация DS1990A
	LDI PID,PID_DS1990A_DRV
	LDI ZH,high(DRV_DS1990A_INIT)
	LDI ZL,low(DRV_DS1990A_INIT)
	LDI ACCUM,PID_1WIRE_DRV
	MCALL C5_CREATE

	;Инициализация задачи тестирования DS1990A
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	TASK__LOGSTR_ERROR:
	.db   "error",0x0d,0x0a,0x00

TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,(200/2)
	MCALL C5_WAIT_2MS											;Ждем ~100+200мс

	LDI TEMP,PID_DS1990A_DRV
	MCALL C5_EXEC
	CPI TEMP,DRV_DS1990A_BUFFER_CRC
	BREQ TASK__ERROR
	CPI TEMP,DRV_DS1990A_RESULT_OK
	BRNE TASK__INFINITE_LOOP

	LDI TEMP,0x08
	MOV YH,XH
	MOV YL,XL
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR
	RJMP TASK__INFINITE_LOOP
TASK__ERROR:
	C5_OUT_ROMSTR TASK__LOGSTR_ERROR
	RJMP TASK__INFINITE_LOOP

