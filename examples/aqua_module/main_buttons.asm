;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;19.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	.include	"./inc/drivers/buttons.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./inc/io/log_bytes.inc"
	.include	"./inc/io/logstr_new_line.inc"
	.include	"./inc/mem/eeprom_write_byte.inc"
	.include	"./inc/mem/eeprom_read_byte.inc"
	.include	"./inc/core/wait_1s.inc"
	.include	"./inc/mem/ram_fill8.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV						= 0|(1<<CORE5277_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS								= 0
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

	;Инициализация BUTTONS
	LDI PID,PID_BUTTONS_DRV
	LDI ZH,high(DRV_BUTTONS_INIT)
	LDI ZL,low(DRV_BUTTONS_INIT)
	LDI TEMP,DRV_BUTTONS_MODE_BUTTONS
	LDI ACCUM,TID_BUTTONS
	MCALL CORE5277_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL CORE5277_CREATE

	MJMP CORE5277_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI TEMP,PID_BUTTONS_DRV
	LDI ACCUM,PC1
	LDI FLAGS,DRV_BUTTONS_OP_ADD
	MCALL CORE5277_EXEC

	MCALL CORE5277_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_WAIT
	MCALL CORE5277_EXEC
	MCALL CORE5277_LOG_BYTE
	RJMP TASK__INFINITE_LOOP

