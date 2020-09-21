;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;14.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./inc/io/log_bytes.inc"
	.include	"./inc/io/logstr_new_line.inc"
	.include	"./inc/mem/eeprom_write_byte.inc"
	.include	"./inc/mem/eeprom_read_byte.inc"
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
	MCALL CORE5277_INIT

	;Инициализация задачи тестирования EEPROM
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL CORE5277_CREATE

	MJMP CORE5277_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL CORE5277_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x56
	MCALL CORE5277_EEPROM_WRITE_BYTE
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI TEMP,0x67
	MCALL CORE5277_EEPROM_WRITE_BYTE


	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL CORE5277_EEPROM_READ_BYTE
	PUSH TEMP

	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	MCALL CORE5277_EEPROM_READ_BYTE
	MOV TEMP_L,TEMP
	POP TEMP_H

	MCALL CORE5277_LOG_WORD
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE

	RET

