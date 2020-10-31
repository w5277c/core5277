;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.10.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;BUILD: avra  -I ../../ main.asm

	.INCLUDE "./inc/devices/atmega328.inc"
	.SET	AVRA										= 1
	.SET	REALTIME									= 0	;0-1
	.SET	TIMERS									= 1	;0-4
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS
	.SET	BUFFER_SIZE								= 0x00;Размер общего буфера
	.SET	LOGGING_PORT							= PC0	;PA0-PC7
;---INCLUDES---------------------------------------------
	.INCLUDE "core5277.asm"
	;Блок драйверов
	.include	"./inc/drivers/adc.inc"
	;---
	;Блок задач
	;---
	;Дополнительно
	.include	"./inc/io/log_word.inc"
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
	LDI ZH,high(DRV_ADC_INIT)
	LDI ZL,low(DRV_ADC_INIT)
	LDI ACCUM,TID_ADC
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
TASK__INFINITE_LOOP:
	;Поиск нуля фазы 50Гц
	LDI ACCUM,ADC1
	LDI TEMP_EH,ADC_VREF_AVCC										;Опорное напряжение AVcc со внешним конденсатором на aref пине
	LDI TEMP_EL,ADC_PRESC_X128										;Делитель в x128
	LDI TEMP_H,0x64													;Кол-во итераций для подсчета среднего
	LDI TEMP_L,0x08|(0<<0x07)										;Периодичность итераций 1/50/50/0.000050=8 (~50Гц с 50-ю выборками)*2
	LDI XH,0x00
	LDI XL,0x00
	MCALL C5_EXEC

	;Находим среднее амплитуды
	MOV XH,TEMP_H
	ORI XH,(1<<0x07)													;Старший бит - суммировать по модулю
	MOV XL,TEMP_L														;Устанавливаем среднее как значение нуля фазы
	LDI TEMP,PID_ADC_DRV
	LDI ACCUM,ADC1
	LDI TEMP_EH,ADC_VREF_AVCC										;Опорное напряжение AVcc со внешним конденсатором на aref пине
	LDI TEMP_EL,ADC_PRESC_X1										;Делитель в x1
	LDI TEMP_H,0x32													;Кол-во итераций для подсчета среднего
	LDI TEMP_L,0x08|(0<<0x07)										;Периодичность итераций 1/50/50/0.000050=8 (~50Гц с 50-ю выборками)
	MCALL C5_EXEC

	MCALL C5_LOG_WORD
	RJMP TASK__INFINITE_LOOP

