;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;26.07.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	;.INCLUDE "./inc/devices/attiny45.inc"
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	.INCLUDE "./inc/drivers/beeper.inc"
	.INCLUDE "./inc/drivers/pcint_h.inc"
	.INCLUDE "./inc/drivers/am2301.inc"
	.INCLUDE "./inc/drivers/1wire_s.inc"
	.INCLUDE "./inc/drivers/ds18b20.inc"
	;Блок задач
	.INCLUDE "beeper.asm"
	.INCLUDE "ds18b20_test.asm"
	.INCLUDE "uptime.asm"
	.INCLUDE "freemem.asm"
	.INCLUDE "leds.asm"
	;---
	;Дополнительно
	;---
;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_BEEPER_DRV							= 0|(1<<CORE5277_PROCID_OPT_DRV)
	.EQU	PID_PCINT_DRV							= 1|(1<<CORE5277_PROCID_OPT_DRV)
	.EQU	PID_AM2301_DRV							= 2|(1<<CORE5277_PROCID_OPT_DRV)
	.EQU	PID_1WIRE_DRV							= 3|(1<<CORE5277_PROCID_OPT_DRV)
	.EQU	PID_DS18B20_DRV						= 4|(1<<CORE5277_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_AM2301								= 0
	.EQU	PID_UPTIME								= 1;|(1<<CORE5277_PROCID_OPT_NOSUSP)
	.EQU	PID_FREEMEM								= 2
	.EQU	PID_LEDS									= 3
	.EQU	PID_BEEPER								= 4
	.EQU	PID_DS18B20								= 5

;--------------------------------------------------------;Выполняемый код при старте контроллера
MAIN:
	CLI
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	;Инициализация ядра
	MCALL CORE5277_INIT

	;Инициализация BEEPER
	LDI PID,PID_BEEPER_DRV
	LDI ZH,high(DRV_BEEPER_INIT)
	LDI ZL,low(DRV_BEEPER_INIT)
	LDI ACCUM,PD7
	MCALL CORE5277_CREATE

	;Инициализация PCINT
	LDI PID,PID_PCINT_DRV
	LDI ZH,high(DRV_HPCINT_INIT)
	LDI ZL,low(DRV_HPCINT_INIT)
;	MCALL CORE5277_CREATE

	;Инициализация AM2301
	LDI PID,PID_AM2301_DRV
	LDI ZH,high(DRV_AM2301_INIT)
	LDI ZL,low(DRV_AM2301_INIT)
	LDI TEMP_H,PID_PCINT_DRV
	LDI TEMP_L,PC1
;	MCALL CORE5277_CREATE

	;Инициализация 1WIRE
	LDI PID,PID_1WIRE_DRV
	LDI ZH,high(DRV_1WIRE_INIT)
	LDI ZL,low(DRV_1WIRE_INIT)
	LDI ACCUM,PC1
;	MCALL CORE5277_CREATE

	;Инициализация DS18B20
	LDI PID,PID_DS18B20_DRV
	LDI ZH,high(DRV_DS18B20_INIT)
	LDI ZL,low(DRV_DS18B20_INIT)
	LDI ACCUM,PID_1WIRE_DRV
;	MCALL CORE5277_CREATE

	;Инициализация задачи тестирования
;	LDI PID,PID_AM2301
;	LDI ZH,high(TSK_TEST_INIT)
;	LDI ZL,low(TSK_TEST_INIT)
;	MCALL CORE5277_CREATE

	;Инициализация задачи тестирования DS18B20
	LDI PID,PID_DS18B20
	LDI ZH,high(DS18B20_TEST_INIT)
	LDI ZL,low(DS18B20_TEST_INIT)
	;MCALL CORE5277_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_UPTIME
	LDI ZH,high(TSK_UPTIME_INIT)
	LDI ZL,low(TSK_UPTIME_INIT)
	MCALL CORE5277_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_FREEMEM
	LDI ZH,high(TSK_FREEMEM_INIT)
	LDI ZL,low(TSK_FREEMEM_INIT)
	MCALL CORE5277_CREATE

	;Инициализация задачи мигания светодиодов
	LDI PID,PID_LEDS
	LDI ZH,high(TSK_LEDS_INIT)
	LDI ZL,low(TSK_LEDS_INIT)
	MCALL CORE5277_CREATE

	;Инициализация задачи проигрывания мелодии
	LDI PID,PID_BEEPER
	LDI ZH,high(TSK_BEEPER_INIT)
	LDI ZL,low(TSK_BEEPER_INIT)
	MCALL CORE5277_CREATE

	MJMP CORE5277_START
