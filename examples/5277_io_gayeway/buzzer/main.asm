;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2020  w5277c@gmail.com        Первая версия шаблона
;11.11.2020  w5277c@gmail.com        Актуализация
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	FW_VERSION										= 1	;0-255
	.SET	CORE_FREQ										= 16	;2-20Mhz
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"
	.SET	_C5_STACK_SIZE									= 0x80	;Стек ядра

	;---CONSTANTS--------------------------------------------
	.EQU	BUS_LED_PORT									= PD5		;Порт индикации активности шины
	.SET	ACT_LED_PORT									= PB2		;Порт индикации активности
	.EQU	BUS_DR_PORT										= PD4		;UART порт для управления направлением прием/передача шины
	.EQU	CH1_PORT											= PC4		;Основной порт, канал 1
	.EQU	CH1_CAP_PORT									= PC1		;Конденсатор 100nf для канала 1
	.EQU	CH1_RES_PORT									= PC3		;Подтяжка на 4K/1M для канала 1
	.EQU	CH2_PORT											= PC5		;Основной порт, канал 2
	.EQU	CH2_CAP_PORT									= PD6		;Конденсатор 100nf для канала 2
	.EQU	CH2_RES_PORT									= PC2		;Подтяжка на 4K/1M для канала 2
	.EQU	CH3_PORT											= PD2		;Основной порт, канал 3
	.EQU	CH3_CAP_PORT									= PB0		;Конденсатор 100nf для канала 3
	.EQU	CH3_RES_PORT									= PB1		;Подтяжка на 4K/1M для канала 3
	.EQU	EXT1_PORT										= PB5		;Порт расширения, N1
	.EQU	EXT2_PORT										= PB3		;Порт расширения, N2
	.EQU	EXT3_PORT										= PB4		;Порт расширения, N3
	.EQU	EXTR_PORT										= PD3		;Порт расширения, правый
	.EQU	EXTB_PORT										= PC0		;Порт расширения, нижний

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUZZER_DRV									= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_ASYNC										= 0
	.EQU	TID_MOD											= 1
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_EVENT
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 2		;0-...
	.SET	BUFFER_SIZE										= 0		;Размер общего буфера
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= SCK		;PA0-PF7
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/buzzer.inc"
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
	LDI ACCUM,BUS_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	LDI ACCUM,ACT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO


	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера
	LDI PID,PID_BUZZER_DRV
	LDI_Z DRV_BUZZER_INIT
	LDI TEMP_EH,TID_ASYNC
	LDI TEMP_EL,0xff													;0xff/TID_MOD
	LDI TEMP_H,0x05													;100ms
	LDI TEMP_L,EXT2_PORT												;BUZZER PORT
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	LDI ACCUM,BUS_LED_PORT
	MCALL PORT_SET_HI

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK_DATA:
	.db "task done",0x0d,0x0a,0x00
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	LDI TEMP,PID_BUZZER_DRV
	LDI FLAGS,DRV_OP_SYNC_START
	LDI TEMP_H,0b10101000											;pi-pi-pi pi-pi
	LDI TEMP_L,0b00010100
	LDI LOOP_CNTR,0x0e
	MCALL C5_EXEC


	C5_OUT_ROMSTR TASK_DATA
	RET
