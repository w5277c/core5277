;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;23.10.2024	konstantin@5277.ru	Начало
;-----------------------------------------------------------------------------------------------------------------------

	.EQU	FW_VERSION										= 1	;0.1
	.EQU	CORE_FREQ										= 8
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 0
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega8.inc"

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_NO		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	;---

;	.SET    BUFFER_SIZE									= 0x20

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C_DRV										= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0

;---CORE-SETTINGS----------------------------------------
	.SET	C5_DRIVERS_QNT									= 1
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
	.SET	TIMERS16											= 0	;0-...

	.SET	LOGGING_PORT								= MISO		;PA0-PC7
	.SET	LOGGING_LEVEL								= LOGGING_LVL_DBG

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/i2c_h.inc"
	;---
	;Дополнительно (по мере использования)
	.include "./core/wait_2ms.inc"

	.include "./mem/ram_fill.inc"
;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	LDI PID,PID_I2C_DRV
	LDI_Z DRV_I2C_H_INIT
	LDI ACCUM,0xff
	LDI XL,DRV_I2C_FREQ_50KHZ										;TODO в _i2c.inc указан для 16MHz
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK_INIT
	MCALL C5_CREATE

	MJMP C5_START

_TASK_DATA:
.db	"PING"

;--------------------------------------------------------;
TASK_INIT:
	LDI ACCUM,0x08
	MCALL C5_RAM_REALLOC

	MCALL C5_READY

_TASK_LOOP:
	MOVW XL,YL
	LDI TEMP,0x00
	LDI LOOP_CNTR,0x08
	MCALL RAM_FILL

	LDI TEMP,PID_I2C_DRV
	LDI_Z _TASK_DATA|0x8000
	LDI ACCUM,'W'
	LDI TEMP_H,0x04
	LDI TEMP_L,0x08
	MOVW XL,YL
	MCALL C5_EXEC

	MCALL C5_OUT_BYTE
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	LDI TEMP,0x08
	MOVW ZL,YL
	MCALL C5_OUT_BYTES

	LDI TEMP_H,0x00
	LDI TEMP_L,0x09
	LDI TEMP,0xc4
	MCALL C5_WAIT_2MS													;5s
	RJMP _TASK_LOOP

	RET
