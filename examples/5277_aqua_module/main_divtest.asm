;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020  w5277c@gmail.com			Начало
;28.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	TIMER_C_ENABLE							= 0	;0-1
	.INCLUDE "./devices/atmega328.inc"

	.SET	TS_MODE									= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	OPT_MODE									= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA										= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_numx8.inc"
	.include	"./core/log/log_numx16.inc"
	.include	"./core/log/log_romstr.inc"
	.include	"./core/log/log_cr.inc"
	.include	"./math/div10.inc"
	.include	"./math/div100.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	;---

	.dw	C5_LOG_NUMx8,DIV10


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

	;Инициализация задачи тестирования EEPROM
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START


	TASK__LOGSTR1:
	.db   "/10=",0x00,0x00
	TASK__LOGSTR2:
	.db   "[0x",0x00
	TASK__LOGSTR3:
	.db   "]",0x0a,0x0d,0x00
	TASK__LOGSTR4:
	.db   "59942/100(DIV100)=",0x00,0x00
;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
	LDI LOOP_CNTR,0x00
TASK__LOOP:
	MOV TEMP,LOOP_CNTR
	MCALL C5_LOG_NUMx8
	C5_LOG_ROMSTR TASK__LOGSTR1
	MCALL DIV10
	MOV TEMP,TEMP_L
	MCALL C5_LOG_NUMx8
	C5_LOG_ROMSTR TASK__LOGSTR2
	MCALL C5_LOG_BYTE
	C5_LOG_ROMSTR TASK__LOGSTR3
	INC LOOP_CNTR
	CPI LOOP_CNTR,0x00
	BRNE TASK__LOOP

	LDI TEMP_H,high(59942)
	LDI TEMP_L,low(59942)
	C5_LOG_ROMSTR TASK__LOGSTR4
	MCALL DIV100
	MCALL C5_LOG_NUMx16

	RET

