;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com        Начало
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
	.include	"./inc/io/log_numx8.inc"
	.include	"./inc/io/log_romstr.inc"
	.include	"./inc/io/logstr_new_line.inc"
	.include	"./inc/math/div10.inc"
	.include	"./inc/math/div100.inc"
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


	TASK__LOGSTR1:
	.db   "/10=",0x00
	TASK__LOGSTR2:
	.db   "[0x",0x00
	TASK__LOGSTR3:
	.db   "]",0x0a,0x0d,0x00
;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL CORE5277_READY
;--------------------------------------------------------
;	LDI LOOP_CNTR,0x00
;TASK__LOOP:
;	MOV TEMP,LOOP_CNTR
;	MCALL CORE5277_LOG_NUMx8
;	CORE5277_LOG_ROMSTR TASK__LOGSTR1
;	MCALL CORE5277_DIV10
;	MOV TEMP,TEMP_L
;	MCALL CORE5277_LOG_NUMx8
;	CORE5277_LOG_ROMSTR TASK__LOGSTR2
;	MCALL CORE5277_LOG_BYTE
;	CORE5277_LOG_ROMSTR TASK__LOGSTR3
;	INC LOOP_CNTR
;	CPI LOOP_CNTR,0x00
;	BRNE TASK__LOOP

	LDI TEMP_H,high(59942);=600
	LDI TEMP_L,low(59942);=600
	MCALL CORE5277_DIV100
	MCALL CORE5277_LOG_WORD

	RET

