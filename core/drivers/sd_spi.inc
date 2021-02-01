;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc7_730.inc"
.include	"./conv/crc8_block.inc"
.include	"./conv/crc16_xmodem.inc"
.include	"./conv/crc16_block.inc"
.include	"./core/drivers/sd/sd_csd_block_info.inc"

.ifdef LOGGING_PORT
	.include	"./core/log/log_byte.inc"
	.include	"./core/log/log_cr.inc"
	.include	"./core/log/log_str_el.inc"

_DRV_SD_LOG_STR_SD_CARD_VERSIONS:
	.db	"Got Ver1.X SD card",0x00,"Got Ver2.00 or later SC SD card",0x00,"Got Ver2.00 or later HC/EC SD card",0x00
_DRV_SD_LOG_STR_DATA_ERROR_TOKEN:
	.db	"Got data error token:",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_DATA_ERROR_TOKENS:
	.db	"Error",0x00,"CC Error",0x00,"Card ECC Failed",0x00,"Out of range",0x00
_DRV_SD_LOG_STR_R1:
	.db "<R1:",0x00
_DRV_SD_LOG_STR_R3R7:
	.db "<R3/R7:",0x00
_DRV_SD_LOG_STR_BLOCK:
	.db "<BLK:",0x00
.endif

;TODO не использовать внутренний буфер если выделен внешний
;TODO учесть в коде смещение(выделенная память) из-за добавленных переменных
;TODO перейти на внешнюю реализацию spi

	;---OPERATIONS---
	.EQU	DRV_SD_OP_INIT										= 0x00;Инициализация
	.EQU	DRV_SD_OP_GET_OCR									= 0x01;Получить OCR
	.EQU	DRV_SD_OP_GET_CSD									= 0x02;Получить CSD
	.EQU	DRV_SD_OP_GET_CID									= 0x03;Получить CID
	.EQU	DRV_SD_OP_READ_BLOCK								= 0x04;Считать блок
	.EQU	DRV_SD_OP_WRITE_BLOCK							= 0x05;Записать блок

;---CONSTANTS--------------------------------------------
	.EQU	DRV_SD_VARS_SIZE									= 0x0e;Размер данных под переменные
	.EQU	DRV_SD_BUFFER_SIZE								= 0x40;Размер внутреннего блока для коммуникации (и записи/чтения если поддерживается partial)
	;---OFFSETS---
	.EQU	_DRV_SD_EXT_BUFFER_ADDR							= 0x00;2Б адрес внешнего буффера размером в 512Б
	.EQU	_DRV_SD_VAR_BLOCK_SIZE							= 0x02;2Б размер блока для чтения/записи
	.EQU	_DRV_SD_VAR_UPTIME_START						= 0x04;5Б uptime
	.EQU	_DRV_SD_VAR_UPTIME_END							= 0x09;5Б uptime
	.EQU	_DRV_SD_BUFFER										= _DRV_SD_EXT_BUFFER_ADDR+DRV_SD_VARS_SIZE;

	.EQU	_DRV_SD_CMD0										= 0x00;
	.EQU	_DRV_SD_CMD1										= 0x01;
	.EQU	_DRV_SD_CMD2										= 0x02;
	.EQU	_DRV_SD_CMD5										= 0x05;
	.EQU	_DRV_SD_CMD8										= 0x08;
	.EQU	_DRV_SD_CMD9										= 0x09;
	.EQU	_DRV_SD_CMD10										= 0x0a;
	.EQU	_DRV_SD_CMD17										= 0x11;
	.EQU	_DRV_SD_CMD24										= 0x18;
	.EQU	_DRV_SD_CMD55										= 0x37;
	.EQU	_DRV_SD_CMD58										= 0x3a;
	.EQU	_DRV_SD_ACMD41										= 0x29;
	;---FLAGS---
	.EQU	DRV_SD_FLAG_VER1									= 0x01;
	.EQU	DRV_SD_FLAG_VER2SC								= 0x02;
	.EQU	DRV_SD_FLAG_VER2HC								= 0x03;
	;---

