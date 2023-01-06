;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.01.2023	konstantin@5277.ru		Первая версия
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
	.EQU	PID_DISK_DRV									= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_RTC_DRV										= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_FS_DRV										= 2|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_EVENT
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0		;0-...
	.SET	BUFFER_SIZE										= 0x200	;Размер общего (буфер драйвера SD карты)
	.SET	LOGGING_PORT									= SDA		;STDOUT
	.SET	C5_IN_PORT										= SCL	;STDIN
	.SET	LOGGING_LEVEL									= LOGGING_LVL_INF
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/spi_ms.inc"

	.INCLUDE "./core/drivers/sd/sd_read_block.inc"
	.INCLUDE "./core/drivers/sd/sd_write_block.inc"
	.INCLUDE "./core/drivers/sd_spi.inc"

	.INCLUDE "./core/drivers/frtc.inc"
	.INCLUDE "./core/drivers/fs/fs_format.inc"
	.INCLUDE "./core/drivers/fs_c5.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.INCLUDE "./io/port_mode_out.inc"
	.INCLUDE "./io/port_set_hi.inc"
	.INCLUDE "./io/port_set_lo.inc"
	.INCLUDE "./mem/rom_read_bytes.inc"

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
	LDI PID,PID_DISK_DRV
	LDI_Z DRV_SD_INIT
	LDI_X C5_BUFFER
	MCALL C5_CREATE

	;Инициализация драйвера
	LDI PID,PID_RTC_DRV
	LDI_Z DRV_FRTC_INIT
	MCALL C5_CREATE

	;Инициализация драйвера
	LDI PID,PID_FS_DRV
	LDI_Z DRV_FS_INIT
	LDI_X C5_BUFFER
	LDI TEMP_H,PID_DISK_DRV
	LDI TEMP_L,PID_RTC_DRV
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_SET_HI

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__DISK_LABEL:
	.db "test",0x00
;--------------------------------------------------------
TASK__INIT:
	LDI ACCUM,0x05
	MCALL C5_RAM_REALLOC
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI_Z TASK__DISK_LABEL
	MCALL ROM_READ_BYTES

	LDI TEMP,PID_FS_DRV
	LDI FLAGS,DRV_FS_OP_FORMAT
	MOVW XL,YL
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE TASK__DONE

TASK__DONE:
	RET
