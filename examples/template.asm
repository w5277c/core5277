;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2020  w5277c@gmail.com        Первая версия шаблона
;11.11.2020  w5277c@gmail.com        Актуализация
;-----------------------------------------------------------------------------------------------------------------------
	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS	;25/50ns
	.SET	TIMERS									= 2	;0-...
	.SET	BUFFER_SIZE								= 0x10;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	;.INCLUDE "./inc/drivers/xxx.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./inc/io/port_mode_out.inc"
	.include "./inc/io/port_set_hi.inc"
	.include "./inc/io/port_set_lo.inc"
	.include	"./inc/core/wait_1s.inc"
	.include	"./inc/io/log_bytes.inc"
	.include	"./inc/io/log_romstr.inc"
	.include	"./inc/io/logstr_new_line.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_XXX_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_XXX									= 0
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

	;Инициализация драйвера
	LDI PID,PID_XXX_DRV
	LDI ZH,high(DRV_XXX_INIT)
	LDI ZL,low(DRV_XXX_INIT)
	MCALL C5_CREATE


	;Инициализация задачи
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	;TODO


	LDI TEMP,0x01														;Пауза в 1 сеунду
	MCALL C5_WAIT_1S
	RJMP TASK__INFINITE_LOOP

