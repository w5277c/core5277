;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;23.08.2023	konstantin@5277.ru			Начало
;24.08.2023	konstantin@5277.ru			Не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Необходим проект https://github.com/w5277c/core5277

	.EQU	CORE_FREQ										= 8
	.EQU	TIMER_C_ENABLE									= 0	;0-1
	.SET	AVRA												= 1	;0-1
	.SET	REPORT_INCLUDES								= 0x01
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/attiny45.inc"

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	TS_MODE											= TS_MODE_NO		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.EQU	LED_PORT											= PB2
	.EQU	SENSOR_PORT										= PB4
	.EQU	BUZZER_PORT										= PB3

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_ADC_DRV										= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_BUZZER_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_BUZZER1										= 0
	.EQU	TID_BUZZER2										= 1

;---CORE-SETTINGS----------------------------------------
	.SET	C5_DRIVERS_QNT									= 2
	.SET	C5_TASKS_QNT									= 1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 2	;0-...

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/adc.inc"
	.INCLUDE "./core/drivers/buzzer.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./io/port_mode_out.inc"
	.include "./io/port_mode_in.inc"
	.include "./io/port_set_lo.inc"
	.include "./core/wait_2ms.inc"
	;---

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	;Инициализация ядра
	MCALL C5_INIT

	LDI ACCUM,LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	LDI ACCUM,SENSOR_PORT
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	LDI ACCUM,BUZZER_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	LDI PID,PID_ADC_DRV
	LDI_Z DRV_ADC_INIT
	LDI ACCUM,0xFF
	MCALL C5_CREATE

	LDI PID,PID_BUZZER_DRV
	LDI_Z DRV_BUZZER_INIT
	LDI TEMP_EH,TID_BUZZER1
	LDI TEMP_EL,TID_BUZZER2
	LDI TEMP_H,0x05
	LDI TEMP_L,BUZZER_PORT
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI_Z TASK_INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;
TASK_INIT:
	MCALL C5_READY
TASK_INFINITE_LOOP:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x09
	LDI TEMP,0xc4
	MCALL C5_WAIT_2MS													;5 sec

	LDI TEMP,PID_ADC_DRV
	LDI ACCUM,ADC2
	LDI TEMP_EH,ADC_VREF_AVCC
	LDI TEMP_EL,0x10
	LDI TEMP_H,0x01
	LDI TEMP_L,0xff
	LDI_X 0x0000
	MCALL C5_EXEC
	ANDI TEMP_H,0x7f
	BREQ TASK_INFINITE_LOOP

	LDI TEMP,PID_BUZZER_DRV
	LDI FLAGS,DRV_OP_ASYNC_START
	LDI TEMP_H,0b10101000
	LDI TEMP_L,0b00101010
	LDI LOOP_CNTR,0x10
	MCALL C5_EXEC

	RJMP TASK_INFINITE_LOOP
