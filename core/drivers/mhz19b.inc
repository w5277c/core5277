;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.09.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер датчика CO2 MH-Z19B(на базе UART 9600N1)
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./conv/sum.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_MHZ19B_UART_ID							= 0x00	;1B-ид драйвера UART
	.EQU	_DRV_MHZ19B_BUFFER							= 0x01	;1B-ид драйвера UART
	
	.EQU	_DRVMHZ19B_RAM_SIZE							= 0x0a	;Необходимо выделить

DRV_MHZ19B_REQ_DATA:
	.db	0xff,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79,0x00

;--------------------------------------------------------
DRV_MHZ19B_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;IN: TEMP_H-ид UART драйвера
;--------------------------------------------------------
	LDI ACCUM,_DRVMHZ19B_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_MHZ19B_UART_ID,TEMP_H

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, получаем значение CO2
;OUT: TEMP-результат(DRV_RESULT_...)
;TEMP_H/TEMP_L-значение
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH LOOP_CNTR
	PUSH FLAGS

	;Загружаем в Y адрес на выделенный блок памяти
	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_MHZ19B_UART_ID
	ADIW YL,_DRV_MHZ19B_BUFFER										;Смещаемся на буффер
	LDI_Z DRV_MHZ19B_REQ_DATA|0x8000
	MOVW XL,YL
	LDI TEMP_EH,0x09													;Размер запроса
	LDI TEMP_EL,0x09													;Размер буфера
	LDI TEMP_H,0x00													;100ms
	LDI TEMP_L,0x32
	MCALL C5_EXEC
	
	LDI TEMP,DRV_RESULT_ABSENT
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE DRV_MHZ19B__ERROR
	LDI TEMP,DRV_RESULT_INVALID_DATA
	LDD TEMP_EH,Y+0x00
	CPI TEMP_EH,0xff
	BRNE DRV_MHZ19B__ERROR
	LDD TEMP_EH,Y+0x01
	CPI TEMP_EH,0x86
	BRNE DRV_MHZ19B__ERROR
	LDD TEMP_H,Y+0x02
	LDD TEMP_L,Y+0x03

	LDI LOOP_CNTR,0x08
	;MOVW XL,YL															;X не изменялся
	MCALL SUM
	COM ACCUM
	LDD TEMP_EH,Y+0x08
	CP ACCUM,TEMP_EH
	BRNE DRV_MHZ19B__ERROR

	LDI TEMP,DRV_RESULT_OK
	RJMP DRV_MHZ19B__END

DRV_MHZ19B__ERROR:
	LDI TEMP_H,0xff
	LDI TEMP_L,0xff
DRV_MHZ19B__END:

	POP FLAGS
	POP LOOP_CNTR
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	POP_Z
	POP_Y
	POP_X
	RET
