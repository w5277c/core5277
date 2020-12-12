;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.12.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./devices/atmega16.inc"
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	TIMERS									= 0	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.INCLUDE "./core/drivers/spi_ms.inc"
	;Блок задач
	;---
	;Дополнительно
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_SPI_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_SPI_TASK							= 0
	;Идентификаторы таймеров
	;---

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

	;Инициализация SPI
	LDI PID,PID_SPI_DRV
	LDI ZH,high(DRV_SPI_MS_INIT)
	LDI ZL,low(DRV_SPI_MS_INIT)
	LDI TEMP_EH,PB1													;SCK
	LDI TEMP_EL,PB2													;MISO
	LDI TEMP_H,PB3														;MOSI
	LDI TEMP_L,PB4														;SS
	LDI FLAGS,0x00														;PAUSE
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_SPI_TASK
	LDI ZH,high(SPI_TASK__INIT)
	LDI ZL,low(SPI_TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	SPI_TASK__DATA:
	.db 0x00,0x01,0x02,0x03,0x04

SPI_TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
SPI_TASK__INFINITE_LOOP:
	LDI YH,high(SPI_TASK__DATA)|0x80								;Вызываем процедуру драйвера (данные мелодии берем из ROM)
	LDI YL,low(SPI_TASK__DATA)
	LDI TEMP_L,0x05
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC

	LDI TEMP,0x01														;Пауза в 5 сеунд
	MCALL C5_WAIT_1S
	RJMP SPI_TASK__INFINITE_LOOP

