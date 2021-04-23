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
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x0000;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_PNC
	.SET	INPUT_PORT								= PB1

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/wait_1s.inc"
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_ramdump.inc"
	.include "./io/input_get_char.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./prim/common.inc"
	.include	"./prim/prim_add.inc"
	.include	"./prim/prim_sub.inc"
	.include	"./prim/prim_cp.inc"
	.include	"./prim/prim_cpi.inc"
	.include	"./prim/prim_mov.inc"
	.include	"./core/log/log_prim.inc"
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
	LDI TEMP_H,0x00
	LDI TEMP_L,0x0c
	MCALL C5_LOG_RAMDUMP

	LDI ACCUM,PRIM_INT

	LDI TEMP,VAR1
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR

	LDI TEMP,VAR2
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR


	LDI TEMP_H,VAR1
	LDI TEMP_L,VAR2
	MCALL PRIM_SUB
	LDI TEMP,VAR1
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR

	PRIM_LDI PRIM_INT,VAR2,1000

	LDI TEMP_H,VAR1
	LDI TEMP_L,VAR2
	MCALL PRIM_SUB
	LDI TEMP,VAR1
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR

	PRIM_LDI PRIM_INT,VAR2,5

	LDI TEMP_H,VAR1
	LDI TEMP_L,VAR2
	MCALL PRIM_ADD
	LDI TEMP,VAR1
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR

	LDI TEMP_H,VAR2
	LDI TEMP_L,VAR1
	MCALL PRIM_MOV

	LDI TEMP,VAR1
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR
	LDI TEMP,VAR2
	MCALL C5_LOG_PRIM
	MCALL C5_LOG_CR

	LDI TEMP_H,0x00
	LDI TEMP_L,0x0c
	MCALL C5_LOG_RAMDUMP

	LDI TEMP_H,VAR2
	LDI TEMP_L,VAR1
	MCALL PRIM_CP
	BRNE L2
	LDI TEMP,'='
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CR
L2:
	PRIM_LDI PRIM_INT,VAR3,256
	LDI TEMP_H,VAR1
	LDI TEMP_L,VAR3
	MCALL PRIM_SUB

	LDI TEMP_H,0x00
	LDI TEMP_L,0x0c
	MCALL C5_LOG_RAMDUMP

	LDI TEMP_H,VAR1
	LDI TEMP_L,VAR2
	MCALL PRIM_CP
	BRNE L3
	LDI TEMP,'='
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CR
	RJMP L4
L3:
	BRCC L4
	LDI TEMP,'<'
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CR
L4:


L1:RJMP L1

