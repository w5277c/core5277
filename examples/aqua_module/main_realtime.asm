;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;31.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_25NS
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	.INCLUDE "./inc/drivers/beeper.inc"
	;Блок задач
	;---
	;Дополнительно
	.include "./inc/io/port_mode_out.inc"
	.include "./inc/io/port_set_hi.inc"
	.include "./inc/io/port_set_lo.inc"
	.include	"./inc/core/wait_1s.inc"
	.include	"./inc/io/log_bytes.inc"
	.include	"./inc/io/log_romstr.inc"
	.include	"./inc/io/logstr_new_line.inc"
	.include	"./inc/core/uptime_copy.inc"
	.include	"./inc/core/meminfo_copy.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BEEPER_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_BEEPER_TASK						= 0
	.EQU	PID_LED_TASK							= 1
	.EQU	PID_UPTIME_TASK						= 2
	.EQU	PID_FREEMEM_TASK						= 3
	.EQU	PID_TIMER_TASK							= 4|(1<<C5_PROCID_OPT_TIMER)
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
	LDI PID,PID_BEEPER_TASK
	LDI ZH,high(BEEPER_TASK__INIT)
	LDI ZL,low(BEEPER_TASK__INIT)
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_LED_TASK
	LDI ZH,high(LED_TASK__INIT)
	LDI ZL,low(LED_TASK__INIT)
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_UPTIME_TASK
	LDI ZH,high(UPTIME_TASK__INIT)
	LDI ZL,low(UPTIME_TASK__INIT)
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_FREEMEM_TASK
	LDI ZH,high(FREEMEM_TASK__INIT)
	LDI ZL,low(FREEMEM_TASK__INIT)
	MCALL C5_CREATE

	LDI PID,PID_TIMER_TASK
	LDI ZH,high(TIMER_TASK__INIT)
	LDI ZL,low(TIMER_TASK__INIT)
	LDI TEMP_H,BYTE3(5000/2)
	LDI TEMP_L,BYTE2(5000/2)
	LDI TEMP,BYTE1(5000/2)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	BEEPER_TASK__DATA:
	.db N16T,C3,D3,D3d,C3,D3,D3d,D3d,F3,G3,D3d,F3,G3,F3,G3,A3,F3,G3,A3,G3d,A3d,C4,G3d,A3d,C4,C4,P,C4,C4,C4,C4,E
	;.db N4,D0,D1,D2,D3,D4,E
	;.db N8,C4,C4d,D4,D4d,E4,E4d,F4,F4d,G4,G4d,A4,A4d,H4,H4d,C5,C5d,D5,D5d,E5,E5d,F5,F5d,G5,G5d,A5,A5d,H5,H5d,E

BEEPER_TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
BEEPER_TASK__INFINITE_LOOP:
	LDI YH,high(BEEPER_TASK__DATA)|0x80							;Вызываем процедуру драйвера (данные мелодии берем из ROM)
	LDI YL,low(BEEPER_TASK__DATA)
	LDI TEMP,PID_BEEPER_DRV
	MCALL C5_EXEC

	LDI TEMP,0x01														;Пауза в 5 сеунд
	MCALL C5_WAIT_1S
	RJMP BEEPER_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
LED_TASK__INIT:
	LDI ACCUM,PD4
	MCALL C5_PORT_MODE_OUT
	MCALL C5_READY
;--------------------------------------------------------
LED_TASK__INFINITE_LOOP:
	LDI ACCUM,PD4
	MCALL C5_PORT_SET_LO

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS

	LDI ACCUM,PD4
	MCALL C5_PORT_SET_HI

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS

	RJMP LED_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
UPTIME_TASK__INIT:
	LDI ACCUM,0x05
	MCALL C5_RAM_REALLOC
	MOV YH,ZH
	MOV YL,ZL
	MCALL C5_READY
;--------------------------------------------------------
UPTIME_TASK__INFINITE_LOOP:
	MCALL C5_UPTIME_COPY
	LDI TEMP,0x05
	MCALL C5_LOG_BYTES
	C5_LOG_ROMSTR LOGSTR_NEW_LINE

	LDI TEMP,0x01
	MCALL C5_WAIT_1S

	RJMP UPTIME_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
FREEMEM_TASK__INIT:
	LDI ACCUM,0x05
	MCALL C5_RAM_REALLOC
	MCALL C5_READY
;--------------------------------------------------------
FREEMEM_TASK__INFINITE_LOOP:
	MOV YH,ZH
	MOV YL,ZL
	MCALL C5_MEMINFO_COPY
	LDI TEMP,0x02
	MCALL C5_LOG_BYTES
	LDI TEMP,'/'
	MCALL C5_LOG_CHAR
	ADIW YL,0x02
	LDI TEMP,0x02
	MCALL C5_LOG_BYTES
	C5_LOG_ROMSTR LOGSTR_NEW_LINE

	LDI TEMP,0x01
	MCALL C5_WAIT_1S

	RJMP FREEMEM_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
TIMER_TASK__INIT:
	LDI ACCUM,PB2
	MCALL C5_PORT_MODE_OUT
	MCALL C5_READY
;--------------------------------------------------------
TIMER_TASK__INFINITE_LOOP:
	MCALL C5_PORT_INVERT

	;LDI TEMP,0x02
	;MCALL C5_WAIT_1S


	;Выполняем SUSPEND с переходом на начало для следующей итерации или просто делаем RET
	MCALL C5_SUSPEND
	RJMP TIMER_TASK__INFINITE_LOOP

