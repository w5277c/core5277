;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.07.2023	konstantin@5277.ru			Начало
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
	.EQU	IR_BUFFER_SIZE									= 0x08	;Максимальный размер буфера IR
	.EQU	TS_MODE											= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
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

;---CORE-SETTINGS----------------------------------------
	.SET	BUFFER_SIZE										= 0x0000;Размер общего буфера
	.SET	C5_DRIVERS_QNT									= 1
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0		;0-...
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= CH1_PORT	;PA0-PC7
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
	.SET	MASTER_LOGGING									= 1
	.SET	LOGGING_RAMUSAGE								= 0
	.SET	LOGGING_STACKUSAGE							= 0
.endif


;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/ir.inc"
		;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./io/port_mode_out.inc"
	.include	"./io/port_set_lo.inc"
	.include	"./io/port_set_hi.inc"
	.include "./core/ram/ram_realloc.inc"
	.include	"./core/io/out_cr.inc"
	.include	"./core/io/out_byte.inc"
	.include	"./core/io/out_char.inc"
	.include	"./core/io/out_bytes.inc"
	.include "./core/wait_2ms.inc"

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_IR_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация светодиода и других портов(которые не инициализируются драйверами)
	LDI ACCUM,BUS_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	LDI ACCUM,ACT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация IR
	LDI PID,PID_IR_DRV
	LDI_Z DRV_IR_INIT
	LDI TEMP_H,CH3_PORT
	LDI TEMP_L,0xff
	LDI TEMP_EH,C5_IR_INT0
	LDI TEMP_EL,ACT_LED_PORT
	LDI FLAGS,TIMER_C_FREQ_38KHz
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
_TASK_MSG_REPEATE:
	.db "REPEATE",0x00

TASK__INIT:
	LDI ACCUM,IR_BUFFER_SIZE
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	LDI TEMP,PID_IR_DRV
	MOVW XL,YL
	LDI TEMP_EH,0x00
	LDI TEMP_EL,IR_BUFFER_SIZE
	LDI TEMP_H,0x00
	LDI TEMP_L,20/2
	MCALL C5_EXEC
	CPI TEMP_H,DRV_RESULT_OK
	BRNE TASK

	CPI TEMP_L,0x00
	BRNE _TASK_DATA
	LDI_Z _TASK_MSG_REPEATE|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR
	RJMP TASK

_TASK_DATA:
	MOVW ZL,YL
	MOV TEMP,TEMP_L
	MCALL C5_OUT_BYTE
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_L
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR
	RJMP TASK