;Драйвер карты памяти (режим SPI)
;--------------------------------------------------------
DRV_SD_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;IN: Y-адрес на буффер в 512 байт, 0x0000 - не использо-
;вать буфер (возможно, если карта поддерживает partial
;для чтения и записи)
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
	LDI TEMP,DRV_SD_BUFFER_SIZE
	LDI ACCUM,DRV_SD_VARS_SIZE
	CPI YH,0x00
	BRNE PC+0x04
	CPI YL,0x00
	BRNE PC+0x02
	ADD ACCUM,TEMP
	MCALL C5_RAM_REALLOC

	STD Z+_DRV_SD_EXT_BUFFER_ADDR+0x00,YH
	STD Z+_DRV_SD_EXT_BUFFER_ADDR+0x01,YL

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: FLAGS - тип операции
;OUT: ACCUM - R1 ответ или
;TEMP_H/L-R2 ответ
;--------------------------------------------------------
	PUSH_Z

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	PUSH TEMP_H
	PUSH TEMP_L
	LDD TEMP_H,Z+_DRV_SD_EXT_BUFFER_ADDR+0x00
	LDD TEMP_L,Z+_DRV_SD_EXT_BUFFER_ADDR+0x01
	CPI TEMP_H,0x00
	BRNE _DRV_SD_MAIN__GLOBAL_BUFFER
	CPI TEMP_L,0x00
	BREQ _DRV_SD_MAIN__LOCAL_BUFFER
_DRV_SD_MAIN__GLOBAL_BUFFER:
	PUSH TEMP_H
	PUSH TEMP_L
;	LDD TEMP_EH,Z+_DRV_SD_VAR_BLOCK_SIZE+0x00
;	LDD TEMP_EL,Z+_DRV_SD_VAR_BLOCK_SIZE+0x01
	POP ZL
	POP ZH
	RJMP _DRV_SD_MAIN__CONTINUE
_DRV_SD_MAIN__LOCAL_BUFFER:
;	LDD TEMP_EH,Z+_DRV_SD_VAR_BLOCK_SIZE+0x00
;	LDD TEMP_EL,Z+_DRV_SD_VAR_BLOCK_SIZE+0x01
	ADIW ZL,_DRV_SD_BUFFER
_DRV_SD_MAIN__CONTINUE:
	POP TEMP_L
	POP TEMP_H

	CPI FLAGS,DRV_SD_OP_INIT
	BRNE PC+0x04
	MCALL _DRV_SD_OP_INIT
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_GET_CSD
	BRNE PC+0x04
	MCALL _DRV_SD_OP_GET_CSD
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_GET_CID
	BRNE PC+0x04
	MCALL _DRV_SD_OP_GET_CID
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_GET_OCR
	BRNE PC+0x04
	MCALL _DRV_SD_OP_GET_OCR
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_READ_BLOCK
	BRNE PC+0x04
	MCALL _DRV_SD_OP_READ_BLOCK
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
;OUT: ACCUM-R1 ответ, FLAGS - версия
;Y-адрес на данные OCR
;TEMP_H/L - размер блока для чтения,
;TEMP_EH/EL - размер блока для записи,
;FLAGS-поддержка partial (0-бит чтение, 1-бит запись),
;ошибка (7 бит)
;--------------------------------------------------------
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
	MCALL _DRV_SD_GET_R1
	SBI PORTB,SS & 0x0f
	CPI ACCUM,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__END
	CBI PORTB,SS & 0x0f

	LDI TEMP,_DRV_SD_CMD8
	STD Z+0x00,TEMP
	LDI TEMP,0x00
	STD Z+0x01,TEMP
	LDI TEMP,0x00
	STD Z+0x02,TEMP
	LDI TEMP,0x01
	STD Z+0x03,TEMP
	LDI TEMP,0x52
	STD Z+0x04,TEMP
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MOV YH,ZH
	MOV YL,ZL
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
	CPI ACCUM,0x05
	BRNE PC+0x03
	LDI FLAGS,DRV_SD_FLAG_VER1
	RJMP PC+0x05
	ANDI ACCUM,0xfe
	CPI ACCUM,0x00
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__END

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
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_INIT__END
	MCALL _DRV_SD_GET_R1
	LDI TEMP_H,0x00
	LDI TEMP_L,0x10
	MCALL _DRV_SD_GET_BLOCK
	SBI PORTB,SS & 0x0f
	CPI ACCUM,0x00
	BREQ _DRV_SD_OP_INIT_SD_READY
	CPI ACCUM,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__END
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS
	RJMP _DRV_SD_OP_INIT__WAIT

