;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.10.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	FW_VERSION										= 1	;0-255
	.SET	CORE_FREQ										= 16	;2-20Mhz
	.EQU	TIMER_C_ENABLE									= 1	;0-1
	.SET	AVRA												= 0	;0-1
	.SET	REPORT_INCLUDES								= 0	;0-1
	;---подключаем библиотеку устройства---
	.INCLUDE "./devices/atmega168.inc"

	;---CONSTANTS--------------------------------------------
	.EQU	INIT_LED_PORT									= PD5		;Порт индикации пройденной инициализации
	.EQU	ACT_LED_PORT									= PB2		;Порт индикации активности
	.EQU	I2C2_SDA											= PD2
	.EQU	I2C2_SCL											= SCL

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_I2C1_DRV									= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_I2C2_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_C_DRV									= 2|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_ADC_V_DRV									= 3|(1<<C5_PROCID_OPT_DRV)
		;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0|(1<<C5_PROCID_OPT_TIMER)
	;Идентификаторы таймеров
	;---

;---CORE-SETTINGS----------------------------------------
	.SET	TS_MODE											= TS_MODE_TIME
	.SET	TIMERS_SPEED									= TIMERS_SPEED_50US	;25/50us
	.SET	TIMERS											= 0		;0-...
	.SET	BUFFER_SIZE										= 0		;Размер общего буфера
.if FLASH_SIZE >= 0x4000
	.SET	LOGGING_PORT									= SCK		;PA0-PF7
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
	.include "./core/wait_1s.inc"
	.include "./core/io/out_uptime.inc"
	.include "./core/io/out_num16.inc"
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

	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера I2C 1
	LDI PID,PID_I2C1_DRV
	LDI_Z DRV_I2C_H_INIT
	LDI XH,0x00
	LDI XL,DRV_I2C_FREQ_400KHZ
	LDI ACCUM,ACT_LED_PORT
	MCALL C5_CREATE
	;Инициализация драйвера ADC(LTC2451)
	LDI PID,PID_ADC_C_DRV
	LDI_Z DRV_LTC2451_INIT
	LDI ACCUM,PID_I2C1_DRV
	LDI FLAGS,DRV_LTC2451_FREQ_60Hz
	MCALL C5_CREATE

	;Инициализация драйвера I2C 2
	LDI PID,PID_I2C2_DRV
	LDI_Z DRV_I2C_MS_INIT
	LDI TEMP_H,I2C2_SDA
	LDI TEMP_L,I2C2_SCL
	LDI TEMP_EH,TID_TIMER_C;TID_I2C2
	LDI TEMP_EL,TIMER_C_FREQ_20KHz
	MCALL C5_CREATE
	;Инициализация драйвера ADC(LTC2451)
	LDI PID,PID_ADC_V_DRV
	LDI_Z DRV_LTC2451_INIT
	LDI ACCUM,PID_I2C2_DRV
	MCALL C5_CREATE

	;Инициализация задачи(опрос в 50Гц)
	LDI PID,PID_TASK
	LDI_Z TASK_INIT
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x0a
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK_INIT:
	
	MCALL C5_READY
	
	MCALL C5_OUT_CR
;--------------------------------------------------------
TASK_INFINITE_LOOP:

	MCALL C5_OUT_UPTIME
	LDI TEMP,','
	MCALL C5_OUT_CHAR

	LDI TEMP,PID_ADC_C_DRV
	MCALL C5_EXEC	
	CPI TEMP,DRV_RESULT_OK
	BRNE PC+0x01+_MCALL_SIZE
	MCALL C5_OUT_NUM16
	LDI TEMP,','
	MCALL C5_OUT_CHAR
	
	LDI TEMP,PID_ADC_V_DRV
	MCALL C5_EXEC	
	CPI TEMP,DRV_RESULT_OK
	BRNE PC+0x01+_MCALL_SIZE
	MCALL C5_OUT_NUM16

	MCALL C5_OUT_CR
	MCALL C5_SUSPEND
	RJMP TASK_INFINITE_LOOP
