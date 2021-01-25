;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/ram/ram_extend.inc"
.include	"./core/ram/ram_reduce.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc7_730.inc"
.include	"./conv/crc_block_x8.inc"
.ifdef LOGGING_PORT
	.include	"./core/log/log_byte.inc"
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_strn.inc"
	.include	"./core/log/log_numx8.inc"
	.include	"./core/log/log_numx16.inc"

_DRV_SD_LOG_STR_VER1:
	.db	"Ver1.X SD card",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_VER2_SC:
	.db	"Ver2.00 or later SC SD card",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_VER2_HC:
	.db	"Ver2.00 or later HC/EC SD card",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_MID:
	.db "MID:0x",0x00
_DRV_SD_LOG_STR_OID:
	.db "OID:",0x00
_DRV_SD_LOG_STR_PNM:
	.db "PNM:",0x00
_DRV_SD_LOG_STR_PRV:
	.db "PRV:",0x00
_DRV_SD_LOG_STR_PSN:
	.db "PSN:",0x00
_DRV_SD_LOG_STR_MDT:
	.db "MDT:",0x00
.endif

	;---OPERATIONS---
	.EQU	DRV_SD_OP_INIT										= 0x00;Инициализируем карту памяти


;---CONSTANTS--------------------------------------------
	.EQU	_DRV_SD_CMD0										= 0x00;
	.EQU	_DRV_SD_CMD1										= 0x01;
	.EQU	_DRV_SD_CMD2										= 0x02;
	.EQU	_DRV_SD_CMD5										= 0x05;
	.EQU	_DRV_SD_CMD8										= 0x08;
	.EQU	_DRV_SD_CMD9										= 0x09;
	.EQU	_DRV_SD_CMD10										= 0x0a;
	.EQU	_DRV_SD_CMD55										= 0x37;
	.EQU	_DRV_SD_CMD58										= 0x3a;
	.EQU	_DRV_SD_ACMD41										= 0x29;
	;---FLAGS---
	.EQU	_DRV_SD_FLAG_VER1									= 0x01;
	.EQU	_DRV_SD_FLAG_VER2SC								= 0x02;
	.EQU	_DRV_SD_FLAG_VER2HC								= 0x03;


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
	PUSH FLAGS
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Z

	CLR FLAGS

	CBI PORTB,SCK & 0x0f
	SBI DDRB,MOSI & 0x0f
	SBI PORTB,SS & 0x0f

	SBI PORTB,MOSI & 0x0f
	LDI LOOP_CNTR,80
_DRV_SD_OP_INIT__LOOP:
	SBI PORTB,SCK & 0x0f
	NOP
	NOP
	NOP
	CBI PORTB,SCK &0x0f
	DEC LOOP_CNTR
	BRNE _DRV_SD_OP_INIT__LOOP

	CBI PORTB,SS & 0x0f

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
	LDI TEMP_L,0x01
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f
	LDD TEMP,Z+0x00
	CPI TEMP,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__FAIL
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
	LDI TEMP_L,0x08
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f
	LDD TEMP,Z+0x00
	CPI TEMP,0x01
	BREQ _DRV_SD_OP_INIT__NOT_V1
	CPI TEMP,0x05
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__FAIL
	LDI FLAGS,_DRV_SD_FLAG_VER1

_DRV_SD_OP_INIT__NOT_V1:
_DRV_SD_OP_INIT__WAIT:
	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_ACMD41
	STD Z+0x00,TEMP
	LDI TEMP,0x40
	STD Z+0x01,TEMP
	LDI TEMP,0x00
	STD Z+0x02,TEMP
	LDI TEMP,0x00
	STD Z+0x03,TEMP
	LDI TEMP,0x00
	STD Z+0x04,TEMP
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_ACMD
	LDI TEMP_L,0x06
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f
	LDD TEMP,Z+0x00
	CPI TEMP,0x00
	BREQ _DRV_SD_OP_INIT_SD_READY
	CPI TEMP,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__FAIL
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x10
	MCALL C5_WAIT_2MS
	RJMP _DRV_SD_OP_INIT__WAIT

