;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.10.2020  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./devices/atmega328.inc"
	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/pcint_h.inc"
	.INCLUDE "./core/drivers/am2301.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_char.inc"
	.include	"./core/log/log_sdnf.inc"
	.include	"./core/log/log_romstr.inc"
	.include	"./core/log/log_cr.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_PCINT_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_AM2301_DRV							= 1|(1<<C5_PROCID_OPT_DRV)
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

	;Инициализация PCINT
	LDI PID,PID_PCINT_DRV
	LDI ZH,high(DRV_PCINT_H_INIT)
	LDI ZL,low(DRV_PCINT_H_INIT)
	MCALL C5_CREATE

	;Инициализация AM2301
	LDI PID,PID_AM2301_DRV
	LDI ZH,high(DRV_AM2301_INIT)
	LDI ZL,low(DRV_AM2301_INIT)
	LDI TEMP_H,PID_PCINT_DRV
	LDI TEMP_L,PC1
	MCALL C5_CREATE

	;Инициализация задачи тестирования AM2301
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
	LDI TEMP_L,high(1000/2)
	LDI TEMP,low(1000/2)
	MCALL C5_WAIT_2MS											;Ждем 1000мс

	LDI TEMP,PID_AM2301_DRV
	MCALL C5_EXEC
	CPI TEMP,DRV_AM2301_RESULT_OK
	BRNE TASK__ERROR

	MCALL C5_LOG_SDNF
	LDI TEMP,'/'
	MCALL C5_LOG_CHAR
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	MCALL C5_LOG_SDNF
	MCALL C5_LOG_CR
	RJMP TASK__INFINITE_LOOP

TASK__ERROR:
	C5_LOG_ROMSTR TASK__LOGSTR_ERROR
	RJMP TASK__INFINITE_LOOP
