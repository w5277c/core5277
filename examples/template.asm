;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2020  w5277c@gmail.com        Первая версия шаблона
;11.11.2020  w5277c@gmail.com        Актуализация
;-----------------------------------------------------------------------------------------------------------------------
	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_XXX_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_XXX									= 0
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS									= 2	;0-...
	.SET	BUFFER_SIZE								= 0x10;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;.INCLUDE "./core/drivers/xxx.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./io/port_mode_out.inc"
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"
	.include	"./core/wait_1s.inc"
	.include	"./core/log/log_bytes.inc"
	.include	"./core/log/log_romstr.inc"
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
;	LDI PID,PID_XXX_DRV
;	LDI ZH,high(DRV_XXX_INIT)
;	LDI ZL,low(DRV_XXX_INIT)
;	MCALL C5_CREATE


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
