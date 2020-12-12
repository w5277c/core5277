;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.11.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать прием данных

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_1s.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_set.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SPI_MS_SCK_PORT								= 0x00;1b - Порт SCK
	.EQU	_DRV_SPI_MS_MISO_PORT							= 0x01;1b - Порт MISO
	.EQU	_DRV_SPI_MS_MOSI_PORT							= 0x02;1b - Порт MOSI
	.EQU	_DRV_SPI_MS_SS_PORT								= 0x03;1b - Порт SS
	.EQU	_DRV_SPI_MS_WAIT_TIME							= 0x04;1b - Время выдерживания паузы (0x00 - не использовать)

	.EQU	_DRV_SPI_MS_RAM_SIZE								= 0x05;5 байт необходимо выделить

;Драйвер шины SPI (Master,Software)
;--------------------------------------------------------
DRV_SPI_MS_INIT:
;--------------------------------------------------------
;Инициализация драйвера в режиме ведущего(MASTER)
;IN: TEMP_EH - тактовый выход(SCK),
;TEMP_EL - вход ведущего (MISO),
;TEMP_H - выход ведущего (MOSI),
;TEMP_L - выбор чипа (SS)
;FLAGS - время выдерживания паузы
;--------------------------------------------------------
	LDI ACCUM,_DRV_SPI_MS_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Z+_DRV_SPI_MS_SCK_PORT,TEMP_EH
	STD Z+_DRV_SPI_MS_MISO_PORT,TEMP_EL
	STD Z+_DRV_SPI_MS_MOSI_PORT,TEMP_H
	STD Z+_DRV_SPI_MS_SS_PORT,TEMP_L
	STD Z+_DRV_SPI_MS_WAIT_TIME,FLAGS

	;Инициализируем порт SCK
	MOV ACCUM,TEMP_EH
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
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
;TEMP_L - длина данных.
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
	PUSH TRY_CNTR

	MOV TRY_CNTR,ACCUM
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
	LDD TEMP_EH,Y+_DRV_SPI_MS_MOSI_PORT
	LDD TEMP_EL,Y+_DRV_SPI_MS_SCK_PORT
	LDD TEMP_H,Y+_DRV_SPI_MS_SS_PORT

	;CLK lo
	MOV ACCUM,TEMP_EL
	MCALL PORT_SET_LO
	;SS lo
	MOV ACCUM,TEMP_H
	MCALL PORT_SET_LO
	MOV LOOP_CNTR,TEMP_L
__DRV_SPI_MS_BYTES_LOOP:
		PUSH LOOP_CNTR
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
_DRV_SPI_MS_BYTE_LOOP:
			;CLK lo
			MOV ACCUM,TEMP_EL
			MCALL PORT_SET_LO
			;DATA
			LSL TEMP
			MCALL PORT_SET
			;Выдерживаем паузу
			PUSH TEMP
			LDD TEMP,Z+_DRV_SPI_MS_WAIT_TIME
			CPI TEMP,0x00
			BREQ PC+0x03
			MCALL C5_WAIT_500NS
			;CLK hi
			MOV ACCUM,TEMP_EL
			MCALL PORT_SET_HI
			CPI TEMP,0x00
			BREQ PC+0x03
			MCALL C5_WAIT_500NS
			POP TEMP
			;Следующий бит
			DEC LOOP_CNTR
		BRNE _DRV_SPI_MS_BYTE_LOOP
		POP LOOP_CNTR
		DEC LOOP_CNTR
	BRNE __DRV_SPI_MS_BYTES_LOOP
	;SS hi
	MOV ACCUM,TEMP_H
	MCALL PORT_SET_HI

	POP TRY_CNTR
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
