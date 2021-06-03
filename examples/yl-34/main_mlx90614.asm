;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;13.09.2020  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega16.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_25NS
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PB0	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/i2c_h.inc"
	.INCLUDE "./core/drivers/mlx90614.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_sdnf.inc"
	.include	"./core/log/log_romstr.inc"
	.include	"./core/log/log_cr.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_MLX90614_DRV						= 1|(1<<C5_PROCID_OPT_DRV)
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

	;Инициализация 1WIRE
	LDI PID,PID_I2C_DRV
	LDI ZH,high(DRV_I2C_H_INIT)
	LDI ZL,low(DRV_I2C_H_INIT)
	LDI XH,0x00
	LDI XL,DRV_I2C_FREQ_100KHZ
	LDI YH,PB4
	MCALL C5_CREATE

	;Инициализация MLX90614
	LDI PID,PID_MLX90614_DRV
	LDI ZH,high(DRV_MLX90614_INIT)
	LDI ZL,low(DRV_MLX90614_INIT)
	LDI ACCUM,PID_I2C_DRV
	MCALL C5_CREATE

	;Инициализация задачи тестирования MLX90614
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
	LDI TEMP_L,high(500/2)
	LDI TEMP,low(500/2)
	MCALL C5_WAIT_2MS											;Ждем 500мс

	LDI TEMP,PID_MLX90614_DRV
	MCALL C5_EXEC
	BRNE TASK__INFINITE_LOOP

	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	MCALL C5_LOG_SDNF
	MCALL C5_LOG_CR
	RJMP TASK__INFINITE_LOOP

