;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.07.2023	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ										= 16
	.EQU	TIMER_C_ENABLE									= 1	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 0x01
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"
	.SET	_C5_STACK_SIZE									= 0x80	;Стек ядра

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.EQU	BUS_DR_PORT										= PC4		;UART порт для управления направлением прием/передача шины
	.EQU	LCD_RS											= PD4		;WH1602 RS
	.EQU	LCD_RW											= PD3		;WH1602 RW
	.EQU	LCD_E												= PD2		;WH1602 E
	.EQU	LCD_K												= PB1		;WH1602 K

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_LCD_DRV										= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров

;---CORE-SETTINGS----------------------------------------
	.SET	C5_DRIVERS_QNT									= 1
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0	;0-...
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= MISO	;PA0-PC7
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
	.SET	MASTER_LOGGING									= 1
	.SET	LOGGING_RAMUSAGE								= 0
	.SET	LOGGING_STACKUSAGE							= 0
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/hd44780.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./core/wait_2ms.inc"
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	;RW 0-WRITE
	LDI ACCUM,LCD_RW
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Light
	LDI ACCUM,LCD_K
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Инициализация ШИМ
	;ATMEGA 48-328 ONLY!
	;Fast PWM 8 bit, clear OC1A on Compare Match, set OC1A at BOTTOM, x8 Prescaller (~2KHz)
	LDI TEMP,(1<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(1<<WGM10)
	STS TCCR1A,TEMP
	LDI TEMP,(0<<ICNC1)|(0<<ICES1)|(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10)
	STS TCCR1B,TEMP
	STS TCNT1H,C0x00
	STS TCNT1L,C0x00
	;OCR1AH/L=(0-255)
	LDI TEMP,(0<<ICIE1)|(0<<OCIE1B)|(0<<OCIE1A)|(0<<TOIE1)
	STS TIMSK1,TEMP
	LDI TEMP,(0<<ICF1)|(0<<OCF1B)|(1<<OCF1A)|(0<<TOV1)
	STS TIFR1,TEMP
	LDI TEMP,high(256)
	STS OCR1AH,C0x00
	;OCR1AL=64(25%)
	LDI TEMP,128
	STS OCR1AL,TEMP
	;Power ON
	_C5_POWER_ON PRTIM1

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