_DRV_SD_OP_INIT_SD_READY:
	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD58
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
	LDI TEMP_L,0x11
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD9
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
	LDI TEMP_L,0x11
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD10
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
	LDI TEMP_L,0x11
	MCALL _DRV_SD_GET_RESP
	SBI PORTB,SS & 0x0f

	LDI XH,0x00
	LDI XL,0x01
	ADD XL,ZL
	ADC XH,ZH
	MCALL _DRV_SD_LOG_CID





_DRV_SD_OP_INIT__DONE:
;	LDI TEMP, 'd'
;	MCALL C5_LOG_CHAR
	RJMP _DRV_SD_OP_INIT__END
_DRV_SD_OP_INIT__FAIL:
;	LDI TEMP, 'f'
;	MCALL C5_LOG_CHAR
_DRV_SD_OP_INIT__END:
SBI PORTB,SS & 0x0f

	POP_Z
	POP LOOP_CNTR
	POP TEMP
	POP FLAGS
	RET




;--------------------------------------------------------
_DRV_SD__SEND_ACMD:
;--------------------------------------------------------
;Отправка команды
;IN: Z-адрес на команду, TEMP_L-длина команды (без CRC)
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH_Z

	LDI ACCUM,0x10
	MCALL C5_RAM_EXTEND

	LDI TEMP,_DRV_SD_CMD55
	STD Y+0x00,TEMP
	LDI TEMP,0x00
	STD Y+0x01,TEMP
	LDI TEMP,0x00
	STD Y+0x02,TEMP
	LDI TEMP,0x00
	STD Y+0x03,TEMP
	LDI TEMP,0x00
	STD Y+0x04,TEMP
	LDI TEMP_L,0x05
	MOV ZH,YH
	MOV ZL,YL
	MCALL _DRV_SD__SEND_CMD
	LDI TEMP_L,0x01
	MCALL _DRV_SD_GET_RESP
	LDI ACCUM,0x10
	MCALL C5_RAM_REDUCE

	POP_Z
	POP TEMP_L
	MJMP _DRV_SD__SEND_CMD

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
	PUSH TEMP_L


	LD TEMP,Z
	ORI TEMP,0x40
	ST Z,TEMP

	PUSH_X
	MOV XH,ZH
	MOV XL,ZL
	LDI_Y CRC7_730
	MOV LOOP_CNTR,TEMP_L
	MCALL CRC_BLOCK_X8
	ADD XL,TEMP_L
	CLR LOOP_CNTR
	ADC XH,LOOP_CNTR
	ORI ACCUM,0x01
	ST X,ACCUM
	INC TEMP_L
	POP_X

.ifdef LOGGING_PORT
	PUSH_Y
	LDI TEMP,'>'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_L
	MOV YH,ZH
	MOV YL,ZL
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR
	POP_Y
.endif

	CBI PORTB,MOSI & 0x0f
	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	MOV LOOP_CNTR,TEMP_L
_DRV_SD__SEND_CMD__LOOP:
	LD TEMP,Z+
	MCALL _DRV_SD_BYTE_SEND
	DEC LOOP_CNTR
	BRNE _DRV_SD__SEND_CMD__LOOP
	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	POP TEMP_L
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
	MCALL _DRV_SD_WAIT
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
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

;	CBI DDRB,MOSI & 0x0f
;	SBI PORTB,MOSI & 0x0f

	LDI TRY_CNTR,0x10
;	INC TEMP_L
	MOV TEMP_H,TEMP_L
_DRV_SD_GET_RESP__BYTE_LOOP:
	LDI LOOP_CNTR,0x08
