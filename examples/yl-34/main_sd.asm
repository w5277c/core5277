;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 8	;2-20Mhz

	.INCLUDE "./devices/atmega16.inc"
	;Важные, но не обязательные параметры ядра
	.SET	AVRA										= 1	;0-1
	.SET	REALTIME									= 1	;0-1
	.SET	C5_DRIVERS_QNT							= 2
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x200;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_DBG
	.SET	INPUT_PORT								= PB1

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/sd_spi.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/wait_1s.inc"
	.include	"./core/log/log_cr.inc"
	.include "./io/input_get_char.inc"
	.include	"./core/drivers/sd/sd_log_ocr.inc"
	.include	"./core/drivers/sd/sd_log_cid.inc"
	.include	"./core/drivers/sd/sd_log_csd.inc"
	.include	"./core/log/log_ramdump.inc"
	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_SD_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
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

	;Инициализация SD
	LDI PID,PID_SD_DRV
	LDI ZH,high(DRV_SD_INIT)
	LDI ZL,low(DRV_SD_INIT)
	LDI YH,high(C5_BUFFER)
	LDI YL,low(C5_BUFFER)
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI ZH,high(TASK__INIT)
	LDI ZL,low(TASK__INIT)
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
_TASK__LOOP:
	MCALL INPUT_GET_CHAR

	MCALL C5_LOG_CR

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_INIT
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__LOOP

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_OCR
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__LOOP
.ifdef LOGGING_PORT
	MOV XH,YH
	MOV XL,YL
	MCALL DRV_SD_LOG_OCR
.endif

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CID
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__LOOP
.ifdef LOGGING_PORT
	MOV XH,YH
	MOV XL,YL
	MCALL DRV_SD_LOG_CID
.endif

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CSD
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__LOOP
.ifdef LOGGING_PORT
	MOV XH,YH
	MOV XL,YL
	MCALL DRV_SD_LOG_CSD
.endif

	MCALL INPUT_GET_CHAR
	MCALL C5_LOG_CHAR




	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_ERASE_BLOCKS
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI ACCUM,0x63	;+1(100 блок)
	MCALL C5_EXEC

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_ERASE_BLOCKS
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	LDI ACCUM,0x8f	;+1(400 блока)
	MCALL C5_EXEC


	LDI_Y C5_BUFFER
	LDI LOOP_CNTR,0x00
_TASK__FILL_LOOP:
	ST Y+,LOOP_CNTR
	ST Y+,LOOP_CNTR
	INC LOOP_CNTR
	BRNE _TASK__FILL_LOOP

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC


	LDI TEMP,0x01
	LDI_Y C5_BUFFER
	LDI LOOP_CNTR,0x00
_TASK__FILL_LOOP2:
	ST Y+,TEMP
	ST Y+,TEMP
	INC LOOP_CNTR
	BRNE _TASK__FILL_LOOP2

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	MCALL C5_EXEC


	LDI TEMP,0xf0
	LDI_Y C5_BUFFER
	LDI LOOP_CNTR,0x00
_TASK__FILL_LOOP3:
	ST Y+,TEMP
	ST Y+,TEMP
	INC LOOP_CNTR
	BRNE _TASK__FILL_LOOP3

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x02
	MCALL C5_EXEC


	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_READ_BLOCK
	LDI TEMP_EH,0x00													;TEMP_EH/EL/H/L-номер блока
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	RJMP _TASK__LOOP

;Есть проблема с памятью. Для операций чтения и записи необходимо выделить буфер.
;Карты 1-ой версии опционально поддерживают частичные чтение и запись. И позво-
;ляют задавать размер буфера. Но с версии 2 размер буфера фиксирован - 512 байт и нет поддержки частичных чтения/записи.
;В режиме чтения можно пропускать часть полученных данных и отменять чтение, таким образом можно выделить только нужный объем памяти.
;Но с записью так не выйдет (если только запись без стирания), мы не можем пропустить часть данных.
;Если выделять буфер в 512 в глобальном буфере ядра, то памяти(у данного МК 1КБ) останется немного.

;Необходимо разорбаться в принципе работы особенно с механизмом стирания.
