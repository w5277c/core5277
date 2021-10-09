;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер карты памяти (режим SPI)
;-----------------------------------------------------------------------------------------------------------------------
;TODO переписать регистры ACCUM-тип операции, FLAGS-результат
;TODO C5_RAM_OFFSET Z->Y
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_SD_SPI
.else
.set DEF__C5_DRV_SD_SPI = 1
.message "Included driver SD Card SPI mode v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/drivers/sd/sd_reset.inc"
.include	"./core/drivers/sd/sd_get_status.inc"
.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_r3r7.inc"
.include	"./core/time32_mark.inc"
.include	"./core/time32_delta.inc"
.include	"./core/wait_2ms.inc"
.include	"./conv/crc7_730.inc"
.include	"./conv/crc8_block.inc"

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/io/out_char.inc"
		.include	"./core/io/out_bytes.inc"
		.include	"./core/io/out_cr.inc"
_DRV_SD_LOG_STR_SD_CARD_VERSIONS:
		.db	"Got Ver1.X SD card",0x00,"Got Ver2.00 or later SC SD card",0x00,"Got Ver2.00 or later HC/EC SD card",0x00
		_DRV_SD_LOG_STR_BLOCK:
		.db "<BLK:",0x00
	.endif
.endif

;TODO перейти на внешнюю реализацию spi

	;---OPERATIONS---
	.EQU	DRV_SD_OP_RESET									= 0x00;Перевод SD в изначальное состояние
	.EQU	DRV_SD_OP_INIT										= 0x01;Инициализация
	.EQU	DRV_SD_OP_GET_OCR									= 0x02;Получить OCR
	.EQU	DRV_SD_OP_GET_CSD									= 0x03;Получить CSD
	.EQU	DRV_SD_OP_GET_CID									= 0x04;Получить CID
	.EQU	DRV_SD_OP_READ_BLOCK								= 0x05;Считать блок
	.EQU	DRV_SD_OP_READ_DUMMY_BLOCK						= 0x06;Фиктивное чтение блока
	.EQU	DRV_SD_OP_WRITE_BLOCK							= 0x07;Записать блок
	.EQU	DRV_SD_OP_WRITE_DUMMY_BLOCK					= 0x08;Фиктивная запись блока
	.EQU	DRV_SD_OP_ERASE_BLOCKS							= 0x09;Очистка блоков
	.EQU	DRV_SD_OP_GET_STATUS								= 0x0a;Чтение статуса карты
	.EQU	DRV_SD_OP_GET_OP_TIME							= 0x0b;Получить время выполнения процедуры
	.EQU	DRV_SD_OP_GEN_CMD									= 0x0c;Чтение/запись данных контроллера?

