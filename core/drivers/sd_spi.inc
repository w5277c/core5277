;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер карты памяти (режим SPI)
;-----------------------------------------------------------------------------------------------------------------------
;TODO C5_RAM_OFFSET Z->Y
;TODO Возвращать TEMP как DRV_RESULT...
;TODO _DRV_SD_OP_INIT заменить бесконечный цикл ожидания SD карты на несколько попыток
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_SD_SPI
.else
.set DEF__C5_DRV_SD_SPI = 1
.message "Included driver SD Card SPI mode v0.1"

.include "./core/drivers/_disk.inc"
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
.include "./mem/ram_copy.inc"

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
	.EQU	DRV_SD_OP_INIT										= DRV_DISK_OP_INIT	;Инициализация
	.EQU	DRV_SD_OP_READ_BLOCK								= DRV_DISK_OP_READ	;Считать блок
	.EQU	DRV_SD_OP_WRITE_BLOCK							= 0x81;Записать блок
	.EQU	DRV_SD_OP_RESET									= 0x82;Перевод SD в изначальное состояние
	.EQU	DRV_SD_OP_GET_OCR									= 0x83;Получить OCR
	.EQU	DRV_SD_OP_GET_CSD									= 0x84;Получить CSD
	.EQU	DRV_SD_OP_GET_CID									= 0x85;Получить CID
	.EQU	DRV_SD_OP_ERASE_BLOCKS							= 0x86;Очистка блоков
	.EQU	DRV_SD_OP_GET_STATUS								= 0x87;Чтение статуса карты
	.EQU	DRV_SD_OP_GEN_CMD									= 0x88;Чтение/запись данных контроллера?

;---CONSTANTS--------------------------------------------
	.EQU	DRV_SD_VARS_SIZE									= 0x0a;Размер данных под переменные
	.SET	DRV_DISK_BLOCK_SIZE								= 0x200;Размер блока
	;---OFFSETS---
	.EQU	_DRV_SD_EXT_BUFFER_ADDR							= 0x00;2Б адрес внешнего буффера размером в 512Б
	.EQU	_DRV_SD_VAR_BLOCK_SIZE							= 0x02;2Б размер блока для чтения/записи
	.EQU	_DRV_SD_CMD_BUFFER								= 0x04;6Б

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
;IN: X-адрес на буффер в 512 байт
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
	LDI ACCUM,DRV_SD_VARS_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_SD_EXT_BUFFER_ADDR+0x00,XH
	STD Y+_DRV_SD_EXT_BUFFER_ADDR+0x01,XL

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: FLAGS-тип операции
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Y

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	LDD XH,Y+_DRV_SD_EXT_BUFFER_ADDR+0x00
	LDD XL,Y+_DRV_SD_EXT_BUFFER_ADDR+0x01

.ifdef DEF_DRIVER_SD_GET_CID
	CPI FLAGS,DRV_DISK_OP_GET_ID
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_ID
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_GET_CSD
	CPI FLAGS,DRV_DISK_OP_GET_SIZE
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_GET_SIZE
	RJMP _DRV_SD__END
.endif
.ifdef DEF_DRIVER_SD_WRITE_BLOCK
 .ifdef DEF_DRIVER_SD_ERASE_BLOCKS
	CPI FLAGS,DRV_DISK_OP_WRITE
	BRNE PC+0x0a+(_MCALL_SIZE*2)
	PUSH_Z
	LDI_Z 0x0001
	MCALL _DRV_SD_OP_ERASE_BLOCKS
	POP_Z
	CPI TEMP,DRV_RESULT_OK
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_SD_OP_WRITE_BLOCK
	RJMP _DRV_SD__END
 .endif
.endif

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
.ifdef DEF_DRIVER_SD_WRITE_BLOCK
	CPI FLAGS,DRV_SD_OP_WRITE_BLOCK
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_SD_OP_WRITE_BLOCK
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
	;...
	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_SD__END:

	POP_Y
	POP_X
	RET


;--------------------------------------------------------
_DRV_SD_OP_INIT:
;--------------------------------------------------------
;Инициализация карты
;IN: Y-адрес на переменные, X-адрес на буфер
;OUT: TEMP-DRV_RESULT_...,
;FLAGS-версия
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH ACCUM
	PUSH LOOP_CNTR
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
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_SD_OP_INIT__ERROR
	CBI PORTB,SS & 0x0f

	LDI TEMP,_DRV_SD_CMD8
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	LDI TEMP,0x01
	ST X+,TEMP
	LDI TEMP,0x52
	ST X+,TEMP
	SBIW XL,0x05
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
	RJMP _DRV_SD_OP_INIT__ERROR

	LDI TRY_CNTR,0x32
_DRV_SD_OP_INIT__WAIT:
	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_ACMD41
	ST X+,TEMP
	LDI TEMP,0x40
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	SBIW XL,0x05
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
	RJMP _DRV_SD_OP_INIT__ERROR
_DRV_SD_OP_INIT__ACMD41_ERR:
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL C5_WAIT_2MS
	POP TEMP_L
	POP TEMP_H
	DEC TRY_CNTR
	BRNE _DRV_SD_OP_INIT__WAIT
	LDI TEMP,DRV_RESULT_TIMEOUT
	RJMP _DRV_SD_OP_INIT__END
