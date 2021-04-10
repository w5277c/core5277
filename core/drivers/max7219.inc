;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.11.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать прием данных

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./math/mul8x8.inc"
.include	"./mem/ram_fill8.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_MAX7219_SPI_ID								= 0x00;1b - ид драйвера SPI
	.EQU	_DRV_MAX7219_QNT									= 0x01;1b - количество чипов
	.EQU	_DRV_MAX7219_SCANLIMIT							= 0x02;1b - количество цифр/строк(0-7)

	.EQU	_DRV_MAX7219_RAM_SIZE							= 0x03;минимум 2 байта необходимо выделить

	;---OPERATIONS---
	.EQU	DRV_MAX7219_OP_INIT								= 0x00
	.EQU	DRV_MAX7219_OP_INTENSITY						= 0x01
	.EQU	DRV_MAX7219_OP_SET_BLOCK						= 0x02
	.EQU	DRV_MAX7219_OP_SHOW								= 0x03

;--------------------------------------------------------
DRV_MAX7219_INIT:
;--------------------------------------------------------
;Инициализация драйвера (пока для матрицы 8x8)
;IN: TEMP_H - ид SPI драйвера
;TEMP_L - количество чипов на шине(0-14)
;TEMP_EH - ид таймера
;TEMP_EL - период таймера
;FLAGS - SCANLIMIT(0-7)
;--------------------------------------------------------
	;Умножаю кол-во чипов на кол-во цифр/строк
	PUSH TEMP_L
	PUSH FLAGS
	INC TEMP_L
	INC FLAGS
	MOV TEMP,FLAGS
	MCALL MUL8X8
	;Прибавляю остальные данные
	LDI ACCUM,_DRV_MAX7219_RAM_SIZE
	ADD ACCUM,TEMP_L
	MCALL C5_RAM_REALLOC
	POP FLAGS
	POP TEMP_L
	;Очищаем память
	LDI XL,_DRV_MAX7219_RAM_SIZE
	ADD XL,ZL
	LDI XH,0x00
	ADD XH,ZH
	MOV LOOP_CNTR,ACCUM
	MCALL RAM_COPY8

	STD Z+_DRV_MAX7219_SPI_ID,TEMP_H
	STD Z+_DRV_MAX7219_QNT,TEMP_L
	STD Z+_DRV_MAX7219_SCANLIMIT,FLAGS

	;Инициализируем порт SCK
	MOV ACCUM,TEMP_EH
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	;Инициализируем порт MISO
	MOV ACCUM,TEMP_EL
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	;Инициализируем порт MOSI
	MOV ACCUM,TEMP_H
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI
	;Инициализируем порт SS
	MOV ACCUM,TEMP_L
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, передача данных
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;TEMP_H - количество чипов,
;TEMP_L - длина данных 1-го устройства,
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH FLAGS

	;Загружаем в Y адрес на выделенный блок памяти
	MCALL C5_RAM_OFFSET
	PUSH_Z
	MOV ZH,YH
	MOV ZL,YL
	POP_Y
	;Умножаем Z на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80

	;Получаю порты
	LDD TEMP_EH,Y+_DRV_SPIM_MOSI_PORT
	LDD TEMP_EL,Y+_DRV_SPIM_CLK_PORT
	LDD FLAGS,Y+_DRV_SPIM_SS_PORT

	;Перебираю все чипы
	MOV LOOP_CNTR,TEMP_H
_DRV_SPIM_CHIP_LOOP:
	PUSH LOOP_CNTR
	MOV ACCUM,FLAGS
	;SS lo,CLK lo
	MCALL PORT_SET_LO
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_LO
	;Получаю байт
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	;Цикл, передача байта
	LDI LOOP_CNTR,0x08
_DRV_SPIM_BYTE_LOOP:
	;CLK lo
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_LO
	;DATA
	LSR TEMP
	MCALL PORT_SET
	;CLK lo
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_LO
	;Следующий бит
	DEC LOOP_CNTR
	BRNE _DRV_SPIM_BYTE_LOOP
	;SS hi
	MOV ACCUM,FLAGS
	MCALL PORT_SET_HI
	;Следующий чип
	POP LOOP_CNTR
	DEC LOOP_CNTR
	BRNE _DRV_SPIM_CHIP_LOOP

	POP FLAGS
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP
	POP ACCUM
	POP_Z
	POP_Y
	RET
