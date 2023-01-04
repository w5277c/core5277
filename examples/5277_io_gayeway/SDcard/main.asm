;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.11.2022	konstantin@5277.ru		Первая версия
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	FW_VERSION										= 1	;0-255
	.SET	CORE_FREQ										= 16	;2-20Mhz
	.SET	REPORT_INCLUDES								= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega328.inc"

	;---CONSTANTS--------------------------------------------
	.EQU	INIT_LED_PORT									= PB2		;Порт индикации пройденной инициализации
	.EQU	ACT_LED_PORT									= PD5		;Порт индикации активности

	;Идентификаторы драйверов(0-7|0-15)
;	.EQU	PID_SPI_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_SD_DRV								= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_EVENT
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0		;0-...
	.SET	BUFFER_SIZE										= 0x200	;Размер общего (буфер драйвера SD карты)
	.SET	LOGGING_PORT									= SDA		;STDOUT
	.SET	C5_IN_PORT										= SCL	;STDIN
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/spi_ms.inc"

	.include "./core/drivers/sd/sd_get_ocr.inc"
	.include "./core/drivers/sd/sd_log_ocr.inc"
	.include "./core/drivers/sd/sd_get_csd.inc"
	.include "./core/drivers/sd/sd_log_csd.inc"
	.include "./core/drivers/sd/sd_get_cid.inc"
	.include "./core/drivers/sd/sd_log_cid.inc"
	.INCLUDE "./core/drivers/sd_spi.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./io/port_mode_out.inc"
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"

	;---


;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:

	;Инициализация портов
	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	LDI ACCUM,ACT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера
	LDI PID,PID_SD_DRV
	LDI_Z DRV_SD_INIT
	LDI_X C5_BUFFER
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_SET_HI

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_INIT
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE TASK__DONE

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_OCR
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE TASK__DONE
	LDI_X C5_BUFFER
	MCALL DRV_SD_LOG_OCR

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CSD
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE TASK__DONE
	LDI_X C5_BUFFER
	MCALL DRV_SD_LOG_CSD

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CID
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE TASK__DONE
	LDI_X C5_BUFFER
	MCALL DRV_SD_LOG_CID

TASK__DONE:
	RET

;	LDI TEMP,0x01														;Пауза в 1 сеунду
;	MCALL C5_WAIT_1S
;	RJMP TASK__INFINITE_LOOP