_DRV_SD_OP_INIT_SD_READY:
	CPI FLAGS,DRV_SD_FLAG_VER1
	BREQ _DRV_SD_OP_INIT__OK

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD58
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	SBIW XL,0x05
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
	LD TEMP,X
	SBRC TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2SC
	SBRS TEMP,0x01
	LDI FLAGS,DRV_SD_FLAG_VER2HC
_DRV_SD_OP_INIT__OK:
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_OP_INIT__END
_DRV_SD_OP_INIT__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_INIT__END:
	SBI PORTB,SS & 0x0f

	POP_Y
	POP LOOP_CNTR
	POP ACCUM
	POP TRY_CNTR
	RET

.ifdef DEF_DRIVER_SD_GET_CSD
;--------------------------------------------------------
_DRV_SD_OP_GET_SIZE:
;--------------------------------------------------------
;Считываем размер диска
;IN: X-адрес на буфер,Y-адрес на переменные
;OUT: TEMP-DRV_RESULT_...,TEMP_EH/EL/H/L-размер в блоках
;--------------------------------------------------------
	PUSH_Y

	MCALL _DRV_SD_OP_GET_CSD
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_SD_OP_GET_SIZE__END

	MOVW Y,X
	LDD TEMP,Y+0x00
	SWAP TEMP
	LSR TEMP
	LSR TEMP
	CPI TEMP,0x00
	BREQ _DRV_SD_OP_GET_SIZE__V1
	CPI TEMP,0x01
	BREQ _DRV_SD_OP_GET_SIZE__V2
_DRV_SD_OP_GET_SIZE__V3:
	LDD TEMP_EH,Y+0x06
	ANDI TEMP_EH,0x0f
	LDD TEMP_EL,Y+0x07
	LDD TEMP_H,Y+0x08
	LDD TEMP_L,Y+0x09
	;TODO привести к *512
	RJMP _DRV_SD_OP_GET_SIZE__OK
_DRV_SD_OP_GET_SIZE__V2:
	LDD TEMP_EH,Y+0x07
	ANDI TEMP_EH,0x3f
	LDD TEMP_EL,Y+0x08
	LDD TEMP_H,Y+0x09
	LSL TEMP_EH
	ROL TEMP_EL
	ROL TEMP_H
	LSL TEMP_EH
	ROL TEMP_EL
	ROL TEMP_H
	LDI TEMP_L,0x00
	RJMP _DRV_SD_OP_GET_SIZE__OK
_DRV_SD_OP_GET_SIZE__V1:
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDD TEMP_H,Y+0x06
	LSL TEMP_H
	LSL TEMP_H
	ANDI TEMP_H,0x0c
	LDD TEMP_L,Y+0x07
	LDD TEMP,Y+0x08
	LSL TEMP
	ROL TEMP_L
	ROL TEMP_H
	LSL TEMP
	ROL TEMP_L
	ROL TEMP_H
	;TODO привести к *512
_DRV_SD_OP_GET_SIZE__OK:
	LDI TEMP,DRV_RESULT_OK
_DRV_SD_OP_GET_SIZE__END:

	POP_Y
	RET
.endif

.ifdef DEF_DRIVER_SD_GET_CID
;--------------------------------------------------------
_DRV_SD_OP_GET_ID:
;--------------------------------------------------------
;Считываем ID диска
;IN: X-адрес на буфер,Y-адрес на переменные
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH LOOP_CNTR

	MCALL _DRV_SD_OP_GET_CID
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_SD_OP_GET_ID__END

	MOVW ZL,XL
	ADIW XL,0x03
	LDI LOOP_CNTR,0x05
	MCALL RAM_COPY
	STD Z+0x05,C0x00
	LDI TEMP,DRV_RESULT_OK
_DRV_SD_OP_GET_ID__END:

	POP LOOP_CNTR
	POP_Z
	POP_X
	RET
.endif

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
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	SBIW XL,0x05
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
	PUSH_Z
	LDI_Z CRC7_730
	MOV LOOP_CNTR,TEMP_L
	LDI ACCUM,0x00
	MCALL CRC8_BLOCK
	POP_Z
	ADD XL,TEMP_L
	ADC XH,C0x00
	ORI ACCUM,0x01
	ST X,ACCUM
	INC TEMP_L
	POP_X

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Z
		LDI TEMP,'>'
		MCALL C5_OUT_CHAR
		MOV TEMP,TEMP_L
		MOVW ZL,XL
		MCALL C5_OUT_BYTES
		MCALL C5_OUT_CR
		POP_Z
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
	;MCALL _DRV_SD_WAIT
	SBI PORTB,SCK & 0x0f
	;MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
	DEC LOOP_CNTR
	BRNE _DRV_SD_BYTE_SEND__LOOP

	CBI PORTB,SCK & 0x0f
	;MCALL _DRV_SD_WAIT

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
	;MCALL _DRV_SD_WAIT
	CBI PORTB,SCK & 0x0f
	;MCALL _DRV_SD_WAIT
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
.endif