_DRV_SD_OP_INIT_SD_READY:
	CPI FLAGS,DRV_SD_FLAG_VER1
	BREQ _DRV_SD_OP_INIT__CMD58_SKIP

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
	MOV YH,ZH
	MOV YL,ZL
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
	LD TEMP,Z
	SBRC TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2SC
	SBRS TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2HC
_DRV_SD_OP_INIT__CMD58_SKIP:

	MOV TEMP,FLAGS
	MCALL _DRV_SD_OP_GET_CSD
	MCALL DRV_SD_CSD_BLOCK_INFO
	OR FLAGS,TEMP
	SBRC FLAGS,0x07
	LDI ACCUM,0xff

_DRV_SD_OP_INIT__END:
	SBI PORTB,SS & 0x0f

.ifdef LOGGING_PORT
	MCALL C5_LOG_CR
	LDI_Y _DRV_SD_LOG_STR_SD_CARD_VERSIONS|0x8000
	MOV TEMP,FLAGS
	DEC TEMP
	ANDI TEMP,0x03
	MCALL C5_LOG_STR_EL
	MCALL C5_LOG_CR
.endif

	MOV YH,ZH
	MOV YL,ZL

	POP_Z
	POP LOOP_CNTR
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD_OP_GET_OCR:
;--------------------------------------------------------
;Получить OCR
;IN: Z-адрес на выделенную память
;OUT: ACCUM-R1 ответ
;Y-адрес на буффер драйвера (где будет записан OCR)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH_Z

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
	MOV YH,ZH
	MOV YL,ZL
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
	MOV YH,ZH
	MOV YL,ZL
_DRV_SD_OP_GET_OCR__END:

	POP_Z
	POP TEMP_L
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD_OP_GET_CSD:
;--------------------------------------------------------
;Получить CSD
;IN: Z-адрес на выделенную память
;OUT: ACCUM-R1 ответ
;Y-адрес на буффер драйвера (где будет записан CSD)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH_Z

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
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GET_CSD__FAIL
	MOV YH,ZH
	MOV YL,ZL
	LDI TEMP_H,0x00
	LDI TEMP_L,0x10
	MCALL _DRV_SD_GET_BLOCK
	SBI PORTB,SS & 0x0f
	MOV YH,ZH
	MOV YL,ZL
_DRV_SD_OP_GET_CSD__FAIL:
_DRV_SD_OP_GET_CSD__END:
SBI PORTB,SS & 0x0f

	POP_Z
	POP TEMP_L
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD_OP_GET_CID:
;--------------------------------------------------------
;Получить CID
;IN: Z-адрес на выделенную память
;OUT: ACCUM-R1 ответ
;Y-адрес на буффер драйвера (где будет записан CID)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH_Z

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
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GET_CID__FAIL
	MOV YH,ZH
	MOV YL,ZL
	LDI TEMP_H,0x00
	LDI TEMP_L,0x10
	MCALL _DRV_SD_GET_BLOCK
	SBI PORTB,SS & 0x0f
	MOV YH,ZH
	MOV YL,ZL
_DRV_SD_OP_GET_CID__FAIL:
_DRV_SD_OP_GET_CID__END:
	SBI PORTB,SS & 0x0f

	POP_Z
	POP TEMP_L
	POP TEMP
	RET


;--------------------------------------------------------
_DRV_SD_OP_READ_BLOCK:
;--------------------------------------------------------
;Читаем блок данных
;IN: Z-адрес на внешний буфер 512 байт
;TEMP_EH/EL/H/L-номер блока
;OUT: ACCUM-R1 ответ
;Y-адрес на буффер драйвера (где будет записан блок)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD17
	STD Z+0x00,TEMP
	STD Z+0x01,TEMP_EH
	STD Z+0x02,TEMP_EL
	STD Z+0x03,TEMP_H
	STD Z+0x04,TEMP_L
	LDI TEMP_L,0x05