_DRV_SD_GET_RESP__BIT_LOOP:
	LSL ACCUM
	SBIC PINB,MISO & 0x0f
	ORI ACCUM,0x01
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
;	CP TEMP_H,TEMP_L
;	BRNE _DRV_SD_GET_RESP__BIT_NEXT
;	CPI LOOP_CNTR,0x08
;	BRNE _DRV_SD_GET_RESP__BIT_NEXT
;	SBRS ACCUM,0x00
;	RJMP _DRV_SD_GET_RESP__BIT_NEXT
;	DEC TRY_CNTR
;	BRNE _DRV_SD_GET_RESP__BIT_LOOP
;	LDI ACCUM,0xff
;	STD Z+0x00,ACCUM
;	SBI DDRB,MOSI & 0x0f
;	CBI PORTB,MOSI & 0x0f
;	RJMP _DRV_SD_GET_RESP__END
_DRV_SD_GET_RESP__BIT_NEXT:
	DEC LOOP_CNTR
	BRNE _DRV_SD_GET_RESP__BIT_LOOP
	ST Z+,ACCUM
	DEC TEMP_H
	BRNE _DRV_SD_GET_RESP__BYTE_LOOP

;	SBI DDRB,MOSI & 0x0f
;	SBI PORTB,MOSI & 0x0f

	POP_Z
	PUSH_Z

; TODO получаю неожиданный ответ 0x37 при CMD55
;	MOV XH,ZH
;	MOV XL,ZL
;	MOV TEMP,ACCUM
;	LDI_Y CRC7_730
;	MOV LOOP_CNTR,TEMP_L
;	DEC LOOP_CNTR
;	MCALL CRC_BLOCK_X8
;	INC ACCUM
;	CP TEMP,ACCUM
;	BREQ _DRV_SD_GET_RESP__END
;_DRV_SD_GET_RESP__ERROR:
;	LDI TEMP,0xff
;	STD Z+0x00,TEMP
;	RJMP _DRV_SD_GET_RESP__END
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
_DRV_SD_LOG_CID:
;Логирование CID
;IN: X-адрес на блок данных
;--------------------------------------------------------
.ifdef LOGGING_PORT
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	MOV ZH,XH
	MOV ZL,XL
	LDI_Y _DRV_SD_LOG_STR_MID|0x8000
	MCALL C5_LOG_STR
	LDD TEMP,Z+0x00
	MCALL C5_LOG_BYTE
	MCALL C5_LOG_CR
	LDI_Y _DRV_SD_LOG_STR_OID|0x8000
	MCALL C5_LOG_STR
	LDD TEMP,Z+0x01
	MCALL C5_LOG_CHAR
	LDD TEMP,Z+0x02
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CR
	LDI_Y _DRV_SD_LOG_STR_PNM|0x8000
	MCALL C5_LOG_STR
	LDI YL,0x03
	LDI YH,0x00
	ADD YL,ZL
	ADC YH,ZH
	LDI TEMP,0x05
	MCALL C5_LOG_STRN
	MCALL C5_LOG_CR
	LDI_Y _DRV_SD_LOG_STR_PRV|0x8000
	MCALL C5_LOG_STR
	LDD TEMP,Z+0x08
	MCALL C5_LOG_BYTE
	MCALL C5_LOG_CR
	LDI_Y _DRV_SD_LOG_STR_PSN|0x8000
	MCALL C5_LOG_STR
	LDI YL,0x09
	LDI YH,0x00
	ADD YL,ZL
	ADC YH,ZH
	LDI TEMP,0x04
	MCALL C5_LOG_BYTES
	LDD TEMP_H,Z+0x0d
	LDD TEMP_L,Z+0x0e
	SWAP TEMP_H
	ANDI TEMP_H,0xf0
	MOV TEMP,TEMP_L
	SWAP TEMP
	ANDI TEMP,0x0f
	OR TEMP_H,TEMP
	ANDI TEMP_L,0x0f
	MCALL C5_LOG_NUMx8
	LDI TEMP,'.'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_H
	LDI TEMP_H,high(2000)
	LDI TEMP_L,low(2000)
	ADD TEMP_L,TEMP
	CLR TEMP
	ADC TEMP_H,TEMP
	MCALL C5_LOG_NUMX8
	MCALL C5_LOG_CR

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Y
	POP_Z
.endif
	RET


;--------------------------------------------------------
_DRV_SD_LOG_OCR:
;Логирование OCR
;IN: X-адрес на блок данных
;--------------------------------------------------------
.ifdef LOGGING_PORT
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L


	MOV ZH,XH
	MOV ZL,XL
	;TODO


	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Y
	POP_Z
.endif
	RET







;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	RET
