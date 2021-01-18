;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать прием данных
;TODO объединить с кодом ведомого?

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_set.inc"
.include	"./io/port_get.inc"
.include	"./conv/crc8_maxim.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SD_CLCK_PORT									= 0x00;1b - Порт SCK
	.EQU	_DRV_SD_CMD_PORT									= 0x01;1b - Порт MISO

	.EQU	_DRV_SD_RAM_SIZE									= 0x02;2 байт необходимо выделить

;Драйвер карты памяти (SD)
;--------------------------------------------------------
DRV_SD_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;TEMP_H - порт CLCK,
;TEMP_L - порт CMD,
;--------------------------------------------------------
	PUSH ACCUM

	LDI ACCUM,_DRV_SD_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Z+_DRV_SD_CLCK_PORT,TEMP_H
	STD Z+_DRV_SD_CMD_PORT,TEMP_L

	;Инициализируем порт CLCK
	MOV ACCUM,TEMP_H
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	;Инициализируем порт CMD
	MOV ACCUM,TEMP_L
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	POP ACCUM
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: ACCUM-команда
;TEMP_EH/EL/H/L-аргументы
;OUT: ACCUM-ответ
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSh TEMP_L

	;Загружаем в Z адрес на выделенный блок памяти
	MCALL C5_RAM_OFFSET

	;Инициализируем порт CMD
	PUSH ACCUM
	LDD ACCUM,Z+_DRV_SD_CMD_PORT
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	POP ACCUM

	MOV TEMP,ACCUM
	CLR ACCUM
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC8_MAXIM
	MOV TEMP,TEMP_EH
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC8_MAXIM
	MOV TEMP,TEMP_EL
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC8_MAXIM
	MOV TEMP,TEMP_H
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC8_MAXIM
	MOV TEMP,TEMP_L
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC8_MAXIM
	MOV TEMP,ACCUM
	MCALL _DRV_SD_BYTE_SEND

	LDD ACCUM,Z+_DRV_SD_CMD_PORT
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x01
	MCALL C5_WAIT_2MS
	MCALL _DRV_SD_BYTE_RECV

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_DRV_SD_BYTE_SEND:
;--------------------------------------------------------
;Передача байта
;IN: Z-выделенная память, TEMP-значение
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH TEMP

	LDI LOOP_CNTR,0x08
_DRV_SD_BYTE_SEND__LOOP:
	LDD ACCUM,Z+_DRV_SD_CLCK_PORT
	MCALL PORT_SET_LO
	MCALL _DRV_SD_WAIT
	LSL TEMP
	LDD ACCUM,Z+_DRV_SD_CMD_PORT
	MCALL PORT_SET
	LDD ACCUM,Z+_DRV_SD_CLCK_PORT
	MCALL PORT_SET_HI
	MCALL _DRV_SD_WAIT
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_SEND__LOOP

	POP TEMP
	POP LOOP_CNTR
	POP ACCUM
	RET


;--------------------------------------------------------
_DRV_SD_BYTE_RECV:
;--------------------------------------------------------
;Передача байта
;IN: Z-выделенная память
;OUT: TEMP-значение
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,0x08
_DRV_SD_BYTE_RECV__LOOP:
	LDD ACCUM,Z+_DRV_SD_CLCK_PORT
	MCALL PORT_SET_LO
	MCALL _DRV_SD_WAIT
	LDD ACCUM,Z+_DRV_SD_CLCK_PORT
	MCALL PORT_SET_HI
	MCALL _DRV_SD_WAIT
	LDD ACCUM,Z+_DRV_SD_CMD_PORT
	MCALL PORT_GET
	ROR TEMP
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_RECV__LOOP

	POP LOOP_CNTR
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,40 ;~20us
	MCALL C5_WAIT_500NS
	POP TEMP
	RET
