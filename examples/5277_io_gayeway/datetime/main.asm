;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ								= 16
	.EQU	TIMER_C_ENABLE							= 1	;0-1
	.SET	AVRA										= 0	;0-1
	.SET	REPORT_INCLUDES						= 0	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"

;---CORE-SETTINGS----------------------------------------
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= SCK	;PA0-PC7
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	;---
	;Блок задач
	;---
	;Дополнительно
	.include "./dt/timestamp_to_dt.inc"
	.include "./conv/dt_to_str.inc"
	.include "./core/ram/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;---
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;---
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

	.EQU	TIMESTAMP		= 1601404252;

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x20
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI TEMP_EH,(TIMESTAMP>>0x18 &0xff)
	LDI TEMP_EL,(TIMESTAMP>>0x10 &0xff)
	LDI TEMP_H, (TIMESTAMP>>0x08 &0xff)
	LDI TEMP_L, (TIMESTAMP>>0x00 &0xff)
	MCALL TIMESTAMP_TO_DT
	MOVW ZL,YL
	MCALL DT_TO_STR
	ST Z,C0x00
	RET
