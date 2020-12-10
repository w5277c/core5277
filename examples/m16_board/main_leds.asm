;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;31.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./devices/atmega16.inc"
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./io/port_mode_out.inc"
	.include	"./io/port_set_hi.inc"
	.include	"./io/port_set_lo.inc"
	.include	"./core/wait_1s.inc"
	;---

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

	;Инициализация задачи тестирования EEPROM
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,PA0
	MCALL PORT_MODE_OUT
;	LDI ACCUM,PD5
;	MCALL PORT_MODE_OUT
;	LDI ACCUM,PD6
;	MCALL PORT_MODE_OUT

	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI ACCUM,PA0
	MCALL PORT_SET_LO
	LDI ACCUM,PA0
	LDI TEMP,0x01
	MCALL C5_WAIT_1S
	MCALL PORT_SET_HI
	LDI TEMP,0x01
	MCALL C5_WAIT_1S

;	LDI ACCUM,PD4
;	MCALL PORT_SET_LO
;	LDI ACCUM,PD5
;	MCALL PORT_SET_HI
;	LDI TEMP,0x01
;	MCALL C5_WAIT_1S

;	LDI ACCUM,PD5
;	MCALL PORT_SET_LO
;	LDI ACCUM,PD6
;	MCALL PORT_SET_HI
;	LDI TEMP,0x01
;	MCALL C5_WAIT_1S

	RJMP TASK__INFINITE_LOOP

