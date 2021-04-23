;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 0	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC4	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_byte.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./conv/crc8_block.inc"
	.include	"./conv/crc7_730.inc"
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
TASK__INIT:
	LDI ACCUM,0x10
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
_TASK__LOOP:

;	LDI TEMP,0x40
;	STD Z+0x00,TEMP
;	LDI TEMP,0x00
;	STD Z+0x01,TEMP
;;	STD Z+0x02,TEMP
	;STD Z+0x03,TEMP
	;STD Z+0x04,TEMP

	LDI TEMP,0x11
	STD Z+0x00,TEMP
	LDI TEMP,0x00
	STD Z+0x01,TEMP
	STD Z+0x02,TEMP
	LDI TEMP,0x09
	STD Z+0x03,TEMP
	LDI TEMP,0x00
	STD Z+0x04,TEMP


	LDI LOOP_CNTR,0x05
	MOV XH,ZH
	MOV XL,ZL
	LDI YH,high(CRC7_730)
	LDI YL,low(CRC7_730)
	MCALL CRC8_BLOCK

	;LSR ACCUM
	;LSL ACCUM
	ORI ACCUM,0x01

	MOV TEMP,ACCUM
	MCALL C5_LOG_BYTE

	RET
