;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.09.2020  w5277c@gmail.com        Первая версия шаблона
;11.11.2020  w5277c@gmail.com        Актуализация
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	FW_VERSION										= 1	;0-255
	.SET	CORE_FREQ										= 16	;2-20Mhz
	.EQU	TIMER_C_ENABLE									= 1	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 1	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega328.inc"

	;---CONSTANTS--------------------------------------------
	.EQU	INIT_LED_PORT									= PD5		;Порт индикации пройденной инициализации
	.EQU	ACT_LED_PORT									= PB2		;Порт индикации активности
	.EQU	I2C2_SDA											= SDA;PC0
	.EQU	I2C2_SDA_RES									= PC3
	.EQU	I2C2_SCL											= SCL;PC1
	.EQU	I2C2_SCL_RES									= PC2

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C1_DRV									= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_I2C2_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_C_DRV									= 2|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_V_DRV									= 3|(1<<C5_PROCID_OPT_DRV)
		;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	.EQU	TID_I2C2											= 0
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_EVENT
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 1		;0-...
	.SET	BUFFER_SIZE										= 0		;Размер общего буфера
.if FLASH_SIZE >= 0x4000
;	.SET	LOGGING_PORT									= SCK		;PA0-PF7
.endif

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;---
	;Блок драйверов
	.INCLUDE "./core/drivers/i2c_h.inc"
	.INCLUDE "./core/drivers/i2c_ms.inc"
	.INCLUDE "./core/drivers/ltc2451.inc"
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
	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	LDI ACCUM,ACT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	LDI ACCUM,I2C2_SDA_RES
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	LDI ACCUM,I2C2_SCL_RES
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI

	LDI ACCUM,PD2
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	
	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера I2C 1
	LDI PID,PID_I2C1_DRV
	LDI_Z DRV_I2C_H_INIT
	LDI XL,DRV_I2C_FREQ_400KHZ
	LDI ACCUM,ACT_LED_PORT
;	MCALL C5_CREATE
	;Инициализация драйвера ADC(LTC2451)
	LDI PID,PID_ADC_C_DRV
	LDI_Z DRV_LTC2451_INIT
	LDI ACCUM,PID_I2C1_DRV
;	MCALL C5_CREATE

	;Инициализация драйвера I2C 2
	LDI PID,PID_I2C2_DRV
	LDI_Z DRV_I2C_MS_INIT
	LDI TEMP_H,I2C2_SDA
	LDI TEMP_L,I2C2_SCL
	LDI TEMP_EH,TID_TIMER_C;TID_I2C2
	LDI TEMP_EL,DRV_I2C_MS_TC_FREQ_DEFAULT
	MCALL C5_CREATE
	;Инициализация драйвера ADC(LTC2451)
	LDI PID,PID_ADC_V_DRV
	LDI_Z DRV_LTC2451_INIT
	LDI ACCUM,PID_I2C2_DRV
;	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI ZH,high(TASK_INIT)
	LDI ZL,low(TASK_INIT)
	MCALL C5_CREATE

	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_SET_HI

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK_INIT:
	
	LDI ACCUM,0x04
	MCALL C5_RAM_REALLOC
	
	LDI TEMP,0x40
	STD Y+0x00,TEMP
	
	LDI TEMP,0x40
	STD Y+0x01,TEMP
	
	LDI TEMP,0x03
	STD Y+0x02,TEMP
	
	LDI TEMP,0x80
	STD Y+0x03,TEMP

	MCALL C5_READY
;--------------------------------------------------------
TASK_INFINITE_LOOP:

	LDI TEMP,PID_I2C2_DRV
	MOVW ZL,YL
	MOVW XL,YL
	LDI TEMP_H,0x04
	LDI TEMP_L,0x00
	LDI ACCUM,0x10
	MCALL C5_EXEC	
	
	RET


	;TODO


;	LDI TEMP,0x01														;Пауза в 1 сеунду
;	MCALL C5_WAIT_1S
	;RJMP TASK_INFINITE_LOOP