;LDI YL,_DRV_SD_VAR_UPTIME_START
;LDI YH,0x00
;ADD YL,ZL
;MCALL C5_UPTIME_COPY
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
;MCALL C5_UPTIME_COPY
;TODO посчитать дельту времени
;TODO проверить CRC. Проверить статус?

	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_READ_BLOCK__FAIL
	MOV YH,ZH
	MOV YL,ZL
	LDI TEMP_H,high(512)
	LDI TEMP_L,low(512)
	MCALL _DRV_SD_GET_BLOCK
	SBI PORTB,SS & 0x0f
	MOV YH,ZH
	MOV YL,ZL
_DRV_SD_OP_READ_BLOCK__FAIL:
_DRV_SD_OP_READ_BLOCK__END:
	SBI PORTB,SS & 0x0f

	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD_OP_WRITE_BLOCK:
;--------------------------------------------------------
;Пишем блок данных
;IN: Z-адрес на внешний буффер 512 байт
;TEMP_EH/EL/H/L-номер блока
;OUT: ACCUM-R1 ответ
;Y-адрес на буффер драйвера (где будет записан блок)
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD24
	STD Z+0x00,TEMP
	STD Z+0x01,TEMP_EH
	STD Z+0x02,TEMP_EL
	STD Z+0x03,TEMP_H
	STD Z+0x04,TEMP_L
	LDI TEMP_L,0x05

;LDI YL,_DRV_SD_VAR_UPTIME_START
;LDI YH,0x00
;ADD YL,ZL
;MCALL C5_UPTIME_COPY
;TODO посчитать дельту времени
;TODO проверить статус, передать CRC16

	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
;MCALL C5_UPTIME_COPY
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_READ_BLOCK__FAIL
	MOV YH,ZH
	MOV YL,ZL
	LDI TEMP_H,high(512)
	LDI TEMP_L,low(512)
	;MCALL _DRV_SD_SET_BLOCK
	SBI PORTB,SS & 0x0f
	MOV YH,ZH
	MOV YL,ZL
_DRV_SD_OP_WRITE_BLOCK__FAIL:
_DRV_SD_OP_WRITE_BLOCK__END:
	SBI PORTB,SS & 0x0f

	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD__SEND_ACMD:
;--------------------------------------------------------
;Отправка команды
;IN: Z-адрес на команду, TEMP_L-длина команды (без CRC)
;OUT: ACCUM-R1 ответ
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH_Y
	PUSH_Z

	ADIW ZL,0x10
	LDI TEMP,_DRV_SD_CMD55
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
	MOV YH,ZH
	MOV YL,ZL
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1

	POP_Z
	POP_Y
	POP TEMP_L

	ANDI ACCUM,0xfe
	CPI ACCUM,0x00
	BREQ _DRV_SD__SEND_CMD
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
	PUSH TEMP_L


	LD TEMP,Z
	ORI TEMP,0x40
	ST Z,TEMP

	PUSH_X
	MOV XH,ZH
	MOV XL,ZL
	LDI_Y CRC7_730
	MOV LOOP_CNTR,TEMP_L
	MCALL CRC8_BLOCK
	ADD XL,TEMP_L
	CLR LOOP_CNTR
	ADC XH,LOOP_CNTR
	ORI ACCUM,0x01
	ST X,ACCUM
	INC TEMP_L
	POP_X

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
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
_DRV_SD_GET_R1:
;--------------------------------------------------------
;Прием R1
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP

	LDI TRY_CNTR,0x10
_DRV_SD_GET_R1__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R1__DONE
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R1__LOOP
_DRV_SD_GET_R1__DONE:
	MOV ACCUM,TEMP

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Y
		LDI_Y _DRV_SD_LOG_STR_R1|0x8000
		MCALL C5_LOG_STR
		MOV TEMP,ACCUM
		MCALL C5_LOG_BYTE
		MCALL C5_LOG_CR
		POP_Y
	.endif
.endif
	POP TEMP
	POP TRY_CNTR
	RET

;--------------------------------------------------------
_DRV_SD_GET_R3R7:
;--------------------------------------------------------
;Прием R7
;IN: Y-адрес на буфер
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP

	LDI TRY_CNTR,0x10
