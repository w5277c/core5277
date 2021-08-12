;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 8	;2-20Mhz
	.INCLUDE "./devices/atmega16.inc"
	;Важные, но не обязательные параметры ядра
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	C5_DRIVERS_QNT							= 0
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 0	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE								= 0x0000;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_PNC
	.SET	INPUT_PORT								= PD2

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/wait_1s.inc"
	.include	"./core/io/out_cr.inc"
	.include	"./core/io/out_ramdump.inc"
	.include "./io/input_get.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./prim/common.inc"
	.include	"./prim/prim_add.inc"
	.include	"./prim/prim_sub.inc"
	.include	"./prim/prim_cp.inc"
	.include	"./prim/prim_cpi.inc"
	.include	"./prim/prim_mov.inc"
	.include	"./core/io/out_prim.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
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
	.EQU	VAR1										= 0x00;4B
	.EQU	VAR2										= 0x04;4B
	.EQU	VAR3										= 0x08;4B
	.EQU	RAM_SIZE									= 0x0c;

TASK__INIT:
	LDI ACCUM,RAM_SIZE
	MCALL C5_RAM_REALLOC

	PRIM_LDI PRIM_INT,VAR1,0xffffffff
	PRIM_LDI PRIM_INT,VAR2,1

	MCALL C5_READY
;--------------------------------------------------------
_TASK__LOOP:

	LDI TEMP,'A'
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL INPUT_WAIT_CHAR
	MCALL C5_OUT_BYTE
	PUSH TEMP
	LDI TEMP,'['
	MCALL C5_OUT_CHAR
	POP TEMP
	MCALL C5_OUT_CHAR
	LDI TEMP,']'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_CR


	LDI TEMP,'B'
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL INPUT_WAIT_CHAR
	MCALL C5_OUT_BYTE
	PUSH TEMP
	LDI TEMP,'['
	MCALL C5_OUT_CHAR
	POP TEMP
	MCALL C5_OUT_CHAR
	LDI TEMP,']'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_CR


	LDI TEMP,'L'
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL INPUT_LISTEN
L1:
	MCALL INPUT_GET_CHAR
	CPI TEMP,0x00
	BREQ L1
	MCALL C5_OUT_BYTE
	PUSH TEMP
	LDI TEMP,'['
	MCALL C5_OUT_CHAR
	POP TEMP
	MCALL C5_OUT_CHAR
	LDI TEMP,']'
	MCALL C5_OUT_CHAR
	RJMP L1

