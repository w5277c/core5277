;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc7_730.inc"
.include	"./conv/crc_block_x8.inc"


	;---OPERATIONS---
	.EQU	DRV_SD_OP_INIT										= 0x00;Инициализируем карту памяти


;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SD_CMD0										= 0x00;
	.EQU	_DRV_SD_CMD1										= 0x01;
	.EQU	_DRV_SD_CMD5										= 0x05;
	.EQU	_DRV_SD_CMD8										= 0x08;
	.EQU	_DRV_SD_CMD41										= 0x69;


	.EQU	_DRV_SD_RAM_SIZE									= 0x20;32 байта необходимо выделить под буфер

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
	;Инициализируем порт SS (чип отключен)
	SBI PORTB,SS & 0x0f
	SBI DDRB,SS & 0x0f

	;Выделяем память
	LDI ACCUM,_DRV_SD_RAM_SIZE
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: FLAGS - тип операции
;OUT: ACCUM - код функции/ответа(0xff - сбой)
;--------------------------------------------------------
	PUSH_Z

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_SD_OP_INIT
	BRNE PC+0x03
	MCALL _DRV_SD_OP_INIT
	RJMP _DRV_SD__END
	;...

	LDI ACCUM,0xff
_DRV_SD__END:
	POP_Z
	RET


;--------------------------------------------------------
_DRV_SD_OP_INIT:
;--------------------------------------------------------
;Инициализация карты
;OUT: ACCUM - код функции/ответа(0xff - сбой)
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Z

	CBI PORTB,SCK & 0x0f
	SBI PORTB,SS & 0x0f
	SBI DDRB,MOSI & 0x0f

	SBI PORTB,MOSI & 0x0f
	LDI LOOP_CNTR,80
_DRV_SD_OP_INIT__LOOP:
	CBI PORTB,SCK & 0x0f
	NOP
	NOP
	NOP
	SBI PORTB,SCK &0x0f
	DEC LOOP_CNTR
	BRNE _DRV_SD_OP_INIT__LOOP


	LDI TEMP,_DRV_SD_CMD0
	STD Z+0x00,TEMP
	LDI TEMP,0x00
	STD Z+0x01,TEMP
	LDI TEMP,0x00
	STD Z+0x02,TEMP
	LDI TEMP,0x00
	STD Z+0x03,TEMP
	LDI TEMP,0x00
	STD Z+0x04,TEMP
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD

CBI PORTB,SS & 0x0f

	LDI TEMP,_DRV_SD_CMD8											;47:40	b01001000
	STD Z+0x00,TEMP
	LDI TEMP,0x00														;39:32	0x00
	STD Z+0x01,TEMP
	LDI TEMP,0x00														;31:24	0x00
	STD Z+0x02,TEMP
	LDI TEMP,0x01														;23:16	19:16=b0001 (2.7-3.6V)
	STD Z+0x03,TEMP
	LDI TEMP,0xAA														;15-08	0x00
	STD Z+0x04,TEMP
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD

	LDI TEMP_L,0x05
	MCALL _DRV_SD_GET_RESP

SBI PORTB,SS & 0x0f

	POP_Z
	POP LOOP_CNTR
	POP TEMP
	RET



;--------------------------------------------------------
_DRV_SD__SEND_CMD:
;--------------------------------------------------------
;Отправка команды
;IN: Z-адрес на команду, TEMP_L-длина команды (без CRC)
;--------------------------------------------------------
	PUSH_Z
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP

.ifdef LOGGING_PORT
	LDI TEMP,'>'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_L
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
.endif

	CBI PORTB,MOSI & 0x0f
	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	CLR ACCUM
	MOV LOOP_CNTR,TEMP_L
_DRV_SD__SEND_CMD__LOOP:
	LD TEMP,Z+
	CP LOOP_CNTR,TEMP_L
	BRNE PC+0x02
	ORI TEMP,0x40
	MCALL _DRV_SD_BYTE_SEND
	MCALL CRC7_730
	DEC LOOP_CNTR
	BRNE _DRV_SD__SEND_CMD__LOOP
	MOV TEMP,ACCUM
	ORI TEMP,0x01
	MCALL _DRV_SD_BYTE_SEND

	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_Z
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
_DRV_SD_GET_RESP:
;--------------------------------------------------------
;Прием ответа
;IN: Z-адрес на выделенную память
;TEMP_L-длина ожидаемого ответа (без CRC)
;OUT: ---
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH TRY_CNTR
	PUSH_Z

	CBI DDRB,MOSI & 0x0f
	SBI PORTB,MOSI & 0x0f

	LDI TRY_CNTR,0x10
	INC TEMP_L
	MOV TEMP_H,TEMP_L
_DRV_SD_GET_RESP__BYTE_LOOP:
	LDI LOOP_CNTR,0x08
_DRV_SD_GET_RESP__BIT_LOOP:
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	LSL ACCUM
	SBIC PINB,MOSI & 0x0f
	ORI ACCUM,0x01
	CP TEMP_H,TEMP_L
	BRNE _DRV_SD_GET_RESP__BIT_NEXT
	CPI LOOP_CNTR,0x08
	BRNE _DRV_SD_GET_RESP__BIT_NEXT
	SBRS ACCUM,0x00
	RJMP _DRV_SD_GET_RESP__BIT_NEXT
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_RESP__BIT_LOOP
	LDI ACCUM,0xff
	STD Z+0x00,ACCUM
	RJMP _DRV_SD_GET_RESP__END
_DRV_SD_GET_RESP__BIT_NEXT:
	DEC LOOP_CNTR
	BRNE _DRV_SD_GET_RESP__BIT_LOOP
	ST Z+,ACCUM
	DEC TEMP_H
	BRNE _DRV_SD_GET_RESP__BYTE_LOOP


	POP_Z
	PUSH_Z
	MOV XH,ZH
	MOV XL,ZL
	MOV TEMP,ACCUM
	LDI YH,high(CRC7_730)
	LDI YL,low(CRC7_730)
	MOV LOOP_CNTR,TEMP_L
	DEC LOOP_CNTR
	MCALL CRC_BLOCK_X8
	INC ACCUM
	CP TEMP,ACCUM
	BREQ _DRV_SD_GET_RESP__END
_DRV_SD_GET_RESP__ERROR:
	LDI TEMP,0xff
	STD Z+0x00,TEMP
	RJMP _DRV_SD_GET_RESP__END
_DRV_SD_GET_RESP__END:
	POP_Z

.ifdef LOGGING_PORT
	PUSH_Y
	LDI TEMP,'<'
	MCALL C5_LOG_CHAR
	MOV YH,ZH
	MOV YL,ZL
	MOV TEMP,TEMP_L
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
	POP_Y
.endif


	POP TRY_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	POP LOOP_CNTR
	RET





;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	RET
