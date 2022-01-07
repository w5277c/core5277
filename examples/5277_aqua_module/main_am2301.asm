;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.10.2020	w5277c@gmail.com			Начало
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
	.INCLUDE "./core/drivers/pcint_h.inc"
	.INCLUDE "./core/drivers/am2301.inc"
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/io/out_char.inc"
	.include	"./core/io/out_sdnf.inc"
	.include	"./core/io/out_romstr.inc"
	.include	"./core/io/out_cr.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_PCINT_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_AM2301_DRV							= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0|(1<<C5_PROCID_OPT_TIMER)
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация PCINT
	LDI PID,PID_PCINT_DRV
	LDI_Z DRV_PCINT_H_INIT
	MCALL C5_CREATE

	;Инициализация AM2301
	LDI PID,PID_AM2301_DRV
	LDI_Z DRV_AM2301_INIT
	LDI TEMP_H,PID_PCINT_DRV
	LDI TEMP_L,PC1
	MCALL C5_CREATE

	;Инициализация задачи тестирования AM2301
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(5000/2)>>0x10 & 0xff
	LDI TEMP_L,(5000/2)>>0x08 & 0xff
	LDI TEMP,  (5000/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	TASK__LOGSTR_ERROR:
	.db   "Error:",0x00,0x00

TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_AM2301_DRV
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE TASK__ERROR

	LDI TEMP,'T'
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_SDNF
	LDI TEMP,' '
	MCALL C5_OUT_CHAR
	LDI TEMP,'H'
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	MCALL C5_OUT_SDNF
	MCALL C5_OUT_CR
	RJMP TASK__DONE

TASK__ERROR:
	C5_OUT_ROMSTR TASK__LOGSTR_ERROR
	MCALL C5_OUT_BYTE
	MCALL C5_OUT_CR

TASK__DONE:
	MCALL C5_SUSPEND
	RJMP TASK__INFINITE_LOOP
