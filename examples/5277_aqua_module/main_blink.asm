;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.04.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm
	
	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	
	.SET	REALTIME									= 1	;0-1
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.INCLUDE "./io/port_mode_out.inc"
	.INCLUDE "./io/port_set_lo.inc"
	.INCLUDE "./io/port_invert.inc"
	.INCLUDE "./core/wait_2ms.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;---
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;---
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

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,PB4
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	MCALL PORT_INVERT
	
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS
	
	RJMP TASK__INFINITE_LOOP

