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
	.INCLUDE "./core/drivers/1wire_s.inc"
	.INCLUDE "./core/drivers/ds1990a.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include "./dt/timestamp_to_dt.inc"
	.include "./conv/dt_to_str.inc"
	.include "./core/ram/ram_realloc.inc"
	.include "./core/io/out_str.inc"
	.include "./core/wait_2ms.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_1WIRE_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_DS1990A_DRV						= 1|(1<<C5_PROCID_OPT_DRV)
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

	;Инициализация 1WIRE
	LDI PID,PID_1WIRE_DRV
	LDI_Z DRV_1WIRE_INIT
	LDI ACCUM,PC1
	MCALL C5_CREATE

	;Инициализация DS1990A
	LDI PID,PID_DS1990A_DRV
	LDI_Z DRV_DS1990A_INIT
	LDI ACCUM,PID_1WIRE_DRV
	MCALL C5_CREATE

	;Инициализация задачи тестирования DS1990A
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(200/2)>>0x10 & 0xff
	LDI TEMP_L,(200/2)>>0x08 & 0xff
	LDI TEMP,  (200/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	TASK__LOGSTR_ERROR:
	.db   "error",0x0d,0x0a,0x00

TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_DS1990A_DRV
	MCALL C5_EXEC
	CPI TEMP,DRV_DS1990A_BUFFER_CRC
	BREQ TASK__ERROR
	CPI TEMP,DRV_RESULT_OK
	BRNE TASK__DONE

	LDI TEMP,0x08
	MOVW YL,XL
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR
	RJMP TASK__DONE
TASK__ERROR:
	C5_OUT_ROMSTR TASK__LOGSTR_ERROR
TASK__DONE:
	MCALL C5_SUSPEND
	RJMP TASK__INFINITE_LOOP

