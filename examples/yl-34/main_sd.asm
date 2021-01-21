;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./devices/atmega16.inc"
	.SET	CORE_FREQ								= 8	;2-20Mhz
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PB0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/sd.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_byte.inc"
	.include	"./core/wait_1s.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_SD_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
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

	;Инициализация SD
	LDI PID,PID_SD_DRV
	LDI ZH,high(DRV_SD_INIT)
	LDI ZL,low(DRV_SD_INIT)
	LDI TEMP_H,PB1
	LDI TEMP_L,PB2
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
	LDI TEMP,0x01
	MCALL C5_WAIT_1S

_TASK_INIT_SD:
	LDI TEMP,PID_SD_DRV
	LDI ACCUM,DRV_SD_CMD_INIT
	MCALL C5_EXEC

_TASK__LOOP:

	LDI TEMP,PID_SD_DRV
	LDI ACCUM,DRV_SD_CMD0
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	LDI TEMP,PID_SD_DRV
	LDI ACCUM,DRV_SD_CMD8											;47:40	b01001000
	LDI TEMP_EH,0x00													;39:32	0x00
	LDI TEMP_EL,0x00													;31:24	0x00
	LDI TEMP_H,0x01													;23:16	19:16=b0001 (2.7-3.6V)
	LDI TEMP_L,0xAA													;15-08	0x00
																			;07:00	CRC
	MCALL C5_EXEC

;	LDI TEMP,PID_SD_DRV
;	LDI ACCUM,DRV_SD_CMD5
;	LDI TEMP_EH,0x00
;	LDI TEMP_EL,0x00
;	LDI TEMP_H,0x00
;	LDI TEMP_L,0x00
;	MCALL C5_EXEC

	LDI TEMP,0x01
	MCALL C5_WAIT_1S

	RJMP _TASK_INIT_SD
