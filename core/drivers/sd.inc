;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO реализовать прием данных
;TODO объединить с кодом ведомого?

.include	"./core/wait_2ms.inc"
.include	"./conv/crc7_730.inc"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_SD_CMD_INIT									= 0xff;
	.EQU	DRV_SD_CMD0											= 0x00;
	.EQU	DRV_SD_CMD1											= 0x01;
	.EQU	DRV_SD_CMD5											= 0x05;
	.EQU	DRV_SD_CMD8											= 0x08;
	.EQU	DRV_SD_CMD41										= 0x69;


	.EQU	_DRV_SD_RAM_SIZE									= 0x00;0 байт необходимо выделить

;Драйвер карты памяти (режим SPI)
;--------------------------------------------------------
DRV_SD_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;--------------------------------------------------------
	;Инициализируем порт CLCK
	CBI PORTB,SCK & 0x0f
	SBI DDRB,SCK & 0x0f
	;Инициализируем порт MOSI
	CBI PORTB,MOSI & 0x0f
	SBI DDRB,MOSI & 0x0f
	;Инициализируем порт MISO
	SBI PORTB,MISO & 0x0f
	CBI DDRB,MISO & 0x0f
	;Инициализируем порт SS
	SBI PORTB,SS & 0x0f
	SBI DDRB,SS & 0x0f

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: ACCUM-команда
;TEMP_EH/EL/H/L-аргументы
;OUT: ACCUM-ответ
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	CBI PORTB,SCK & 0x0f
	CBI PORTB,SS & 0x0f
	CBI PORTB,MOSI & 0x0f

	CPI ACCUM,0xff
	BRNE _DRV_SD__NOT_INIT_MODE
	CBI DDRB,MOSI & 0x0f
	LDI LOOP_CNTR,80
_DRV_SD__INIT_LOOP:
	CBI PORTB,SCK & 0x0f
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	SBI PORTB,SCK &0x0f
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	DEC LOOP_CNTR
	BRNE _DRV_SD__INIT_LOOP
	SBI DDRB,MOSI & 0x0f
	RJMP _DRV_SD__END

_DRV_SD__NOT_INIT_MODE:


	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	MOV TEMP,ACCUM
	CLR ACCUM
	ORI TEMP,0x40
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	MOV TEMP,TEMP_EH
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	MOV TEMP,TEMP_EL
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	MOV TEMP,TEMP_H
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	MOV TEMP,TEMP_L
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	MOV TEMP,ACCUM
	ORI TEMP,0x01
	MCALL _DRV_SD_BYTE_SEND

	CBI DDRB,MOSI & 0x0f
	CBI PORTB,MOSI & 0x0f

	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	LDI LOOP_CNTR,0x40
_DRV_SD__WAIT_RESP_LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP PC+0x03
	DEC LOOP_CNTR
	BRNE _DRV_SD__WAIT_RESP_LOOP

	SBI DDRB,MOSI & 0x0f

_DRV_SD__END:
	;Отключаем чип
	SBI PORTB,SS & 0x0f

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_SD_BYTE_SEND:
;--------------------------------------------------------
;Передача байта
;IN: TEMP-значение
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP

	LDI LOOP_CNTR,0x08
_DRV_SD_BYTE_SEND__LOOP:
	SBRS TEMP,0x07
	CBI PORTB,MOSI & 0x0f
	SBRC TEMP,0x07
	SBI PORTB,MOSI & 0x0f
	LSL TEMP
	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_SEND__LOOP

	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT

	POP TEMP
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_SD_BYTE_RECV:
;--------------------------------------------------------
;Передача байта
;OUT: TEMP-значение
;--------------------------------------------------------
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,0x08
_DRV_SD_BYTE_RECV__LOOP:
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	LSR TEMP
	SBIC PINB,MISO & 0x0f
	ORI TEMP,0x80
	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_RECV__LOOP

	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	RET
