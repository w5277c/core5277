;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.03.2020	w5277c@gmail.com			Начало
;30.08.2020	w5277c@gmail.com			Адаптировано под изменения ядра
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;12.02.2020	w5277c@gmail.com			CRC8_MAXIM_BLOCK->CRC8_BLOCK
;03.07.2021	w5277c@gmail.com			Z->Y
;26.08.2021	w5277c@gmail.com			Вывод приведен к общему виду с драйвером AM2301 и DHT11
;20.12.2021	w5277c@gmail.com			Приведение результата к виду DRV_RESULT_...
;24.02.2022	w5277c@gmail.com			Исправлена ошибка при отрицательной температуре
;17.09.2022	konstantin@5277.ru		Корректное вычисление
;-----------------------------------------------------------------------------------------------------------------------
;TODO: Не пройден тест, ERROR
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_DS18B20
.else
.set DEF__C5_DRV_DS18B20 = 1
.message "Included driver DS18B20 v0.8"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc8_block.inc"
.include	"./conv/crc8_maxim.inc"
.include	"./math/mul16x8.inc"
.include	"./math/div16x8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_DS18B20_BUFFER					= 0x00			;9b - буффер для чтения с шины 1w
	.EQU		_DRV_DS18B20_BUFFER_TL			= 0x00
	.EQU		_DRV_DS18B20_BUFFER_TH			= 0x01
	.EQU		_DRV_DS18B20_BUFFER_CRC			= 0x08
	.EQU	_DRV_DS18B20_1W_DRV					= 0x09			;1b - идентификатор драйвера 1w
;---
	.EQU	_DRV_DS18B20_DELAY					= 750/2			;750ms
	.EQU	_DRV_DS18B20_BUFFER_SIZE			= 0x09

	.EQU	_DRV_DS18B20_RAM_SIZE				= 0x0a			;10 байт необходимо выделить

_DRV_1WIRE_SKIP_ROM_N_CONVERT_T:
	.db   0xcc,0x44
_DRV_1WIRE_SKIP_ROM_N_READ_S:
	.db   0xcc,0xbe

;--------------------------------------------------------
DRV_DS18B20_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид драйвера 1w
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_DS18B20_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Y+_DRV_DS18B20_1W_DRV,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, опрос датчика
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP-результат (DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH ACCUM
	PUSH LOOP_CNTR

	MCALL C5_RAM_OFFSET
	LDD ACCUM,Y+_DRV_DS18B20_1W_DRV

	MOV TEMP,ACCUM
	LDI_Z _DRV_1WIRE_SKIP_ROM_N_CONVERT_T|0x8000
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_PROC__ERROR

	LDI TEMP_H,BYTE3(_DRV_DS18B20_DELAY)
	LDI TEMP_L,BYTE2(_DRV_DS18B20_DELAY)
	LDI TEMP,BYTE1(_DRV_DS18B20_DELAY)
	MCALL C5_WAIT_2MS

	MOV TEMP,ACCUM
	LDI_Z _DRV_1WIRE_SKIP_ROM_N_READ_S|0x8000
	MOVW XL,YL
	LDI TEMP_H,0x02
	LDI TEMP_L,_DRV_DS18B20_BUFFER_SIZE
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_PROC__ERROR

	LDI LOOP_CNTR,0x08
	MOVW XL,YL
	LDI_Z CRC8_MAXIM
	LDI ACCUM,0x00
	MCALL CRC8_BLOCK
	LDD TEMP,Y+_DRV_DS18B20_BUFFER_CRC
	CP TEMP,ACCUM
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ERROR
	RJMP _DRV_DS18B20_PROC__ERROR

	LDD TEMP_H,Y+_DRV_DS18B20_BUFFER_TH
	LDD TEMP_L,Y+_DRV_DS18B20_BUFFER_TL

	MOV ACCUM,TEMP_H
	SBRS TEMP_H,0x07
	RJMP _DRV_DS18B20__POSITIVE
	COM TEMP_H
	COM TEMP_L
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
_DRV_DS18B20__POSITIVE:

	MOV TEMP,TEMP_L													;Преобразуем в человеческий вид целую часть
	ANDI TEMP,0xf0
	SWAP TEMP
	ANDI TEMP_H,0x0f
	SWAP TEMP_H
	OR TEMP_H,TEMP
	PUSH TEMP_H

	ANDI TEMP_L,0x0f													;Оставили только дробную часть
	MOV TEMP,TEMP_L
	LDI TEMP_H,0x02													;Умножаем на 625
	LDI TEMP_L,0x71
	MCALL MUL16x8
	LDI TEMP,100														;Делим на 100
	MCALL DIV16x8														;В TEMP_L - дробная часть в человеческом формате 0-99

	POP TEMP_H
	ANDI ACCUM,0x80
	OR TEMP_L,ACCUM
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_DS18B20_PROC__END

_DRV_DS18B20_PROC__ERROR:
	LDI TEMP_L,0xff
_DRV_DS18B20_PROC__END:
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
.endif
