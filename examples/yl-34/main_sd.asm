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
	.SET	c5_DRIVERS_QNT							= 2
	.SET	C5_TASKS_QNT							= 1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x00;0200;Размер общего буфера (буфер для SD)
	.SET	LOGGING_PORT							= PB0	;PA0-PC7
	.SET	LOGGING_LEVEL							= LOGGING_LVL_PNC
	.SET	INPUT_PORT								= PD2

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

	SBI DDRB,PB2 & 0x0f
	CBI PORTB,PB2 & 0x0f

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
	MCALL C5_LOG_CR

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_INIT
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__REPEATE

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_OCR
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__REPEATE
.ifdef LOGGING_PORT
	MOV ZH,YH
	MOV ZL,YL
	MCALL DRV_SD_LOG_OCR
.endif

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CID
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__REPEATE
.ifdef LOGGING_PORT
	MOV ZH,YH
	MOV ZL,YL
	MCALL DRV_SD_LOG_CID
.endif

	LDI TEMP,PID_SD_DRV
	LDI FLAGS,DRV_SD_OP_GET_CSD
	MCALL C5_EXEC
	CPI ACCUM,0x00
	BRNE _TASK__REPEATE
.ifdef LOGGING_PORT
	MOV ZH,YH
	MOV ZL,YL
	MCALL DRV_SD_LOG_CSD
.endif

_TASK__REPEATE:
	MCALL INPUT_GET_CHAR
MCALL C5_LOG_CHAR
	RJMP _TASK__LOOP

;Есть проблема с памятью. Для операций чтения и записи необходимо выделить буфер.
;Карты 1-ой версии опционально поддерживают частичные чтение и запись. И позво-
;ляют задавать размер буфера. Но с версии 2 размер буфера фиксирован - 512 байт и нет поддержки частичных чтения/записи.
;В режиме чтения можно пропускать часть полученных данных и отменять чтение, таким образом можно выделить только нужный объем памяти.
;Но с записью так не выйдет (если только запись без стирания), мы не можем пропустить часть данных.
;Если выделять буфер в 512 в глобальном буфере ядра, то памяти(у данного МК 1КБ) останется немного.

;Необходимо разорбаться в принципе работы особенно с механизмом стирания.
