;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.10.2021	konstantin@5277.ru	Начало
;-----------------------------------------------------------------------------------------------------------------------

	.EQU	FW_VERSION										= 1	;0.1
	.EQU	CORE_FREQ										= 16
	.EQU	TIMER_C_ENABLE									= 1	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 0
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega88.inc"
	.SET	_C5_STACK_SIZE									= 0x80	;Стек ядра

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.EQU	LCD_RS											= PB4		;WH1602 RS
	.EQU	LCD_E												= PB5		;WH1602 E
	;---

;	.SET    BUFFER_SIZE									= 0x20

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_LCD_DRV										= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_IR_DRV										= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы 8бит таймеров
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	C5_DRIVERS_QNT									= 2
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
	.SET	TIMERS16											= 0	;0-...

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/hd44780.inc"
	.INCLUDE "./core/drivers/ir.inc"
	;---
	;Дополнительно (по мере использования)
	.include "./core/wait_2ms.inc"
	.include "./mem/ram_fill.inc"
	.include "./mem/rom_read_bytes.inc"

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	LDI PID,PID_LCD_DRV
	LDI_Z DRV_HD44780_INIT
	LDI ACCUM,PC0
	LDI TEMP_H,LCD_RS
	LDI TEMP_L,LCD_E
	LDI FLAGS,(1<<DRV_HD44780_FL_RL)|(1<<DRV_HD44780_FL_N)|(0<<DRV_HD44780_FL_DL)|(0<<DRV_HD44780_FL_I2C)
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK_INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;
TASK_INIT:
	LDI ACCUM,0x20
	MCALL C5_RAM_REALLOC

	MCALL C5_READY

	MOVW XL,YL
	LDI TEMP,' '
	LDI LOOP_CNTR,0x20
	MCALL RAM_FILL

	LDI_Z C5_NAME*0x02
	LDI LOOP_CNTR,0x0f
	MCALL ROM_READ_BYTES

	LDI TEMP,PID_LCD_DRV
	LDI FLAGS,DRV_VIDEO_OP_INIT
	MCALL C5_EXEC

_TASK__LOOP:
	LDI TEMP,PID_LCD_DRV
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI FLAGS,DRV_VIDEO_OP_SET_XY
	MCALL C5_EXEC
	
	LDI TEMP,PID_LCD_DRV
	MOVW ZL,YL
	LDI LOOP_CNTR,0x10
	LDI FLAGS,DRV_VIDEO_OP_SET_TEXT
	MCALL C5_EXEC

	LDI TEMP,PID_LCD_DRV
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI FLAGS,DRV_VIDEO_OP_SET_XY
	MCALL C5_EXEC

	LDI TEMP,PID_LCD_DRV
	MOVW ZL,YL
	ADIW ZL,0x10
	LDI LOOP_CNTR,0x10
	LDI FLAGS,DRV_VIDEO_OP_SET_TEXT
	MCALL C5_EXEC

	RET

