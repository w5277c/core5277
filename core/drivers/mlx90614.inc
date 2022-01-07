;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.06.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;03.07.2021	w5277c@gmail.com			Z->Y
;20.12.2021	w5277c@gmail.com			Приведение результата к виду DRV_RESULT_...
;-----------------------------------------------------------------------------------------------------------------------
;TODO поддержка отрицательной температуры
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_MLX90614
.else
.set DEF__C5_DRV_MLX90614 = 1
.message "Included driver MLX90614 v0.4"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./conv/word_to_sdnf.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_MLX90614_K						= 27315			;1С=273.15К
	.EQU	_DRV_MLX90614_I2C_ADDR				= 0x5a			;Адрес устройства на шине I2C
	.EQU	_DRV_MLX90614_SEND_LEN				= 0x01			;Пишем 1 байт
	.EQU	_DRV_MLX90614_RECV_LEN				= 0x02			;Читаем 2 байта
	;---VARS---
	.EQU	_DRV_MLX90614_RESULT					= 0x00			;2 байта температура датчика
	.EQU	_DRV_MLX90614_I2C_DRV				= 0x02			;1 байт, ид драйвера шины i2c
	;---
	.EQU	_DRV_MLX96014_RAM_SIZE				= 0x03

_DRV_MLX90614_DATAPCKET:
	.db 0x07,0x00														;Tobj1
;--------------------------------------------------------
DRV_MLX90614_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид драйвера i2c
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_MLX96014_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Y+_DRV_MLX90614_I2C_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, работа с датчиком
;OUT: TEMP_H,L-температура объекта,
;Флаг TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_X
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_MLX90614_I2C_DRV
	LDI_Z _DRV_MLX90614_DATAPCKET|0x8000
	MOVW XL,YL
	LDI TEMP_H,_DRV_MLX90614_SEND_LEN
	LDI TEMP_L,_DRV_MLX90614_RECV_LEN
	LDI ACCUM,_DRV_MLX90614_I2C_ADDR
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_MLX90614__ERROR

	LDD TEMP_H,Y+_DRV_MLX90614_RESULT+0x01
	LDD TEMP_L,Y+_DRV_MLX90614_RESULT+0x00
	LSL TEMP_L
	ROL TEMP_H
	SUBI TEMP_L,low(_DRV_MLX90614_K)
	SBCI TEMP_H,high(_DRV_MLX90614_K)
	MCALL WORD_TO_SDNF
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_MLX90614__END
_DRV_MLX90614__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_MLX90614__END:

	POP_Z
	POP_Y
	POP_X
	POP ACCUM
	RET
.endif
