;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.10.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_PWM_S
.else
.set DEF__C5_DRV_PWM_S = 1
.message "Included driver software PWM v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./mem/ram_fill.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_PWM_S_RAM_ITEMS_QNT			= 0x00	;1B - количество каналов
	.EQU	_DRV_PWM_S_RAM_TIMER_ID				= 0x01	;1B - ид таймера
	.EQU	_DRV_PWM_S_RAM_ITEMS_DATA			= 0x02	;xB - структура данных для каждого канала
	;---ITEM-STRUCT---
	.EQU	_DRV_PWM_S_ITEM_PA					= 0x00	;1B - адрес PORT
	.EQU	_DRV_PWM_S_ITEM_BN					= 0x01	;1B - номер бита
	.EQU	_DRV_PWM_S_ITEM_CNTR				= 0x03	;1B - счетчик
	.EQU	_DRV_PWM_S_ITEM_MAX					= 0x04	;1B - максимальное число для счетчика
	;---OPERATIONS---
	.EQU	DRV_PWM_S_OP_CHSET					= 0x00	;Прописываем канал
	
;--------------------------------------------------------
DRV_PWM_S_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: LOOP_CNTR-количество каналов, TEMP_H-ид таймера,
;TEMP_L-периодичность таймера(см. C5_TIMER_SET)
;--------------------------------------------------------
	;Выделяем память (кол-во каналов*4+2)
	MOV ACCUM,LOOP_CNTR
	LSL ACCUM
	LSL ACCUM
	SUBI ACCUM,(0x100-0x02)
	MCALL C5_RAM_REALLOC
	
	;Записываем длину
	STD Y+_DRV_PWM_S_RAM_ITEMS_QNT,LOOP_CNTR
	;Заполняем
	MOVW XL,YL
	MOV LOOP_CNTR,ACCUM
	LDI TEMP,0xff
	MCALL RAM_FILL

	;Настраиваем и запускаем таймер
	MOV ACCUM,TEMP_L
	MOV TEMP,TEMP_H
	LDI TEMP_H,high(_DRV_PWM_S_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_PWM_S_TIMER_EVENT)
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	;Завершаем инициализацию
	MCALL C5_READY

	;TODO
.endif
