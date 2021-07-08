;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.10.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.SET	CORE_FREQ								= 16	;2-20Mhz
	.SET	TIMER_C_ENABLE							= 0	;0-1
	.INCLUDE "./devices/atmega328.inc"

	.SET	TS_MODE									= TS_MODE_TIME		;TS_MODE_NO/TS_MODE_EVENT/TS_MODE_TIME
	.SET	OPT_MODE									= OPT_MODE_SPEED	;OPT_MODE_SPEED/OPT_MODE_SIZE
	.SET	AVRA										= 1	;0-1
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	TIMERS									= 1	;0-4
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= SCK	;PA0-PC7

;---INCLUDES---------------------------------------------
	.INCLUDE "./core/core5277.inc"
	;Блок драйверов
	.include	"./core/drivers/adc.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./core/log/log_word.inc"

.include	"./core/wait_1s.inc"
.include	"./core/log/log_byte.inc"
;	;---

;---CONSTANTS--------------------------------------------
	;Идентификаторы драйверов(0-7|0-15)
	.EQU	PID_ADC_DRV								= 0|(1<<C5_PROCID_OPT_DRV)
	;Идентификаторы задач(0-3|0-15)
	.EQU	PID_TASK									= 0
	;Идентификаторы таймеров
	.EQU	TID_ADC									= 0
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

	;Инициализация ADC
	LDI PID,PID_ADC_DRV
	LDI_Z DRV_ADC_INIT
	LDI ACCUM,TID_ADC
	MCALL C5_CREATE

	;Инициализация задачи тестирования
	LDI PID,PID_TASK
	LDI_Z TASK__INIT
	MCALL C5_CREATE

	MJMP C5_START

;--------------------------------------------------------;Задача
TASK__INIT:
	MCALL C5_READY
;--------------------------------------------------------
TASK__INFINITE_LOOP:
	;Поиск нуля фазы 50Гц
	LDI TEMP,PID_ADC_DRV
	LDI ACCUM,ADC1
	LDI TEMP_EH,ADC_VREF_AVCC										;Опорное напряжение AVcc со внешним конденсатором на aref пине
	LDI TEMP_EL,ADC_PRESC_X128										;Делитель в x128
	LDI TEMP_H,0x64													;Кол-во итераций для подсчета среднего
	LDI TEMP_L,0x08|(0<<0x07)										;Периодичность итераций 1/50/50/0.000050=8 (~50Гц с 50-ю выборками)*2
	LDI_X 0x0000
	MCALL C5_EXEC


;	;Находим среднее амплитуды
;	LDI TEMP,PID_ADC_DRV
;	MOV XH,TEMP_H
;	ORI XH,(1<<0x07)													;Старший бит - суммировать по модулю
;	MOV XL,TEMP_L														;Устанавливаем среднее как значение нуля фазы
;	LDI TEMP,PID_ADC_DRV
;	LDI ACCUM,ADC1
;	LDI TEMP_EH,ADC_VREF_AVCC										;Опорное напряжение AVcc со внешним конденсатором на aref пине
;	LDI TEMP_EL,ADC_PRESC_X128										;Делитель в x1
;	LDI TEMP_H,0x32													;Кол-во итераций для подсчета среднего
;	LDI TEMP_L,0x08|(0<<0x07)										;Периодичность итераций 1/50/50/0.000050=8 (~50Гц с 50-ю выборками)
;	MCALL C5_EXEC

	MCALL C5_LOG_WORD
	MCALL C5_LOG_CR

	LDI TEMP,0x01
	MCALL C5_WAIT_1S

	RJMP TASK__INFINITE_LOOP

