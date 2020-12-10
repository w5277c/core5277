;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./devices/atmega328.inc"
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_25NS
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/beeper.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/wait_1s.inc"
	.include	"./core/wait_1s.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BEEPER_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_BEEPER								= 0
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

	;Инициализация BEEPER
	LDI PID,PID_BEEPER_DRV
	LDI ZH,high(DRV_BEEPER_INIT)
	LDI ZL,low(DRV_BEEPER_INIT)
	LDI ACCUM,PD7
	LDI FLAGS,TID_BEEPER
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	TASK__DATA:
	.db N16T,C3,D3,D3d,C3,D3,D3d,D3d,F3,G3,D3d,F3,G3,F3,G3,A3,F3,G3,A3,G3d,A3d,C4,G3d,A3d,C4,C4,P,C4,C4,C4,C4,E
	;.db N4,D0,D1,D2,D3,D4,E
	;.db N8,C4,C4d,D4,D4d,E4,E4d,F4,F4d,G4,G4d,A4,A4d,H4,H4d,C5,C5d,D5,D5d,E5,E5d,F5,F5d,G5,G5d,A5,A5d,H5,H5d,E

TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI YH,high(TASK__DATA)|0x80									;Вызываем процедуру драйвера (данные мелодии берем из ROM)
	LDI YL,low(TASK__DATA)
	LDI TEMP,PID_BEEPER_DRV
	MCALL C5_EXEC

	LDI TEMP,0x05														;Пауза в 5 сеунд
	MCALL C5_WAIT_1S
	RJMP TASK__INFINITE_LOOP