_DRV_SD_GET_R7__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R7__GOT_R1
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R7__LOOP
_DRV_SD_GET_R7__GOT_R1:
	MOV ACCUM,TEMP

	LDI TEMP,0x04
	MCALL _DRV_SD_BYTES_RECV
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Y
		LDI_Y _DRV_SD_LOG_STR_R3R7|0x8000
		MCALL C5_LOG_STR
		POP_Y
		MOV TEMP,ACCUM
		MCALL C5_LOG_BYTE
		LDI TEMP,0x04
		MCALL C5_LOG_BYTES
		MCALL C5_LOG_CR
	.endif
.endif
	POP TEMP
	POP TRY_CNTR
	RET

;--------------------------------------------------------
_DRV_SD_GET_BLOCK:
;--------------------------------------------------------
;Прием блока данных
;IN: Y-адрес на выделенную память
;TEMP_H,L-максимальная длина данных
;OUT: TEMP_H,L-длина полученных данных
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_Y

	MOV XH,TEMP_H
	MOV XL,TEMP_L
	LDI TRY_CNTR,0x10
_DRV_SD_GET_BLOCK__BEGIN_LOOP:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xfe
	BREQ _DRV_SD_GET_BLOCK__DATA
_DRV_SD_GET_BLOCK__NO_ERROR:
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_BLOCK__BEGIN_LOOP
	RJMP _DRV_SD_GET_BLOCK__FAIL

_DRV_SD_GET_BLOCK__DATA:
	MCALL _DRV_SD_BYTE_RECV
;	CPI TEMP,0xfd
;	BREQ _DRV_SD_GET_BLOCK__DONE
	ST Y+,TEMP
	SUBI XL,0x01
	SBCI XH,0x00
	CPI XH,0x00
	BRNE _DRV_SD_GET_BLOCK__DATA
	CPI XL,0x00
	BRNE _DRV_SD_GET_BLOCK__DATA
_DRV_SD_GET_BLOCK__DONE:
MCALL _DRV_SD_BYTE_RECV
MCALL C5_LOG_BYTE
MCALL _DRV_SD_BYTE_RECV
MCALL C5_LOG_BYTE
	SUB TEMP_L,XL
	SBC TEMP_H,XH

	POP_X
	PUSH_X
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI YH,high(CRC16_XMODEM)
	LDI YL,low(CRC16_XMODEM)
	MCALL CRC16_BLOCK
MOV TEMP,TEMP_EH
COM TEMP
MCALL C5_LOG_BYTE
MOV TEMP,TEMP_EL
COM TEMP
MCALL C5_LOG_BYTE


.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		LDI_Y _DRV_SD_LOG_STR_BLOCK|0x8000
		MCALL C5_LOG_STR
		LDI TEMP,'['
		MCALL C5_LOG_CHAR
		MCALL C5_LOG_WORD
		LDI TEMP,']'
		MCALL C5_LOG_CHAR
		POP_Y
		PUSH_Y
		MCALL C5_LOG_RAMDUMP
	.endif
.endif
	RJMP _DRV_SD_GET_BLOCK__END
_DRV_SD_GET_BLOCK__FAIL:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
_DRV_SD_GET_BLOCK__END:

	POP_Y
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP
	POP TRY_CNTR
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
_DRV_SD_BYTE_RECV:
;--------------------------------------------------------
;Прием байта
;OUT: TEMP-значение
;--------------------------------------------------------
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,0x08
_DRV_SD_BYTE_RECV__LOOP:
	LSL TEMP
	SBIC PINB,MISO & 0x0f
	ORI TEMP,0x01
	SBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
	MCALL _DRV_SD_WAIT
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_RECV__LOOP

	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_SD_BYTES_RECV:
;--------------------------------------------------------
;Прием данных
;IN: Y-адрес на выделенную память
;TEMP-длина данных
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Y

	MOV LOOP_CNTR,TEMP
_DRV_SD_BYTES_RECV__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	ST Y+,TEMP
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTES_RECV__LOOP

	POP_Y
	POP LOOP_CNTR
	POP TEMP
	RET









;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	RET
