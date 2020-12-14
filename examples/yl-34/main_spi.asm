;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.12.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

;TODO наблюдаются проблемы с TIMER_B

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
	LDI XH,PB1															;SCK
	LDI XL,PB2															;MISO
	LDI YH,PB3															;MOSI
	LDI YL,PB4															;SS
	LDI TEMP_H,0x02													;Кол-во устройств на шине
	LDI TEMP_L,0x02													;Размер пакета (в байтах)
	LDI ACCUM,0x80;40													;PAUSE
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_SPI_TASK
	LDI ZH,high(SPI_TASK__INIT)
	LDI ZL,low(SPI_TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
	SPI_TASK__DATA_INIT:
	.db 0x0f,0x00,0x0c,0x01,0x0b,0x07,0x09,0x00,0x0a,0x01, 0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,0x00
	SPI_TASK__DATA1_CHIP1:
	.db 0x08,0xff
	SPI_TASK__DATA2_CHIP1:
	.db 0x08,0x00
	SPI_TASK__DATA1_CHIP2:
	.db 0x01,0x80
	SPI_TASK__DATA2_CHIP2:
	.db 0x01,0x01

SPI_TASK__INIT:														;MAX7219 8x8 led matrix
	MCALL C5_READY
;--------------------------------------------------------
	LDI YH,high(SPI_TASK__DATA_INIT)|0x80
	LDI YL,low(SPI_TASK__DATA_INIT)
	LDI TEMP_L,0x0d
	LDI TEMP_H,0x00
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC
	LDI TEMP_H,0x01
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC

SPI_TASK__INFINITE_LOOP:
	LDI YH,high(SPI_TASK__DATA1_CHIP1)|0x80
	LDI YL,low(SPI_TASK__DATA1_CHIP1)
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC
	LDI YH,high(SPI_TASK__DATA1_CHIP2)|0x80
	LDI YL,low(SPI_TASK__DATA1_CHIP2)
	LDI TEMP_H,0x01
	LDI TEMP_L,0x01
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x64
	MCALL C5_WAIT_2MS

	LDI YH,high(SPI_TASK__DATA2_CHIP1)|0x80
	LDI YL,low(SPI_TASK__DATA2_CHIP1)
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC
	LDI YH,high(SPI_TASK__DATA2_CHIP2)|0x80
	LDI YL,low(SPI_TASK__DATA2_CHIP2)
	LDI TEMP_H,0x01
	LDI TEMP_L,0x01
	LDI TEMP,PID_SPI_DRV
	MCALL C5_EXEC

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x64
	MCALL C5_WAIT_2MS

	RJMP SPI_TASK__INFINITE_LOOP

