;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ								= 16
	.EQU	TIMER_C_ENABLE							= 1	;0-1
	.SET	AVRA										= 0	;0-1
	.SET	REPORT_INCLUDES						= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_TIME
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= SCK	;PA0-PC7
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.INCLUDE "/core/drivers/frtc.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include "./dt/timestamp_to_dt.inc"
	.include "./dt/dt_to_timestamp.inc"
	.include "./conv/dt_to_str.inc"
	.include "./core/ram/ram_realloc.inc"
	.include "./core/io/out_str.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_RTC_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;---
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0|(1<<C5_PROCID_OPT_TIMER)
	;---
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера RTC
	LDI PID,PID_RTC_DRV
	LDI_Z DRV_FRTC_INIT
	MCALL C5_CREATE
	
	;Инициализация задачи тестирования(отработка по таймеру раз в секунду)
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(1000/2)>>0x10 & 0xff
	LDI TEMP_L,(1000/2)>>0x08 & 0xff
	LDI TEMP,  (1000/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

	.EQU	TIMESTAMP		= 1641562770							;13:39:30 07.01.22

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x12														;xx:xx:xx xx.xx.xx0
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
	;Задаем текущее время
	LDI TEMP,PID_RTC_DRV
	LDI TEMP_EH,(TIMESTAMP>>0x18 &0xff)
	LDI TEMP_EL,(TIMESTAMP>>0x10 &0xff)
	LDI TEMP_H, (TIMESTAMP>>0x08 &0xff)
	LDI TEMP_L, (TIMESTAMP>>0x00 &0xff)
	LDI ACCUM,DRV_RTC_SET_TIMESTAMP
	MCALL C5_EXEC

TASK_LOOP:
	;Получаем текущее время
	LDI TEMP,PID_RTC_DRV
	LDI ACCUM,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	;Ковертируем
	MCALL TIMESTAMP_TO_DT
	MOVW ZL,YL
	MCALL DT_TO_STR
	ST Z,C0x00
	;Выводим
	MOVW ZL,YL
	MCALL C5_OUT_STR

	;MCALL DT_TO_TIMESTAMP
	;TOTO вывести TIMESTAMP

	MCALL C5_OUT_CR



	;Засыпаем до следующего события от таймера
	MCALL C5_SUSPEND
	RJMP TASK_LOOP
