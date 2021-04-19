;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com,
;по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;16.04.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;---DESCRIPTION------------------------------------------
;core5277 for ATMega328 (32Kb ROM, 1Kb EEPROM, 2Kb SRAM) at 16MHz (LITTLE ENDIAN)
;avrdude -p m328 -c avrispmkII -U flash:w:main.hex
;Ext crystal 8.0-..., 1K CK/14 CK + 4.1ms, CKDIV8 disabled, BOD 4.3V, SPI enabled:
;avrdude -p m328p -c avrispmkII -U lfuse:w:0xdf:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

	.SET	CORE_FREQ								= 16
	.INCLUDE "./devices/atmega328.inc"
	.SET	C5_DRIVERS_QNT							= 0x03

;---CONSTANTS--------------------------------------------
	;---MAIN-CONSTANTS---
	.EQU	INIT_LED_PORT									= PD5		;Порт индикации пройденной инициализации
	.SET	ACT_LED_PORT									= PB2		;Порт индикации активности
	.EQU	HC05_TX_PORT									= PB0		;UART порт для передачи данных на BL(HC-05)
	.EQU	HC05_RX_PORT									= PB1		;UART порт для приема данных с BL(HC-05)

	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_HC05_UART_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	.EQU	PID_HC05_DRV									= 1|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK											= 0
	;Идентификаторы таймеров
	;---
;---CORE-SETTINGS----------------------------------------
	.SET	AVRA												= 1	;0-1
	.SET	REALTIME											= 0	;0-1
	.SET	TIMERS_SPEED									= TIMERS_SPEED_25NS	;25/50ns
	.SET	TIMERS											= 1	;0-...
	.SET	TIMER_C_ENABLE									= 1	;0-1
	.SET	LOGGING_PORT									= PC5
	.SET	LOGGING_LEVEL									= LOGGING_LVL_PNC
	.SET	INPUT_PORT										= PD2

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/uart_f.inc"
	.INCLUDE "./core/drivers/hc05.inc"
	;---
	;Блок задач
	;---
	;Дополнительно (по мере использования)
	.include "./core/ram/ram_realloc.inc"
	.include "./core/ui/menu.inc"
	.include	"./core/ui/hex32_input.inc"
	.include "./core/ui/text_input.inc"
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
	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	LDI ACCUM,ACT_LED_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI

	LDI ACCUM,MISO
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO


	;Инициализация ядра
	MCALL C5_INIT

	;Инициализация драйвера HC05 UART(программный)
	LDI PID,PID_HC05_UART_DRV
	LDI_Z DRV_UART_F_INIT
	LDI TEMP_H,HC05_RX_PORT
	LDI TEMP_L,HC05_TX_PORT
	LDI TEMP_EH,0xff
	LDI TEMP_EL,DRV_UART_F_BAUDRATE_38400
	MCALL C5_CREATE

	;Инициализация драйвера HC-05
	LDI PID,PID_HC05_DRV
	LDI ZH,high(DRV_HC05_INIT)
	LDI ZL,low(DRV_HC05_INIT)
	LDI FLAGS,(1<<DRV_HC05_FLAG_LOGGING)
	LDI TEMP,PID_HC05_UART_DRV
	MCALL C5_CREATE

	;Инициализация задачи
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	LDI ACCUM,INIT_LED_PORT
	MCALL PORT_SET_HI

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	LDI ACCUM,0x80
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
TASK:
	MOV YH,ZH
	MOV YL,ZL
	LDI ACCUM,0x32
	MCALL C5_TEXT_INPUT
	CPI TEMP,0x00
	BREQ TASK

	ADD YL,TEMP
	LDI TEMP_H,0x00
	ADC YH,TEMP_H
	LDI TEMP_H,0x0d
	ST Y+,TEMP_H
	LDI TEMP_H,0x0a
	ST Y+,TEMP_H
	SUBI TEMP,(0x100-0x02)

	MOV XH,ZH
	MOV XL,ZL
	MOV TEMP_H,TEMP
	LDI TEMP,PID_HC05_DRV
	LDI ACCUM,DRV_HC05_OP_RAW
	MCALL C5_EXEC

	CPI TEMP_H,DRV_HC05_RESULT_OK
	BRNE TASK
	CPI TEMP_L,0x00
	BREQ TASK
	MOV TEMP,TEMP_L
	MCALL C5_LOG_STRN
	MCALL C5_LOG_CR
	RJMP TASK
