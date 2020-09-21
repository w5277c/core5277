;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;17.06.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

;---CONSTANTS--------------------------------------------
	.EQU	DRV_MLX90614_K							= 27315			;1С=273.15К
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
	MCALL CORE5277_RAM_REALLOC
	POP ACCUM
	STD Z+_DRV_MLX90614_I2C_DRV,ACCUM

	MCALL CORE5277_READY
;--------------------------------------------------------
;Основной код, работа с датчиком
;OUT: Z - температура объекта,
;Флаг Z - результат(true = успешно)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X
	PUSH_Y

	MCALL CORE5277_RAM_OFFSET

	LDD TEMP,Z+_DRV_MLX90614_I2C_DRV
	LDI YH,high(_DRV_MLX90614_DATAPCKET)|0x80
	LDI YL,low(_DRV_MLX90614_DATAPCKET)
	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP_H,_DRV_MLX90614_SEND_LEN
	LDI TEMP_L,_DRV_MLX90614_RECV_LEN
	LDI ACCUM,_DRV_MLX90614_I2C_ADDR
	MCALL CORE5277_EXEC
	CPI TEMP,DRV_I2C_STATE_OK
	BRNE _DRV_MLX90614__ERROR

	;Записываю данные в регистры
	LDD TEMP,Z+_DRV_MLX90614_RESULT+0x01
	LDD ZL,Z+_DRV_MLX90614_RESULT+0x00
	MOV ZH,TEMP
	SEZ
	RJMP _DRV_MLX90614__END
_DRV_MLX90614__ERROR:

	CLZ
_DRV_MLX90614__END:
	POP_Y
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	RET