;---CONSTANTS--------------------------------------------
	.EQU	DRV_SD_VARS_SIZE									= 0x0e;Размер данных под переменные
	.EQU	DRV_SD_BUFFER_SIZE								= 0x40;Размер внутреннего блока для коммуникации (и записи/чтения если поддерживается partial)
	;---OFFSETS---
	.EQU	_DRV_SD_EXT_BUFFER_ADDR							= 0x00;2Б адрес внешнего буффера размером в 512Б
	.EQU	_DRV_SD_VAR_BLOCK_SIZE							= 0x02;2Б размер блока для чтения/записи
	.EQU	_DRV_SD_VAR_TIME									= 0x04;4Б uptime
	.EQU	_DRV_SD_WRITE_CMD_BUFFER						= 0x08;6Б
	.EQU	_DRV_SD_BUFFER										= 0x0e;DRV_SD_BUFFER_SIZE

	.EQU	_DRV_SD_CMD0										= 0x00;Reset
	.EQU	_DRV_SD_CMD8										= 0x08;Send interface condition
	.EQU	_DRV_SD_CMD9										= 0x09;Read CSD
	.EQU	_DRV_SD_CMD10										= 0x0a;Read CID
	.EQU	_DRV_SD_CMD13										= 0x0d;Read status
	.EQU	_DRV_SD_CMD17										= 0x11;Read single block
	.EQU	_DRV_SD_CMD24										= 0x18;Write single block
	.EQU	_DRV_SD_CMD32										= 0x20;First erase block
	.EQU	_DRV_SD_CMD33										= 0x21;Last erase block
	.EQU	_DRV_SD_CMD38										= 0x26;Erases all selected blocks
	.EQU	_DRV_SD_CMD55										= 0x37;Exec application CMD
	.EQU	_DRV_SD_CMD56										= 0x38;General command
	.EQU	_DRV_SD_CMD58										= 0x3a;Read OCR
	.EQU	_DRV_SD_ACMD41										= 0x29;Activate inicialization
	;---DATA-RESPONSE-TOKEN---
	.EQU	_DRV_SD_DR_TOKEN_ACCEPTED						= 0x05
	.EQU	_DRV_SD_DR_TOKEN_CRC_ERROR						= 0x0b
	.EQU	_DRV_SD_DR_TOKEN_WRITE_ERROR					= 0x0d

	;---FLAGS---
	.EQU	DRV_SD_FLAG_VER1									= 0x01;
	.EQU	DRV_SD_FLAG_VER2SC								= 0x02;
	.EQU	DRV_SD_FLAG_VER2HC								= 0x03;
	;---

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
;TEMP_H/L-R2 ответ или
;TEMP_EH/EL/H/L-time32
;--------------------------------------------------------
	PUSH_X
	PUSH_Z

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	LDD XH,Z+_DRV_SD_EXT_BUFFER_ADDR+0x00
	LDD XL,Z+_DRV_SD_EXT_BUFFER_ADDR+0x01
	CPI XH,0x00
	BRNE _DRV_SD_MAIN__CONTINUE
	CPI XL,0x00
	BRNE _DRV_SD_MAIN__CONTINUE
	MOV XH,ZH
	MOV XL,ZL
	ADIW XL,_DRV_SD_BUFFER
_DRV_SD_MAIN__CONTINUE:

	CPI FLAGS,DRV_SD_OP_RESET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_RESET
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_INIT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_INIT
	RJMP _DRV_SD__END
	CPI FLAGS,DRV_SD_OP_GET_STATUS
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_STATUS
	RJMP _DRV_SD__END
.ifdef DEF_DRIVER_SD_GET_CSD
	CPI FLAGS,DRV_SD_OP_GET_CSD
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_CSD
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_GET_CID
	CPI FLAGS,DRV_SD_OP_GET_CID
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_CID
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_GET_OCR
	CPI FLAGS,DRV_SD_OP_GET_OCR
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_OCR
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_READ_BLOCK
	CPI FLAGS,DRV_SD_OP_READ_BLOCK
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_READ_BLOCK
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_READ_DUMMY_BLOCK
	CPI FLAGS,DRV_SD_OP_READ_DUMMY_BLOCK
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_READ_DUMMY_BLOCK
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_WRITE_BLOCK
	CPI FLAGS,DRV_SD_OP_WRITE_BLOCK
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_WRITE_BLOCK
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_WRITE_DUMMY_BLOCK
	CPI FLAGS,DRV_SD_OP_WRITE_DUMMY_BLOCK
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_WRITE_DUMMY_BLOCK
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_ERASE_BLOCKS
	CPI FLAGS,DRV_SD_OP_ERASE_BLOCKS
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_ERASE_BLOCKS
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_GEN_CMD
	CPI FLAGS,DRV_SD_OP_GEN_CMD
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GEN_CMD
	RJMP _DRV_SD__END
.endif
	CPI FLAGS,DRV_SD_OP_GET_OP_TIME
	BRNE PC+0x06
	LDD TEMP_EH,Z+_DRV_SD_VAR_TIME+0x00
	LDD TEMP_EL,Z+_DRV_SD_VAR_TIME+0x01
	LDD TEMP_H,Z+_DRV_SD_VAR_TIME+0x02
	LDD TEMP_L,Z+_DRV_SD_VAR_TIME+0x03
	RJMP _DRV_SD__END
	;...


	LDI ACCUM,0xff
_DRV_SD__END:

	POP_Z
	POP_X
	RET


