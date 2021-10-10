;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.10.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

	.EQU	FW_VERSION										= 1	;0.1 (так-же должна быть записана в EEPROM)
	.EQU	CORE_FREQ										= 16
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.SET	AVRA												= 1	;0-1
	.SET	REPORT_INCLUDES								= 0
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	ACT_LED_PORT									= PB2		;Порт индикации активности
	;---

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C_DRV										= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_WH_DRV										= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров

;---CORE-SETTINGS----------------------------------------
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
	.SET	LOGGING_PORT									= PD2	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/i2c_h.inc"
	.INCLUDE "./core/drivers/wh.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация UART для датчика
	LDI PID,PID_I2C_DRV
	LDI_Z DRV_I2C_H_INIT
	LDI XL,DRV_I2C_FREQ_100KHZ
	LDI ACCUM,ACT_LED_PORT
	MCALL C5_CREATE

	LDI PID,PID_WH_DRV
	LDI TEMP_H,PID_I2C_DRV
	LDI_Z DRV_WH1602_PCF8574_INIT
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_MASTER
	LDI_Z TASK_INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;
TASK_INIT:
	MCALL C5_READY

	;TODO
	RET
