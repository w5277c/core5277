;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.05.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.SET	CORE_FREQ										= 0x08	;Max: 8-ATMega16, 10-ATMega382

	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega88.inc"
	.SET	REPORT_INCLUDES								= 0x01

	;Важные, но не обязательные параметры ядра
	.SET	AVRA												= 0	;0-1
	.SET	REALTIME											= 0	;0-1
	.SET	C5_DRIVERS_QNT									= 2
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS											= 1	;0-4

	;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов (дополнительные включения должны быть перед основным)
	.include	"./core/drivers/buttons.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	;---

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	DIG1_PORT										= PB6		;Порт управления анодом цифры 1
	.EQU	DIG2_PORT										= PB4		;Порт управления анодом цифры 2
	.EQU	DIG3_PORT										= PB5		;Порт управления анодом цифры 3
	.EQU	DIG4_PORT										= PB1		;Порт управления анодом цифры 4
	.EQU	SEGA_PORT										= PB2		;Порт управления катодом сегмента A
	.EQU	SEGB_PORT										= PC0		;Порт управления катодом сегмента B
	.EQU	SEGC_PORT										= PD7		;Порт управления катодом сегмента C
	.EQU	SEGD_PORT										= PD5		;Порт управления катодом сегмента D
	.EQU	SEGE_PORT										= PB7		;Порт управления катодом сегмента E
	.EQU	SEGF_PORT										= PB3		;Порт управления катодом сегмента F
	.EQU	SEGG_PORT										= PB0		;Порт управления катодом сегмента Q
	.EQU	SEGP_PORT										= PD6		;Порт управления катодом сегмента Q
	.EQU	BATTERY_PORT									= PC5		;Порт для анализа уровня заряда батареи
	.EQU	BUTTON_PORT										= PC1		;Порт для кнопки
	.EQU	INT_PORT											= PD2		;Порт ввода

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BUTTONS_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_BUTTONS										= 0
	;---

	;Дополнительно (по мере использования)
	.include "./io/port_set_hi.inc"
	.include "./io/port_set_lo.inc"
	.include "./io/port_invert.inc"
	.include "./core/wait_1s.inc"
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	CLI
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	;Инициализация портов
	LDI TEMP,(1<<(DIG1_PORT&0x0f))|(1<<(DIG2_PORT&0x0f))|(1<<(DIG3_PORT&0x0f))|(1<<(DIG4_PORT&0x0f))|(1<<(SEGA_PORT&0x0f))|(1<<(SEGE_PORT&0x0f))|(1<<(SEGF_PORT&0x0f))|(1<<(SEGG_PORT&0x0f))
	OUT DDRB,TEMP
	LDI TEMP,(0<<(DIG1_PORT&0x0f))|(0<<(DIG2_PORT&0x0f))|(0<<(DIG3_PORT&0x0f))|(0<<(DIG4_PORT&0x0f))|(0<<(SEGA_PORT&0x0f))|(0<<(SEGE_PORT&0x0f))|(0<<(SEGF_PORT&0x0f))|(0<<(SEGG_PORT&0x0f))
	OUT PORTB,TEMP
	LDI TEMP,(1<<(SEGB_PORT&0x0f))|(0<<(BATTERY_PORT&0x0f))|(0<<(BUTTON_PORT&0x0f))
	OUT DDRC,TEMP
	LDI TEMP,(0<<(SEGB_PORT&0x0f))|(0<<(BATTERY_PORT&0x0f))|(1<<(BUTTON_PORT&0x0f))
	OUT PORTC,TEMP
	LDI TEMP,(1<<(SEGC_PORT&0x0f))|(1<<(SEGP_PORT&0x0f))|(1<<(SEGD_PORT&0x0f))|(0<<(INT_PORT))
	OUT DDRD,TEMP
	LDI TEMP,(0<<(SEGC_PORT&0x0f))|(0<<(SEGP_PORT&0x0f))|(0<<(SEGD_PORT&0x0f))|(1<<(INT_PORT))
	OUT PORTD,TEMP

	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация BUTTONS
	LDI PID,PID_BUTTONS_DRV
	LDI ZH,high(DRV_BUTTONS_INIT)
	LDI ZL,low(DRV_BUTTONS_INIT)
	LDI ACCUM,TID_BUTTONS
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI TEMP,PID_BUTTONS_DRV
	LDI ACCUM,BUTTON_PORT
	LDI FLAGS,DRV_BUTTONS_OP_ADD
	MCALL C5_EXEC

	LDI TEMP,0x01
	LDI ACCUM,DIG1_PORT
	MCALL PORT_SET_HI

;	LDI ACCUM,DIG2_PORT
;	MCALL PORT_SET_HI
;	LDI ACCUM,DIG3_PORT
;	MCALL PORT_SET_HI
;	LDI ACCUM,DIG4_PORT
;	MCALL PORT_SET_HI

	MCALL C5_READY
;--------------------------------------------------------
TASK:

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGA_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGB_PORT
	MCALL PORT_INVERT
	
	MCALL C5_WAIT_1S
	LDI ACCUM,SEGC_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGD_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGE_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGF_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGG_PORT
	MCALL PORT_INVERT

	MCALL C5_WAIT_1S
	LDI ACCUM,SEGP_PORT
	MCALL PORT_INVERT

;	LDI TEMP,PID_BUTTONS_DRV
;	LDI FLAGS,DRV_BUTTONS_OP_WAIT
;	MCALL C5_EXEC
;	MCALL C5_LOG_WORD
	RJMP TASK

