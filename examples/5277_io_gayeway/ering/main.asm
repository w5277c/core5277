;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;09.03.2024	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ								= 16
	.EQU	TIMER_C_ENABLE							= 1	;0-1
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
	.INCLUDE "/core/drivers/ering.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include "/core/io/out_byte.inc"
	.include "/core/io/out_char.inc"
	.include "/core/io/out_num32_hex.inc"
	.include "/core/io/out_uptime.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_ERING_DRV							= 0|(1<<C5_PROCID_OPT_DRV)
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
	LDI PID,PID_ERING_DRV
	LDI_Z DRV_ERING_INIT
	LDI TEMP_EH,0x04
	LDI TEMP_EL,0x0a
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI_X 0x0001
	MCALL C5_CREATE

	;Инициализация задачи тестирования(отработка по таймеру раз в 10 миллисекунд)
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	LDI TEMP_H,(10/2)>>0x10 & 0xff
	LDI TEMP_L,(10/2)>>0x08 & 0xff
	LDI TEMP,  (10/2)>>0x00 & 0xff
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------

	LDI TEMP,PID_ERING_DRV
	LDI FLAGS,DRV_OP_GET
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _ERROR_READ

TASK_LOOP:
	LDI TEMP,PID_ERING_DRV
	LDI FLAGS,DRV_OP_SET
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _ERROR_SET

	LDI TEMP,PID_ERING_DRV
	LDI FLAGS,DRV_OP_EVENT
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_SENSELESS
	BREQ _TASK_CONTINUE
	CPI TEMP,DRV_RESULT_TRY_LATER
	BREQ _TASK_CONTINUE
	CPI TEMP,DRV_RESULT_OK
	BRNE _ERROR_EVENT

	MCALL C5_OUT_UPTIME
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_NUM32_HEX
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	LDI TEMP,PID_ERING_DRV
	LDI FLAGS,DRV_OP_STATUS
	MCALL C5_EXEC
	MOV TEMP,TEMP_L
	MCALL C5_OUT_BYTE
	MCALL C5_OUT_CR

_TASK_CONTINUE:
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	ADC TEMP_EL,C0x00
	ADC TEMP_EH,C0x00

	;Засыпаем до следующего события от таймера
	MCALL C5_SUSPEND
	RJMP TASK_LOOP

_ERROR_READ:
	C5_OUT_ROMSTR TXT_READ
	MCALL C5_OUT_CR
	RET
_ERROR_SET:
	C5_OUT_ROMSTR TXT_SET
	MCALL C5_OUT_CR
	RET
_ERROR_EVENT:
	C5_OUT_ROMSTR TXT_EVENT
	MCALL C5_OUT_CR
	RET


TXT_READ:
	.db "read error\r\n",0x00
TXT_SET:
	.db "set error\r\n",0x00
TXT_EVENT:
	.db "event error\r\n",0x00
TXT_FLUSH:
	.db "flush",0x00

