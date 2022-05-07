;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.09.2020	w5277c@gmail.com			Начало
;28.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;07.01.2022	w5277c@gmail.com			Актуализация кода
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ								= 16	;2-20Mhz
	.EQU	TIMER_C_ENABLE							= 0	;0-1
	.SET	AVRA										= 0	;0-1
	.SET	REPORT_INCLUDES						= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega328.inc"

	.SET	TS_MODE									= TS_MODE_TIME
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	TIMERS									= 0	;0-4
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.INCLUDE "./core/drivers/i2c_h.inc"
	.INCLUDE "./core/drivers/mlx90614.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_sdnf.inc"
	.include	"./core/io/out_romstr.inc"
	.include	"./core/io/out_cr.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_MLX90614_DRV						= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0|(1<<C5_PROCID_OPT_TIMER)
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация I2C
	LDI PID,PID_I2C_DRV
	LDI_Z DRV_I2C_H_INIT
	LDI XH,0x00
	LDI XL,DRV_I2C_FREQ_100KHZ
	LDI ACCUM,PB4
	MCALL C5_CREATE

	;Инициализация MLX90614
	LDI PID,PID_MLX90614_DRV
	LDI_Z DRV_MLX90614_INIT
	LDI ACCUM,PID_I2C_DRV
	MCALL C5_CREATE

	;Инициализация задачи тестирования MLX90614
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(500/2)>>0x10 & 0xff
	LDI TEMP_L,(500/2)>>0x08 & 0xff
	LDI TEMP,  (500/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_MLX90614_DRV
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE TASK__DONE

	MCALL C5_OUT_SDNF
	MCALL C5_OUT_CR
TASK__DONE:
	MCALL C5_SUSPEND
	RJMP TASK__INFINITE_LOOP