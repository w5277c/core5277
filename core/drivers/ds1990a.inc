;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.03.2020  w5277c@gmail.com			Начало
;13.09.2020  w5277c@gmail.com			Актуализирован
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;12.02.2020  w5277c@gmail.com			CRC8_MAXIM_BLOCK->CRC8_BLOCK
;16.09.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./conv/crc8_block.inc"
.include	"./conv/crc8_maxim.inc"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_DS1990A_BUFFER					= 0x00			;7b - буффер для чтения с шины 1w
	.EQU		DRV_DS1990A_BUFFER_FC			= 0x00			;1b - family code
	.EQU		DRV_DS1990A_BUFFER_SER			= 0x01			;6b - serial
	.EQU		DRV_DS1990A_BUFFER_CRC			= 0x07			;1b - crc
	.EQU	_DRV_DS1990A_1W_DRV					= 0x08			;1b - идентификатор драйвера 1w
;---
	.EQU	_DRV_DS1990A_RAM_SIZE				= 0x09

_DRV_DS1990A_READ_ROM:
	.db   0x33,0x00

;--------------------------------------------------------
DRV_DS1990A_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид драйвера 1w
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_DS1990A_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Y+_DRV_DS1990A_1W_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: Z-адрес на блок UID(6 байт)
;TEMP-код результата
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM
	PUSH LOOP_CNTR

	MCALL C5_RAM_OFFSET
	LDD ACCUM,Y+_DRV_DS1990A_1W_DRV

	CPI ACCUM,0xFF
	BRNE PC+0x03
	LDI TEMP,DRV_RESULT_INVALID_RESOURCE
	RJMP _DRV_DS1990A_PROC__ERROR

	MOV TEMP,ACCUM
	LDI_Z _DRV_DS1990A_READ_ROM|0x8000
	LDI TEMP_H,0x01
	LDI TEMP_L,0x08
	MOVW XL,YL
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS1990A_PROC__END

	LDI LOOP_CNTR,0x07
	MOVW XL,YL
	LDI_Z CRC8_MAXIM
	MCALL CRC8_BLOCK
	LDD TEMP,Y+DRV_DS1990A_BUFFER_CRC
	CP TEMP,ACCUM
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_INVALID_DATA
	RJMP _DRV_DS1990A_PROC__ERROR

	MOVW ZL,YL
	LDI TEMP,DRV_RESULT_OK
_DRV_DS1990A_PROC__ERROR:
_DRV_DS1990A_PROC__END:
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP_Y
	POP_X
	RET