;--------------------------------------------------------
_DRV_SD_OP_INIT:
;--------------------------------------------------------
;Инициализация карты
;IN: Z-адрес на переменные, X-адрес на буфер
;OUT: ACCUM-R1 ответ(0x80-сбой инициализации), FLAGS-вер-
;сия, TEMP_H/L-размер блока для чтения,
;TEMP_EH/EL-размер блока для записи,
;FLAGS-поддержка partial (0-бит чтение, 1-бит запись),
;ошибка (7 бит)
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Z
	PUSH_Y

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

	MCALL _DRV_SD_OP_RESET
	CPI ACCUM,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__END
	CBI PORTB,SS & 0x0f


	LDI TEMP,_DRV_SD_CMD8
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x01
	ST X+,TEMP
	LDI TEMP,0x52
	ST X+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
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

	LDI TRY_CNTR,0x32
	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_MARK
_DRV_SD_OP_INIT__WAIT:
	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_ACMD41
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x40
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_ACMD
	CPI ACCUM,0x00
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__ACMD41_ERR
	MCALL _DRV_SD_GET_R1
	SBI PORTB,SS & 0x0f
	CPI ACCUM,0x00
	BREQ _DRV_SD_OP_INIT_SD_READY
	CPI ACCUM,0x01
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__END
_DRV_SD_OP_INIT__ACMD41_ERR:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS
	DEC TRY_CNTR
	BRNE _DRV_SD_OP_INIT__WAIT
	LDI ACCUM,0x80
	RJMP _DRV_SD_OP_INIT__END
_DRV_SD_OP_INIT_SD_READY:
	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_DELTA
	CPI FLAGS,DRV_SD_FLAG_VER1
	BREQ _DRV_SD_OP_INIT__END

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD58
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
	LD TEMP,X
	SBRC TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2SC
	SBRS TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2HC
	LDI ACCUM,0x00
_DRV_SD_OP_INIT__END:
	SBI PORTB,SS & 0x0f

	POP_Y
	POP_Z
	POP LOOP_CNTR
	POP TEMP
	POP TRY_CNTR
	RET


;--------------------------------------------------------
_DRV_SD__SEND_ACMD:
;--------------------------------------------------------
;Отправка команды
;IN: X-адрес на команду, TEMP_L-длина команды (без CRC)
;OUT: ACCUM-R1 ответ
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH_Y
	PUSH_X

	ADIW XL,0x10
	LDI TEMP,_DRV_SD_CMD55
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1

	POP_X
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
;IN: X-адрес на команду, TEMP_L-длина команды (без CRC)
;--------------------------------------------------------
	PUSH_X
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_L

	LD TEMP,X
	ORI TEMP,0x40
	ST X,TEMP

	PUSH_X
	PUSH_Y
	LDI_Y CRC7_730
	MOV LOOP_CNTR,TEMP_L
	MCALL CRC8_BLOCK
	POP_Y
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
		MCALL C5_OUT_CHAR
		MOV TEMP,TEMP_L
		MOV YH,XH
		MOV YL,XL
		MCALL C5_OUT_BYTES
		MCALL C5_OUT_CR
		POP_Y
	.endif
.endif
	CBI PORTB,MOSI & 0x0f
	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	MOV LOOP_CNTR,TEMP_L
_DRV_SD__SEND_CMD__LOOP:
	LD TEMP,X+
	MCALL _DRV_SD_BYTE_SEND
	DEC LOOP_CNTR
	BRNE _DRV_SD__SEND_CMD__LOOP
	LDI TEMP,0xff
	MCALL _DRV_SD_BYTE_SEND

	POP TEMP_L
	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_X
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

	;Низкий уровень приводил к боям в логике некотороых SD после записи блока
	SBI PORTB,MOSI & 0x0f

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
;IN: X-адрес на буффер
;TEMP-длина данных
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_X

	MOV LOOP_CNTR,TEMP
_DRV_SD_BYTES_RECV__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	ST X+,TEMP
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTES_RECV__LOOP

	POP_X
	POP LOOP_CNTR
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_SD_WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	RET
.endif
