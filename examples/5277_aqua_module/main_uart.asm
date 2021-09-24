;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.09.2020  w5277c@gmail.com			Начало
;28.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/uart_h.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_bytes.inc"
	.include	"./core/io/out_cr.inc"
	.include	"./mem/eeprom_write_byte.inc"
	.include	"./mem/eeprom_read_byte.inc"
	.include	"./core/wait_1s.inc"
	.include	"./mem/ram_fill.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_UART_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_UART									= 0
;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация UART
	LDI PID,PID_UART_DRV
	LDI_Z DRV_UART_H_INIT
	LDI TEMP_H,PD4
	LDI TEMP_L,0xff
	LDI YH,0xff
	LDI YL,0xff
	LDI TEMP_EH,TID_UART
	LDI TEMP_EL,DRV_UART_H_BAUDRATE_9600
	LDI ACCUM,C5_IR_INT0
	LDI FLAGS,(1<<DRV_UART_OPT_BREAK)

	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

	TASK_DATA:
	.db	"Hello world!",0x0d,0x0a,0x00

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x21
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI LOOP_CNTR,0x21
	LDI TEMP,0x00
	MOV XH,ZH
	MOV XL,ZL
	MCALL RAM_FILL

	LDI TEMP,PID_UART_DRV
	LDI_Z TASK_DATA|0x8000
	MOVW XL,YL
	LDI TEMP_EH,0x0e
	LDI TEMP_EL,0x20
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE TASK__INFINITE_LOOP

	MOV YH,ZH
	MOV YL,ZL
	MCALL C5_OUT_STR
	RJMP TASK__INFINITE_LOOP

