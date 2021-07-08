;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.06.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.INCLUDE "./devices/atmega328.inc"
	.INCLUDE "./core/drivers/grx/font4x6.inc"
	.SET	AVRA												= 1		;0-1
	.SET	TS_MODE											= TS_MODE_TIME
	.SET	TIMERS											= 1		;0-4
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US
	.SET	BUFFER_SIZE										= 0x00	;Размер общего буфера
;	.SET	LOGGING_PORT									= PC0		;PA0-PC7

	.EQU	INIT_LED_PORT									= PD5		;Порт индикации пройденной инициализации
	.EQU	ACT_LED_PORT									= PB2		;Порт индикации активности
	.EQU	BUS_DR_PORT										= PD4		;UART порт для управления направлением прием/передача шины
	.EQU	EXT1_PORT										= PB5		;Порт расширения, N1
	.EQU	EXT2_PORT										= PB3		;Порт расширения, N2
	.EQU	EXT3_PORT										= PB4		;Порт расширения, N3
	.EQU	EXTR_PORT										= PD3		;Порт расширения, правый
	.EQU	CH1_PORT											= PC4		;Основной порт, канал 1
	.EQU	CH2_PORT											= PC5		;Основной порт, канал 2
	.EQU	CH3_PORT											= PD2		;Основной порт, канал 3

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/pcd8544.inc"						;LCD драйвер
	.include	"./core/drivers/grx.inc"							;Окружение для вывода текста/графики
	.include	"./core/drivers/grx/font4x6.inc"					;Загрузка шрифта 4x6
	;---
	;Блок задач
	;---
	;Дополнительно
;	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_PCD8544_DRV						= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_GRX_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_PCD8544								= 0


;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	CLI
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация PCD8544
	LDI PID,PID_PCD8544_DRV
	LDI ZH,high(DRV_PCD8544_INIT)
	LDI ZL,low(DRV_PCD8544_INIT)
	LDI TEMP_EH,EXTR_PORT
	LDI TEMP_EL,CH1_PORT
	LDI TEMP_H,CH2_PORT
	LDI TEMP_L,CH3_PORT
	LDI ACCUM,TID_PCD8544
	MCALL C5_CREATE

	;Инициализация GRX
	LDI PID,PID_GRX_DRV
	LDI ZH,high(DRV_GRX_INIT)
	LDI ZL,low(DRV_GRX_INIT)
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;
TASK_HELLO_TXT:
	.db	"Hello blinking led!",0x00
;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	;Устанавливаю чернобелый текстовой режим
	LDI PID,PID_GRX_DRV
	LDI FLAGS,DRV_GRX_OP_SET_MODE
	LDI TEMP,DRV_GRX_MODE_TXT1B
	MCALL C5_EXEC
	;Указываю используемый шрифт
	LDI PID,PID_GRX_DRV
	LDI FLAGS,DRV_GRX_OP_SET_FONT
	LDI TEMP,0x46
	LDI_Y FONT_DATA|0x8000
	MCALL C5_EXEC
	;Получаю размер в символах
	LDI PID,PID_GRX_DRV
	LDI FLAGS,DRV_GRX_OP_GET_SIZE
	MCALL C5_EXEC
	;Середина по вертикали и слева по горизонтали
	LDI_X 0x0000
	LSR YH
	ROR YL
	;Устанавливаю позицию печати
	LDI PID,PID_GRX_DRV
	LDI FLAGS,DRV_GRX_OP_SET_XY
	MCALL C5_EXEC
	;Печатаю текст
	LDI PID,PID_GRX_DRV
	LDI FLAGS,DRV_GRX_OP_SET_TEXT
	LDI_Y TASK_HELLO_TXT|0x8000
	LDI TEMP,0x00
	MCALL C5_EXEC

	RET
