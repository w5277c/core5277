;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2020  w5277c@gmail.com        Первая версия шаблона
;-----------------------------------------------------------------------------------------------------------------------
	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS									= 4	;0-4
	.SET	BUFFER_SIZE								= 0x10;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	;.INCLUDE "./inc/drivers/i2c_h.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;.EQU	PID_I2C_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
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

	;Инициализация задачи
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
	;TODO
	RET

