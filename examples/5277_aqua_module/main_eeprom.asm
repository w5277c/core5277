;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.09.2020  w5277c@gmail.com        Начало
;28.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	TIMER_C_ENABLE							= 0	;0-1
	.INCLUDE "./devices/atmega328.inc"

	.SET	TS_MODE									= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	OPT_MODE									= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA										= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_bytes.inc"
	.include	"./core/log/log_cr.inc"
	.include	"./mem/eeprom_write_byte.inc"
	.include	"./mem/eeprom_read_byte.inc"
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
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x56
	MCALL EEPROM_WRITE_BYTE
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI TEMP,0x67
	MCALL EEPROM_WRITE_BYTE


	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL EEPROM_READ_BYTE
	PUSH TEMP

	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	MCALL EEPROM_READ_BYTE
	MOV TEMP_L,TEMP
	POP TEMP_H

	MCALL C5_LOG_WORD
	MCALL C5_LOG_CR

	RET

