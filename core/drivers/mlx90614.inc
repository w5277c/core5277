;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.06.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/math/mul24x8.inc"
.include	"./inc/math/div16x8.inc"

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
	.db 0x07																;Tobj1
;--------------------------------------------------------
DRV_MLX90614_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид драйвера i2c
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_MLX96014_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Z+_DRV_MLX90614_I2C_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, работа с датчиком
;OUT: Z - температура объекта,
;Флаг Z - результат(true = успешно)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_X
	PUSH_Y

	MCALL C5_RAM_OFFSET

	LDD TEMP,Z+_DRV_MLX90614_I2C_DRV
	LDI YH,high(_DRV_MLX90614_DATAPCKET)|0x80
	LDI YL,low(_DRV_MLX90614_DATAPCKET)
	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP_H,_DRV_MLX90614_SEND_LEN
	LDI TEMP_L,_DRV_MLX90614_RECV_LEN
	LDI ACCUM,_DRV_MLX90614_I2C_ADDR
	MCALL C5_EXEC
	CPI TEMP,DRV_I2C_STATE_OK
	BRNE _DRV_MLX90614__ERROR


	CLR TEMP_EL
	LDD TEMP_H,Z+_DRV_MLX90614_RESULT+0x01
	LDD TEMP_L,Z+_DRV_MLX90614_RESULT+0x00
	LDI TEMP,0x02
	MCALL C5_MUL24X8
	SUBI TEMP_L,low(_DRV_MLX90614_K)
	SBCI TEMP_H,high(_DRV_MLX90614_K)
	MOV ACCUM,TEMP_L
	LDI TEMP,100
	MCALL C5_DIV16X8
	MOV ZH,TEMP_L
	CLR TEMP_EL
	CLR TEMP_H
	LDI TEMP,100
	MCALL C5_MUL24X8
	MOV ZL,ACCUM
	SUB ZL,TEMP_L
	SEZ
	RJMP _DRV_MLX90614__END
_DRV_MLX90614__ERROR:

	CLZ
_DRV_MLX90614__END:
	POP_Y
	POP_X
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	RET
