;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;31.10.2020	w5277c@gmail.com			Начало
;30.01.2021	w5277c@gmail.com			Тест, отработано корректно.
;03.07.2021	w5277c@gmail.com			Z->Y, отработано корректно.
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	TIMER_C_ENABLE							= 0	;0-1
	.INCLUDE "./devices/atmega328.inc"
	.SET	TS_MODE									= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	OPT_MODE									= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA										= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/beeper.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/wait_1s.inc"
	.include	"./core/uptime_copy.inc"
	.include	"./core/meminfo_copy.inc"
	.include	"./core/io/out_bytes.inc"
	.include	"./core/io/out_romstr.inc"
	.include	"./core/io/out_cr.inc"
	.include "./io/port_mode_out.inc"
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"
	.include "./core/io/out_num16.inc"
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
	LDI_Z DRV_BEEPER_INIT
	LDI ACCUM,PD7
	LDI FLAGS,TID_BEEPER
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_BEEPER_TASK
	LDI_Z BEEPER_TASK__INIT
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_LED_TASK
	LDI_Z LED_TASK__INIT
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_UPTIME_TASK
	LDI_Z UPTIME_TASK__INIT
	MCALL C5_CREATE
	;Инициализация задачи тестирования
	LDI PID,PID_FREEMEM_TASK
	LDI_Z FREEMEM_TASK__INIT
	MCALL C5_CREATE

	LDI PID,PID_TIMER_TASK
	LDI_Z TIMER_TASK__INIT
	LDI TEMP_H,BYTE3(5000/2)
	LDI TEMP_L,BYTE2(5000/2)
	LDI TEMP,BYTE1(5000/2)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	BEEPER_TASK__DATA:
	.db N64T,C5,D5,E5,F5,G5,A5,H5,E,0x00

BEEPER_TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
BEEPER_TASK__INFINITE_LOOP:
	LDI TEMP,PID_BEEPER_DRV
	LDI_Z BEEPER_TASK__DATA|0x8000								;Вызываем процедуру драйвера (данные мелодии берем из ROM)
	MCALL C5_EXEC

	LDI TEMP,0x05														;Пауза в 5 сеунд
	MCALL C5_WAIT_1S
	RJMP BEEPER_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
LED_TASK__INIT:
	LDI ACCUM,PD4
	MCALL PORT_MODE_OUT
	MCALL C5_READY
;--------------------------------------------------------
LED_TASK__INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS

	LDI ACCUM,PD4
	MCALL PORT_INVERT

	RJMP LED_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
	UPTIME_TASK__STR1:
	.db "UPTIME[2ms]:0x",0x00,0x00

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
	MCALL C5_DISPATCHER_LOCK
	PUSH_Y
	LDI_Y UPTIME_TASK__STR1|0x8000
	MCALL C5_OUT_STR
	POP_Y
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR
	MCALL C5_DISPATCHER_UNLOCK

	LDI TEMP,0x01
	MCALL C5_WAIT_1S
	RJMP UPTIME_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
	FREEMEM_TASK__STR1:
	.db "MEM. TOTAL:",0x00
	FREEMEM_TASK__STR2:
	.db " FREE:",0x00,0x00

FREEMEM_TASK__INIT:
	LDI ACCUM,0x04
	MCALL C5_RAM_REALLOC
	MOV YH,ZH
	MOV YL,ZL
	MCALL C5_READY
;--------------------------------------------------------
FREEMEM_TASK__INFINITE_LOOP:
	MCALL C5_MEMINFO_COPY
	MCALL C5_DISPATCHER_LOCK
	PUSH_Y
	LDI_Y FREEMEM_TASK__STR1|0x8000
	MCALL C5_OUT_STR
	POP_Y
	LDD TEMP_H,Y+0x02
	LDD TEMP_L,Y+0x03
	MCALL C5_OUT_NUM16
	PUSH_Y
	LDI_Y FREEMEM_TASK__STR2|0x8000
	MCALL C5_OUT_STR
	POP_Y
	LDD TEMP_H,Y+0x00
	LDD TEMP_L,Y+0x01
	MCALL C5_OUT_NUM16
	MCALL C5_OUT_CR
	MCALL C5_DISPATCHER_UNLOCK

	LDI TEMP,0x01
	MCALL C5_WAIT_1S
	RJMP FREEMEM_TASK__INFINITE_LOOP

;--------------------------------------------------------;Задача
TIMER_TASK__INIT:
	LDI ACCUM,PB2
	MCALL PORT_MODE_OUT
	MCALL C5_READY
;--------------------------------------------------------
TIMER_TASK__INFINITE_LOOP:
	MCALL PORT_INVERT

	LDI TEMP,0x02
	MCALL C5_WAIT_1S

	;Выполняем SUSPEND с переходом на начало для следующей итерации или просто делаем RET
	MCALL C5_SUSPEND
	RJMP TIMER_TASK__INFINITE_LOOP
