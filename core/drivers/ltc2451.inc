;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;20.09.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;16 бит АЦП с шиной I2C
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./math/mul16x8.inc"
.include	"./math/div16x8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_LTC2451_FREQ_60Hz				= 0x00			;Частота преобразования - 60Гц
	.EQU	DRV_LTC2451_FREQ_30Hz				= 0x01			;Частота преобразования - 30Гц

	.EQU	_DRV_LTC2451_I2C_ADDR				= 0x14			;Адрес устройства
	
	.EQU	_DRV_LTC2451_FREQ						= 0x00			;1B-частота преобразования
	.EQU	_DRV_LTC2451_I2C_DRV					= 0x01			;1B-ид драйвера шины I2C
	.EQU	_DRV_LTC2451_BUFFER					= 0x02			;2B-буффер для показания
	;---
	.EQU	_DRV_LTC2451_RAM_SIZE				= 0x04

;--------------------------------------------------------
DRV_LTC2451_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид драйвера I2C
;FLAGS-частота преобразования(DRV_LTC2451_FREQ_...)
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_LTC2451_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Y+_DRV_LTC2451_FREQ,FLAGS
	STD Y+_DRV_LTC2451_I2C_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, работа с датчиком
;OUT: TEMP-результат(DRV_RESULT_...,
;TEMP_H,TEMP_L-значение измерения
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_LTC2451_I2C_DRV
	MOVW ZL,YL
	MOVW XL,YL
	ADIW Xl,_DRV_LTC2451_BUFFER
	LDI TEMP_H,0x01
	LDI TEMP_L,0x02
	LDI ACCUM,_DRV_LTC2451_I2C_ADDR
	MCALL C5_EXEC	

	LDD TEMP_H,Y+_DRV_LTC2451_BUFFER+0x00
	LDD TEMP_L,Y+_DRV_LTC2451_BUFFER+0x01

	POP ACCUM
	POP_Z
	POP_Y
	POP_X
	RET
