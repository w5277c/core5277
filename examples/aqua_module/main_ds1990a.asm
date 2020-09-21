;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;13.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	.INCLUDE "./inc/drivers/1wire_s.inc"
	.INCLUDE "./inc/drivers/ds1990a.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./inc/io/log_bytes.inc"
	.include	"./inc/io/log_romstr.inc"
	.include	"./inc/io/logstr_new_line.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_1WIRE_DRV							= 0|(1<<CORE5277_PROCID_OPT_DRV)
	.EQU	PID_DS1990A_DRV						= 1|(1<<CORE5277_PROCID_OPT_DRV)
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
	MCALL CORE5277_INIT

	;Инициализация 1WIRE
	LDI PID,PID_1WIRE_DRV
	LDI ZH,high(DRV_1WIRE_INIT)
	LDI ZL,low(DRV_1WIRE_INIT)
	LDI ACCUM,PC1
	MCALL CORE5277_CREATE

	;Инициализация DS1990A
	LDI PID,PID_DS1990A_DRV
	LDI ZH,high(DRV_DS1990A_INIT)
	LDI ZL,low(DRV_DS1990A_INIT)
	LDI ACCUM,PID_1WIRE_DRV
	MCALL CORE5277_CREATE

	;Инициализация задачи тестирования DS1990A
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL CORE5277_CREATE

	MJMP CORE5277_START

;--------------------------------------------------------;Задача
	TASK__LOGSTR_ERROR:
	.db   "error",0x0d,0x0a,0x00

TASK__INIT:
	MCALL CORE5277_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,(200/2)
	MCALL CORE5277_WAIT_2MS											;Ждем ~100+200мс

	LDI TEMP,PID_DS1990A_DRV
	MCALL CORE5277_EXEC
	CPI TEMP,DRV_DS1990A_BUFFER_CRC
	BREQ TASK__ERROR
	CPI TEMP,DRV_DS1990A_RESULT_OK
	BRNE TASK__INFINITE_LOOP

	LDI TEMP,0x08
	MOV YH,XH
	MOV YL,XL
	MCALL CORE5277_LOG_BYTES
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE
	RJMP TASK__INFINITE_LOOP
TASK__ERROR:
	CORE5277_LOG_ROMSTR TASK__LOGSTR_ERROR
	RJMP TASK__INFINITE_LOOP

