;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.10.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

	.EQU	FW_VERSION										= 1	;0.1 (так-же должна быть записана в EEPROM)
	.EQU	CORE_FREQ										= 16
	.EQU	TIMER_C_ENABLE									= 1	;0-1
	.SET	AVRA												= 0	;0-1
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
	.EQU	PID_LCD_DRV										= 1|(1<<C5_PROCID_OPT_DRV)
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
	;.INCLUDE "./core/drivers/i2c_ms.inc"
	.INCLUDE "./core/drivers/hd44780.inc"
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
	LDI XH,0x00
	LDI XL,DRV_I2C_FREQ_100KHZ
	LDI ACCUM,ACT_LED_PORT
	MCALL C5_CREATE

;	LDI PID,PID_I2C_DRV
;	LDI_Z DRV_I2C_MS_INIT
;	LDI TEMP_H,SDA
;	LDI TEMP_L,SCL
;	LDI TEMP_EH,TID_TIMER_C
;	LDI TEMP_EL,TIMER_C_FREQ_10KHz
;	MCALL C5_CREATE

	LDI PID,PID_LCD_DRV
	LDI_Z DRV_HD44780_INIT
	LDI ACCUM,PID_I2C_DRV
	LDI FLAGS,(1<<DRV_HD44780_FL_RL)|(1<<DRV_HD44780_FL_N)|(1<<DRV_HD44780_FL_I2C)
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK_INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;
TASK_TEXT:
	.db "^ ",0x00,0x00

TASK_INIT:
	MCALL C5_READY


	LDI TEMP,PID_LCD_DRV
	LDI FLAGS,DRV_VIDEO_OP_INIT
	MCALL C5_EXEC

	LDI TEMP,PID_LCD_DRV
	LDI_Z OUTSTR_CORE+0x01|0x8000
	LDI LOOP_CNTR,10
	LDI FLAGS,DRV_VIDEO_OP_SET_TEXT
	MCALL C5_EXEC

	LDI TEMP,PID_LCD_DRV
	LDI TEMP_H,0
	LDI TEMP_L,1
	LDI FLAGS,DRV_VIDEO_OP_SET_XY
	MCALL C5_EXEC

	LDI TEMP,PID_LCD_DRV
	LDI_Z OUTSTR_CORE+0x06|0x8000
	LDI LOOP_CNTR,16
	LDI FLAGS,DRV_VIDEO_OP_SET_TEXT
	MCALL C5_EXEC

TASK_INFINITE_LOOP:
	LDI_Z TASK_TEXT*0x02
	LDI LOOP_CNTR,0x02
TASK_LOOP:
	LDI TEMP_H,0
	LDI TEMP_L,0
	LDI TEMP,500/2
	MCALL C5_WAIT_2MS
	
	LDI TEMP,PID_LCD_DRV
	LDI TEMP_H,15
	LDI TEMP_L,0
	LDI FLAGS,DRV_VIDEO_OP_SET_XY
	MCALL C5_EXEC

	LPM ACCUM,Z+
	LDI TEMP,PID_LCD_DRV
	LDI FLAGS,DRV_VIDEO_OP_SET_SYMBOL
	MCALL C5_EXEC
	DEC LOOP_CNTR
	BRNE TASK_LOOP
	RJMP TASK_INFINITE_LOOP

	RET
