;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.11.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать прием данных
;TODO объединить с кодом ведомого?

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
	.EQU	_DRV_SPI_MS_CS_PORT								= 0x03;1b - Порт CS
	.EQU	_DRV_SPI_MS_CHIP_QNT								= 0x04;1b - Количество чипов на шине
	.EQU	_DRV_SPI_MS_PACKET_SZIE							= 0x05;1b - Размер пакета в байтах
	.EQU	_DRV_SPI_MS_WAIT_TIME							= 0x06;1b - Время выдерживания паузы (0x00 - не использовать)

	.EQU	_DRV_SPI_MS_RAM_SIZE								= 0x07;7 байт необходимо выделить

;Драйвер шины SPI (Master,Software)
;--------------------------------------------------------
DRV_SPI_MS_INIT:
;--------------------------------------------------------
;Инициализация драйвера в режиме ведущего(MASTER)
;IN: XH - тактовый выход(SCK),
;XL - вход ведущего (MISO),
;YH - выход ведущего (MOSI),
;YL - выход, конец данных (СS)
;TEMP_H - кол-во чипов на шине,
;TEMP_L - размер пакета в байтах,
;ACCUM - время выдерживания паузы
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_SPI_MS_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	STD Z+_DRV_SPI_MS_SCK_PORT,XH
	STD Z+_DRV_SPI_MS_MISO_PORT,XL
	STD Z+_DRV_SPI_MS_MOSI_PORT,YH
	STD Z+_DRV_SPI_MS_CS_PORT,YL
	STD Z+_DRV_SPI_MS_CHIP_QNT,TEMP_H
	STD Z+_DRV_SPI_MS_PACKET_SZIE,TEMP_H
	STD Z+_DRV_SPI_MS_WAIT_TIME,ACCUM

	;Инициализируем порт SCK
	MOV ACCUM,XH
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	;Инициализируем порт MISO
	MOV ACCUM,XL
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	;Инициализируем порт MOSI
	MOV ACCUM,YH
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	;Инициализируем порт CS
	MOV ACCUM,YL
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_HI

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, передача данных
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;TEMP_H - номер чипа(0-...)
;TEMP_L - количество пакетов
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP
	PUSH LOOP_CNTR

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

	LDD ACCUM,Y+_DRV_SPI_MS_CS_PORT
	;Перебираем все пакеты
	MOV LOOP_CNTR,TEMP_L
_DRV_SPI_MS_PACKETS_LOOP:
	MCALL PORT_SET_LO
	MCALL _DRV_SPI_MS_WAIT

	;Перебираем все чипы
	PUSH LOOP_CNTR
	LDD LOOP_CNTR,Y+_DRV_SPI_MS_CHIP_QNT
	DEC LOOP_CNTR
_DRV_SPI_MS_CHIP_LOOP:
	CP TEMP_H,LOOP_CNTR
	BREQ _DRV_SPI_MS_SELECTED_CHIP

	;Если данное устройство не выбрано, то передаем данные пустого пакета
	PUSH LOOP_CNTR
	LDD LOOP_CNTR,Y+_DRV_SPI_MS_PACKET_SZIE
	LDI TEMP,0x00
_DRV_SPI_MS_EMPTY_DATA_LOOP:
	MCALL _DRV_SPI_MS_BYTE
	DEC LOOP_CNTR
	BRNE _DRV_SPI_MS_EMPTY_DATA_LOOP
	POP LOOP_CNTR
	RJMP _DRV_SPI_MS_PACKET_BYTES_END

	;Если данный чип выбран, то передаем реальные данные
_DRV_SPI_MS_SELECTED_CHIP:
	;Перебираем байты пакета
	PUSH LOOP_CNTR
	LDD LOOP_CNTR,Y+_DRV_SPI_MS_PACKET_SZIE
_DRV_SPI_MS_PACKET_BYTES_LOOP:
	;Получаю байт
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	MCALL _DRV_SPI_MS_BYTE
	DEC LOOP_CNTR
	BRNE _DRV_SPI_MS_PACKET_BYTES_LOOP
	POP LOOP_CNTR

_DRV_SPI_MS_PACKET_BYTES_END:
	DEC LOOP_CNTR
	CPI LOOP_CNTR,0xff
	BRNE _DRV_SPI_MS_CHIP_LOOP
	POP LOOP_CNTR

	MCALL PORT_SET_HI
	MCALL _DRV_SPI_MS_WAIT
	DEC LOOP_CNTR
	BRNE _DRV_SPI_MS_PACKETS_LOOP

	POP LOOP_CNTR
	POP TEMP
	POP ACCUM
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_SPI_MS_BYTE:
;--------------------------------------------------------
;Коммуникация (передача и прием байта)
;IN: Y - адрес на выделенную память
;TEMP - Данные
;OUT: ??? TODO
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR
	;Цикл, передача байта
	LDI LOOP_CNTR,0x08
_DRV_SPI_MS_BYTE_LOOP:
	;DATA
	LSL TEMP
	LDD ACCUM,Y+_DRV_SPI_MS_MOSI_PORT
	MCALL PORT_SET
	MCALL _DRV_SPI_MS_WAIT
	;CLK hi
	LDD ACCUM,Y+_DRV_SPI_MS_SCK_PORT
	MCALL PORT_SET_HI
	MCALL _DRV_SPI_MS_WAIT

	;TODO RECIEVE DATA
	;TODO SAVE RECEIVED DATA

	;CLK lo
	MCALL PORT_SET_LO
	;Следующий бит
	DEC LOOP_CNTR
	BRNE _DRV_SPI_MS_BYTE_LOOP
	POP LOOP_CNTR
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_SPI_MS_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;IN: Y - адрес на выделенную память
;TEMP - время паузы(*500нс), не может быть менее 40
;--------------------------------------------------------
	PUSH TEMP
	LDD TEMP,Y+_DRV_SPI_MS_WAIT_TIME
	CPI TEMP,40
	BRCS PC+0x03
	MCALL _C5_WAIT_500NS
	POP TEMP
	RET
