;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;23.06.2024	konstantin@5277.ru		Взято из ds18b20.inc, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_GET, DRV_OP_GET_BY_ID
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_DS18B20
.else
.set DEF__C5_DRV_DS18B20 = 1
.message "Included driver DS18B20 v2.0"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc8_block.inc"
.include	"./conv/crc8_maxim.inc"
.include	"./math/mul16x8.inc"
.include	"./math/div16x8.inc"
.include "./mem/ram_copy.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_DS18B20_BUFFER_TL				= 0x00
	.EQU	_DRV_DS18B20_BUFFER_TH				= 0x01
	.EQU	_DRV_DS18B20_BUFFER_CRC				= 0x08
	.EQU	_DRV_DS18B20_DATA_SIZE				= 0x09
	.EQU	_DRV_DS18B20_DELAY					= 750/2			;750ms

	.EQU	_DRV_DS18B20_BUFFER					= 0x00			;10B-буфер для чтения с шины 1w
	.EQU	_DRV_DS18B20_1W_DRV					= 0x0a			;1B-ид драйвера 1w
	.EQU	_DRV_DS18B20_RAM_SIZE				= 0x0a			;11 байт необходимо выделить

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
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	LDD ACCUM,Y+_DRV_DS18B20_1W_DRV

	CPI FLAGS,DRV_OP_GET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS18B20_GET
	RJMP _DRV_DS18B20__END
	CPI FLAGS,DRV_OP_GET_BY_ID
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS18B20_GET_BY_ID

_DRV_DS18B20__END:
	POP ACCUM
	POP_Y
	RET

;--------------------------------------------------------
_DRV_DS18B20_GET:
;--------------------------------------------------------
;Чтение показания через SKIP ROM(0xCC)
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP-результат (DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_Z

	LDI TEMP,0xCC
	STD Y+0x00,TEMP
	LDI TEMP,0x44
	STD Y+0x01,TEMP
	MOV TEMP,ACCUM
	MOVW ZL,YL
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_GET__END

	RCALL _DRV_DS18B20_WAIT

	LDI TEMP,0xBE
	STD Y+0x01,TEMP
	MOV TEMP,ACCUM
	MOVW ZL,YL
	LDI TEMP_H,0x02
	LDI TEMP_L,_DRV_DS18B20_DATA_SIZE
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_GET__END

	RCALL _DRV_DS18B20_CRC_CHECK
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_DS18B20_GET__END

	RCALL _DRV_DS18B20_GET_SDNF
	LDI TEMP,DRV_RESULT_OK

_DRV_DS18B20_GET__END:
	POP_Z
	RET

;--------------------------------------------------------
_DRV_DS18B20_GET_BY_ID:
;--------------------------------------------------------
;Чтение показания через MATCH ROM(0x55)
;IN: X-адрес на ROM(8байт)
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;TEMP-результат (DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_Z

	LDI TEMP,0x55
	STD Y+0x00,TEMP
	LDI TEMP,0x44
	STD Y+0x019,TEMP

	PUSH LOOP_CNTR
	MOVW ZL,YL
	ADIW ZL,0x01
	LDI LOOP_CNTR,0x08
	MCALL RAM_COPY
	POP LOOP_CNTR

	MOV TEMP,ACCUM
	MOVW ZL,YL
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_GET_BY_ID__END

	RCALL _DRV_DS18B20_WAIT

	LDI TEMP,0xBE
	STD Y+0x01,TEMP
	MOV TEMP,ACCUM
	MOVW ZL,YL
	LDI TEMP_H,0x02
	LDI TEMP_L,_DRV_DS18B20_DATA_SIZE
	MCALL C5_EXEC
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_DS18B20_GET_BY_ID__END

	RCALL _DRV_DS18B20_CRC_CHECK
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_DS18B20_GET_BY_ID__END

	RCALL _DRV_DS18B20_GET_SDNF
	LDI TEMP,DRV_RESULT_OK

_DRV_DS18B20_GET_BY_ID__END:
	POP_Z
	RET

;--------------------------------------------------------
;Проверка CRC
;TEMP-результат (DRV_RESULT_...)
;--------------------------------------------------------
_DRV_DS18B20_CRC_CHECK:
	PUSH LOOP_CNTR
	PUSH_X

	LDI LOOP_CNTR,_DRV_DS18B20_DATA_SIZE-0x01
	MOVW XL,YL
	LDI_Z CRC8_MAXIM
	LDI ACCUM,0x00
	MCALL CRC8_BLOCK
	LDD TEMP,Y+_DRV_DS18B20_BUFFER_CRC
	CP TEMP,ACCUM
	LDI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	LDI TEMP,DRV_RESULT_ERROR

	POP_X
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
_DRV_DS18B20_WAIT:
	LDI TEMP_H,BYTE3(_DRV_DS18B20_DELAY)
	LDI TEMP_L,BYTE2(_DRV_DS18B20_DELAY)
	LDI TEMP,BYTE1(_DRV_DS18B20_DELAY)
	MCALL C5_WAIT_2MS
	RET

;--------------------------------------------------------
;Конвертирование полученного знчения в SDNF формат
;IN: Y-адрес на выделенную память
;OUT: TEMP_H,TEMP_L-значение температуры в SDNF формате
;--------------------------------------------------------
_DRV_DS18B20_GET_SDNF:
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_EH
	PUSH TEMP_EL

	LDD TEMP_H,Y+_DRV_DS18B20_BUFFER_TH
	LDD TEMP_L,Y+_DRV_DS18B20_BUFFER_TL
	MOV ACCUM,TEMP_H
	SBRS TEMP_H,0x07
	RJMP _DRV_DS18B20_TEMPERATURE_TO_SDNF__POSITIVE
	COM TEMP_H
	COM TEMP_L
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
_DRV_DS18B20_TEMPERATURE_TO_SDNF__POSITIVE:

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

	POP TEMP_EL
	POP TEMP_EH
	POP TEMP
	POP ACCUM
	RET
.endif
