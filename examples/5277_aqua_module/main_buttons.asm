;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.09.2020  w5277c@gmail.com			Начало
;28.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	.SET	AVRA										= 0	;0-1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/buttons.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_word.inc"
;	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV						= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS								= 0
;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация BUTTONS
	LDI PID,PID_BUTTONS_DRV
	LDI ZH,high(DRV_BUTTONS_INIT)
	LDI ZL,low(DRV_BUTTONS_INIT)
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI TEMP,PID_BUTTONS_DRV
	LDI ACCUM,PC1|(0x80)												;Включаем подтяжку к 5в

	LDI FLAGS,DRV_BUTTONS_OP_ADD_CUSTOM
	LDI TEMP_H,(1<<_DRV_BUTTON_FL_LED_MODE)
;
;	LDI FLAGS,DRV_BUTTONS_OP_ADD_CUSTOM
;	LDI TEMP_H,(1<<_DRV_BUTTON_FL_FIX)
;
;	LDI FLAGS,DRV_BUTTONS_OP_ADD_CUSTOM
;	LDI TEMP_H,(1<<_DRV_BUTTON_FL_LED_MODE)|(1<<_DRV_BUTTON_FL_SENSOR)
	MCALL C5_EXEC

	MCALL C5_READY
;--------------------------------------------------------
	LDI LOOP_CNTR,0x00
TASK__INFINITE_LOOP:
	
	
	LDI TEMP,PID_BUTTONS_DRV
	LDI FLAGS,DRV_BUTTONS_OP_WAIT
	MCALL C5_EXEC
	MCALL C5_OUT_WORD
	MCALL C5_OUT_CR

	CPI TEMP_L,0x10
	BRNE TASK__INFINITE_LOOP
	LDI TEMP,0x01
	EOR LOOP_CNTR,TEMP
	BREQ PC+0x03
	LDI FLAGS,DRV_BUTTONS_OP_LED_ON
	RJMP PC+0x02
	LDI FLAGS,DRV_BUTTONS_OP_LED_OFF
	LDI ACCUM,PC1
	LDI TEMP,PID_BUTTONS_DRV
	MCALL C5_EXEC

	RJMP TASK__INFINITE_LOOP

