;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.03.2020  w5277c@gmail.com        Начало
;30.08.2020  w5277c@gmail.com        Адаптировано под изменения ядра
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/conv/crc8_maxim_block.inc"
.include	"./inc/math/mul24x8.inc"
.include	"./inc/math/div16x8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_DS18B20_BUFFER					= 0x00			;9b - буффер для чтения с шины 1w
	.EQU		_DRV_DS18B20_BUFFER_TL			= 0x00
	.EQU		_DRV_DS18B20_BUFFER_TH			= 0x01
	.EQU		_DRV_DS18B20_BUFFER_CRC			= 0x08
	.EQU	_DRV_DS18B20_1W_DRV					= 0x09			;1b - идентификатор драйвера 1w
;---
	.EQU	_DRV_DS18B20_DELAY					= 750/2			;750с
	.EQU	_DRV_DS18B20_BUFFER_SIZE			= 0x09

	.EQU	_DRV_DS18B20_RAM_SIZE				= 0x0a			;10 байт необходимо выделить

;---RESULTS---
	.EQU	DRV_DS18B20_RESULT_NO_DRIVER_ID	= 0x00
	.EQU	DRV_DS18B20_RESULT_ABSENT1			= 0x01
	.EQU	DRV_DS18B20_RESULT_ABSENT2			= 0x02
	.EQU	DRV_DS18B20_RESULT_BAD_CRC			= 0x03

_DRV_1WIRE_SKIP_ROM_N_CONVERT_T:
	.db   0xcc,0x44
_DRV_1WIRE_SKIP_ROM_N_READ_S:
	.db   0xcc,0xbe

;--------------------------------------------------------
DRV_DS18B20_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид драйвера 1w
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_DS18B20_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Z+_DRV_DS18B20_1W_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: TEMP_H,TEMP_L - значение температуры в SDNF формате
;TEMP_L = 0xff - ошибка, код в TEMP_H:
;0x01,0x02 - датчик не обнаружен
;0x03 - ошибка CRC
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR

	MCALL C5_RAM_OFFSET
	LDD ACCUM,Z+_DRV_DS18B20_1W_DRV

	MOV TEMP,ACCUM
	LDI YH,high(_DRV_1WIRE_SKIP_ROM_N_CONVERT_T)|0x80
	LDI YL,low(_DRV_1WIRE_SKIP_ROM_N_CONVERT_T)
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_DS18B20_RESULT_ABSENT1
	RJMP _DRV_DS18B20_PROC__ERROR

	LDI TEMP_H,low(_DRV_DS18B20_DELAY/0x10000)
	LDI TEMP_L,low(_DRV_DS18B20_DELAY/0x100)
	LDI TEMP,low(_DRV_DS18B20_DELAY)
	MCALL C5_WAIT_2MS

	MOV TEMP,ACCUM
	LDI YH,high(_DRV_1WIRE_SKIP_ROM_N_READ_S)|0x80
	LDI YL,low(_DRV_1WIRE_SKIP_ROM_N_READ_S)
	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP_H,0x02
	LDI TEMP_L,_DRV_DS18B20_BUFFER_SIZE
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_DS18B20_RESULT_ABSENT2
	RJMP _DRV_DS18B20_PROC__ERROR

	LDI LOOP_CNTR,0x08
	MCALL C5_CRC8_MAXIM_BLOCK
	LDD TEMP,Z+_DRV_DS18B20_BUFFER_CRC
	CP TEMP,ACCUM
	BREQ PC+0x03
	LDI TEMP,0x03
	RJMP _DRV_DS18B20_PROC__ERROR

	LDD TEMP_H,Z+_DRV_DS18B20_BUFFER_TH
	LDD TEMP_L,Z+_DRV_DS18B20_BUFFER_TL
	;Пребразовываем в SDNF
	MOV TEMP,TEMP_L													;Преобразуем в человеческий вид целую часть
	ANDI TEMP,0xF0
	SWAP TEMP
	SWAP TEMP_H
	OR TEMP_H,TEMP
	PUSH TEMP_H

	ANDI TEMP_L,0x0f													;Оставили только дробную часть
	MOV TEMP,TEMP_L
	CLR TEMP_EL
	LDI TEMP_H,0x02													;Умножаем на 625
	LDI TEMP_L,0x71
	MCALL C5_MUL24x8
	LDI TEMP,100														;Делим на 100
	MCALL C5_DIV16x8													;В TEMP_L - дробная часть в человеческом формате 0-99

	POP TEMP_H
	BRTC PC+0x02														;Переносим знак в новый формат
	ORI TEMP_L,0x80
	RJMP _DRV_DS18B20_PROC__END

_DRV_DS18B20_PROC__ERROR:
	LDI TEMP_L,0xff
	MOV TEMP_H,TEMP

_DRV_DS18B20_PROC__END:
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
