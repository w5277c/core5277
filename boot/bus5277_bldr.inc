;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;21.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_BOOTLADER
.error "Bootloader already included!"
.else
.set DEF_BOOTLADER = 1

.if REPORT_INCLUDES == 0x01
.message "included BLDR Bus5277"
.endif

.ORG BOOT_512W_ADDR
	;---CONSTANTS---
	.SET	ACT_LED_PORT									= 0xff
	.SET	BUS_DR_PORT										= 0xff
	;---
	.SET	EEPROM_MSET_OFFSET							= 0x08						;Смещение на основные настройки
	;---
	.EQU	EEPROM_RESERVED1								= 0x00	;8Б зарезервировано, первая страница, не безопасна
	.SET	EEPROM_DEV_TYPE								= 0x08	;1B тип устройства
	.SET	EEPROM_DEV_SERIAL								= 0x09	;8Б серийного номера
	.SET	EEPROM_PCB_VERSION							= 0x11	;1Б Версия PCB
	.SET	EEPROM_FW_VERSION								= 0x12	;1Б Версия прошивки
	.SET	EEPROM_BUS_ADDR								= 0x13	;1Б Адрес устройства на шине
	.SET	EEPROM_BLDR_ERR_CNTR							= 0x14	;1Б Счетчик ошибок, изначально равен 64, при нуле блокируем работу загрузчика
	.SET	EEPROM_BLDR_ENABLED							= 0x15	;1Б Признак работы бутлоадера(0x52)
	;---выравниваем по размеру страницы 8Б---
.ifdef EEPROM_BLDR_SECURE_DATA_SIZE
	.EQU	EEPROM_BLDR_SECURE_DATA						= 0x18	;Смещение для опциональных данных процедуры расшифровки данных прошивки
	.SET	EEPROM_ESET_OFFSET							= EEPROM_BLDR_SECURE_DATA+EEPROM_BLDR_SECURE_DATA_SIZE	;Смещение доступное для нужд основной программы
.else
	.SET	EEPROM_ESET_OFFSET							= 0x18	;Смещение доступное для нужд основной программы
.endif

	.EQU	_BLDR_UART_BUFFER_LEN						= 0x28	;40B - максимальная длина буфера UART
	.EQU	_BLDR_FLASH_BUFFER							= RAMSTART+0x28;FLASH_PAGESIZE - размер буфера

	.EQU	_APPLICATION_FLASH_SIZE						= (FLASH_SIZE-1024)
	.EQU	_BLDR_BOCKS_QNT								= _APPLICATION_FLASH_SIZE/FLASH_PAGESIZE
	;---регистры---
	.DEF	B00												= r2
	.DEF	FLASH_CUR_BLOCK								= r4
	.DEF	BUS_ADDR											= r25		;PID
	.DEF	ERR_CNTR											= r23		;TRY_CNTR

	;---BUS5277-OFFSETS---
	.EQU	_BLDR_BUS5277_PACKET_HEADER				= 0x00	;3B - заголовок пакета
	.EQU	_BLDR_BUS5277_PACKET_HEADER_ADDR			= 0x00
	.EQU	_BLDR_BUS5277_PACKET_HEADER_SIZE			= 0x01
	.EQU	_BLDR_BUS5277_PACKET_HEADER_CODE			= 0x02
	.EQU	_BLDR_BUS5277_PACKET_DATA					= 0x03
	;---BUS5277-REQUEST-CODES---
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_MODE				= 0x0c	;Перейти в режим бутлоадера
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_WRITE			= 0x0d	;Записать блока данных во флеш, передаем номер 32б блока в странице или смещение во FLASH
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_DONE				= 0x0e	;Выйти из режима бутлоадера
	;---BUS5277-RESPONSE-CODES---
	.EQU	_BLDR_BUS5277_RESP_OK						= 0x80	;Все в порядке
	.EQU	_BLDR_BUS5277_RESP_ILLEGAL_DATA_ADDR	= 0x84	;Не корректная длина пакета или запрошенный блок выходит за рамки доступной FLASH
	.EQU	_BLDR_BUS5277_RESP_REJECT					= 0x86	;Произошел сбой записи
	.EQU	_BLDR_BUS5277_RESP_ILLEGAL_COMMAND		= 0x8a	;Не корректная команда
;--------------------------------------------------------
BLDR_START:
;--------------------------------------------------------
;Точка входа в загрузчик
;--------------------------------------------------------
	;Запрет прерываний
	CLI
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	LDI TEMP,0x00
	MOV B00,TEMP

	LDI_X EEPROM_BLDR_ENABLED
	MCALL _BLDR__EEPROM_READ_BYTE
	CPI TEMP,0x52
	BREQ PC+0x03
	MJMP 0x0000

	LDI_X EEPROM_BUS_ADDR
	MCALL _BLDR__EEPROM_READ_BYTE
	MOV BUS_ADDR,TEMP

	LDI_X EEPROM_BLDR_ERR_CNTR
	MCALL _BLDR__EEPROM_READ_BYTE
	MOV ERR_CNTR,TEMP
	CP ERR_CNTR,B00
	BREQ PC+0x00

	;Инициализация портов
	;DR-OUT,LO
.if	BUS_DR_PORT != 0xff
	LDI ACCUM,BUS_DR_PORT
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_LO
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_HI
.endif
	;LED-OUT,LO
.if	ACT_LED_PORT != 0xff
	LDI ACCUM,ACT_LED_PORT
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_LO
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_HI
.endif
	;RX-IN,HI
	LDI ACCUM,RXD
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_HI
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_LO
	;TX-OUT,HI
	LDI ACCUM,TXD
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_HI
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_HI

	;Инициализация UART
	LDI TEMP,(1<<RXC0)|(1<<TXC0)
	STS UCSR0A,TEMP
	STS UCSR0B,B00
	LDI TEMP,low((1<<URSEL0)|(1<<UCSZ01)|(1<<UCSZ00))		;URSEL необходим для ATmega16, в остальных должен быть равен 0
	STS UCSR0C,TEMP
	LDI TEMP,51/(16/CORE_FREQ)
	STS UBRR0L,TEMP
	STS UBRR0H,B00

_BLDR_START__REINIT:
	LDI TEMP,_BLDR_BOCKS_QNT
	MOV FLASH_CUR_BLOCK,TEMP

_BLDR_START__RX_START:
	;Прием UART
	LDI TEMP,(1<<RXEN0)
	STS UCSR0B,TEMP

	CLR TEMP_L															;OFFSET
	LDI_Z RAMSTART
	LDI_Y 0x00
	LDS TEMP,UCSR0A	;2
	ADIW YL,0x01		;2
	;Проверка на таймаут
	CPI YH,0x06		;1													;чуть больше 1 октета на скорости 19200 (цикл 9 тактов)
	BREQ _BLDR_START__RX_DONE	;1
	SBRS TEMP,RXC0	;1
	RJMP PC-0x06	;2
	LDS TEMP,UDR0
	ST Z+,TEMP
	INC TEMP_L
	CPI TEMP_L,_BLDR_UART_BUFFER_LEN
	BREQ _BLDR_START__RX_START
_BLDR_START__RX_DONE:
	;Выключаю UART
	LDS TEMP,UCSR0B
	ANDI TEMP,~((1<<TXEN0)|(1<<RXEN0))
	STS UCSR0B,TEMP

	LDI_Z RAMSTART
	;Проверка на соответствие адреса
	LDD TEMP,Z+_BLDR_BUS5277_PACKET_HEADER_ADDR
	CP TEMP,BUS_ADDR
	BRNE _BLDR_START__RX_START
	;Проверка на соответствие длины пакета и принятых данных
	LDD TEMP,Z+_BLDR_BUS5277_PACKET_HEADER_SIZE
	CP TEMP,TEMP_L
	BRNE _BLDR_START__RX_START

	LDD TEMP_H,Z+_BLDR_BUS5277_PACKET_HEADER_CODE
	;Проверка на CRC8
	MCALL _BLDR_CRC8_BLOCK
	DEC TEMP_L
	ADD ZL,TEMP_L
	CLR TEMP
	ADC ZH,TEMP
	LD TEMP,Z
	CP TEMP,ACCUM
	BRNE _BLDR_START__RX_START

	;Анализ на код функции
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_MODE
	BRNE _BLDR_START__NOT_MODE

		LDI_Y RAMSTART
		ST Y+,B00
		LDI TEMP,0x04+0x0f											;TYPE+SERIAL+PCB_VER+FW_VER+BUS_ADDR+ERR_CNTR+_BLDR_BOCKS_QNT+FLASH_PAGESIZE/8
		ST Y+,TEMP
		LDI TEMP,_BLDR_BUS5277_RESP_OK
		ST Y+,TEMP

		LDI_X EEPROM_DEV_TYPE
		LDI LOOP_CNTR,0x0d
_BLDR_START__MODE_LOOP:
		MCALL _BLDR__EEPROM_READ_BYTE
		ST Y+,TEMP
		ADIW XL,0x01
		DEC LOOP_CNTR
		BRNE _BLDR_START__MODE_LOOP

		LDI TEMP,_BLDR_BOCKS_QNT
		ST Y+,TEMP
		LDI TEMP,FLASH_PAGESIZE/8
		ST Y+,TEMP

		LDI_Z RAMSTART
		LDI TEMP_L,0x03+0x0f
		MCALL _BLDR_CRC8_BLOCK
		ST Y,ACCUM

		MCALL _BLDR__UART_TX_START
		RJMP _BLDR_START__RX_START

_BLDR_START__NOT_MODE:
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_WRITE
	BRNE _BLDR_START__NOT_WRITE
		CPI TEMP_L,0x03+0x01+0x01+0x20+0x01						;HEADER(3)+FLASH_BLOCK(1)+BUFFER BLOCK(1)+DATA(32)+CRC(1)
		BRNE _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDD ACCUM,Y+_BLDR_BUS5277_PACKET_DATA+0x00			;FLASH BLOCK NUM
		CP ACCUM,FLASH_CUR_BLOCK
		BRNE _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDD TEMP_L,Y+_BLDR_BUS5277_PACKET_DATA+0x01			;BUFFER BLOCK 0-3 или 0-7
		CPI TEMP_L,FLASH_PAGESIZE/32
		BRCC _BLDR_RESP__ILLEGAL_DATA_ADDR

		ADIW YL,_BLDR_BUS5277_PACKET_DATA+0x02

		;Вычисляю смещение в буфере с учетом номера блока
		PUSH TEMP_L
		LDI_Z _BLDR_FLASH_BUFFER
		CLR TEMP_H
		SWAP TEMP_L
		ANDI TEMP_L,0xf0
		LSL TEMP_L
		ROL TEMP_H
		LSL TEMP_L
		ROL TEMP_H
		ADD ZL,TEMP_L
		ADC ZH,TEMP_H
		LDI LOOP_CNTR,0x20
		POP TEMP_L
_BLDR_START__WRITE_FILL_FLASH_BUFFER:
		LD TEMP,Y+
		ST Z+,TEMP
		DEC LOOP_CNTR
		BRNE _BLDR_START__WRITE_FILL_FLASH_BUFFER

		CP TEMP_L,B00
		BREQ _BLDR__WRITE_PAGE_TO_FLASH

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		MCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

_BLDR_RESP__ILLEGAL_DATA_ADDR:
		LDI TEMP_H,_BLDR_BUS5277_RESP_ILLEGAL_DATA_ADDR
		MCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

_BLDR_START__NOT_WRITE:
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_DONE
	BRNE _BLDR_START__NOT_DONE
		LDI_Y _APPLICATION_FLASH_SIZE
		LDI ACCUM,0x00
_BLDR_START__DONE_LOOP:
		LD TEMP,-Y
		MCALL _BLDR_CRC8
		CP YH,B00
		BRNE _BLDR_START__DONE_LOOP
		CP YL,B00
		BRNE _BLDR_START__DONE_LOOP

		LDI_Z RAMSTART
		LDD TEMP,Z+_BLDR_BUS5277_PACKET_DATA+0x00
		CP TEMP,ACCUM
		BREQ PC+0x0a
		;CRC не совпался
		CP ERR_CNTR,B00
		BREQ PC+0x00
		DEC ERR_CNTR
		MOV TEMP,ERR_CNTR
		LDI_X EEPROM_BLDR_ERR_CNTR
		MCALL _BLDR__EEPROM_WRITE_BYTE
		RJMP _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDI_X EEPROM_BLDR_ENABLED
		LDI TEMP,0x00
		MCALL _BLDR__EEPROM_WRITE_BYTE

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		MCALL _BLDR__MAKE_RESP
		MJMP 0x0000

_BLDR_START__NOT_DONE:
		RJMP _BLDR_RESP__ILLEGAL_DATA_ADDR

;--------------------------------------------------------
_BLDR__WRITE_PAGE_TO_FLASH:
;--------------------------------------------------------
;Блок процедуры записи страницы FLASH
;IN: Z-адрес на выделенную память, ACCUM-комер страницы
;--------------------------------------------------------
		;LED TURN ON
.if	ACT_LED_PORT != 0xff
		PUSH ACCUM
		LDI ACCUM,ACT_LED_PORT
		MCALL _BLDR__PORT_OFFSETS
		MOV TEMP,TEMP_H
		MCALL _BLDR__PORT_SET_HI
		POP ACCUM
.endif

.ifdef DEF_BOOTLADER_SECURE
		LDI_X EEPROM_BLDR_SECURE_DATA
		_BLDR_SECURE
.endif

		LDI TEMP,(1<<PGERS)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP

		PUSH_Z
		LDI_Y _BLDR_FLASH_BUFFER
		LDI LOOP_CNTR,FLASH_PAGESIZE/2
		LDI TEMP,(1<<SPMEN)
_BLDR__WRITE_PAGE_TO_FLASH__WLOOP:
		LD r0,Y+
		LD r1,Y+
		MCALL _BLDR__DO_SMP
		ADIW ZL,0x02
		DEC LOOP_CNTR
		BRNE _BLDR__WRITE_PAGE_TO_FLASH__WLOOP
		POP_Z

		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP

		LDI_Y _BLDR_FLASH_BUFFER
		LDI LOOP_CNTR,FLASH_PAGESIZE
_BLDR__WRITE_PAGE_TO_FLASH__RLOOP:
		LPM r0,Z+
		LD r1,Y+
		CP r0,r1
		BREQ PC+0x05
		LDI TEMP_H,_BLDR_BUS5277_RESP_REJECT
		MCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START
		DEC LOOP_CNTR
		BRNE _BLDR__WRITE_PAGE_TO_FLASH__RLOOP

_BLDR__WRITE_PAGE_TO_FLASH__RWWLOOP:
		LDS TEMP,SPMCSR
		SBRS TEMP,RWWSB
		RJMP _BLDR__WRITE_PAGE_TO_FLASH__DONE
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP
		RJMP _BLDR__WRITE_PAGE_TO_FLASH__RWWLOOP

_BLDR__WRITE_PAGE_TO_FLASH__DONE:
		;LED TURN OFF
.if	ACT_LED_PORT != 0xff
		LDI ACCUM,ACT_LED_PORT
		MCALL _BLDR__PORT_OFFSETS
		MOV TEMP,TEMP_H
		MCALL _BLDR__PORT_SET_LO
.endif

		DEC FLASH_CUR_BLOCK

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		MCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

;--------------------------------------------------------
_BLDR__MAKE_RESP:
;--------------------------------------------------------
;TEMP_H-code(если включен 6 бит, то в тело записываем
;1 байт из ACCUM)
;--------------------------------------------------------
	LDI_Y RAMSTART
	ST Y+,B00
	LDI TEMP_L,0x04
	SBRC TEMP_H,0x06
	INC TEMP_L
	ST Y+,TEMP_L
	ST Y+,TEMP_H
	SBRC TEMP_H,0x06
	ST Y+,ACCUM
	LDI_Z RAMSTART
	MCALL _BLDR_CRC8_BLOCK
	ST Y,ACCUM
;--------------------------------------------------------
_BLDR__UART_TX_START:
;--------------------------------------------------------
;IN:TEMP_L-длина данных
;--------------------------------------------------------
	;DR TURN ON
.if	BUS_DR_PORT != 0xff
	LDI ACCUM,BUS_DR_PORT
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_HI
.endif
	LDI TEMP,(1<<TXEN0)
	STS UCSR0B,TEMP
	LDI_Y RAMSTART
_BLDR__UART_TX_LOOP:
	LD TEMP,Y+
	STS UDR0,TEMP
	LDS TEMP,UCSR0A
	SBRS TEMP,TXC0
	RJMP PC-0x03
	DEC TEMP_L
	BRNE _BLDR__UART_TX_LOOP
	STS UCSR0B,B00
	;DR TURN OFF
.if	BUS_DR_PORT != 0xff
	LDI ACCUM,BUS_DR_PORT
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_LO
.endif
	RET

;--------------------------------------------------------
_BLDR__EEPROM_READ_BYTE:
;--------------------------------------------------------
;Считываем байт из EEPROM
;IN: X-адрес в EEPROM
;OUT: TEMP-значение
;--------------------------------------------------------
_BLDR__EEPROM_READ_BYTE__LOOP:
	LDS TEMP,EECR
	ANDI TEMP,(1<<EEPE)
	BRNE _BLDR__EEPROM_READ_BYTE__LOOP
	STS EEARH,XH
	STS EEARL,XL
	LDS TEMP,EECR
	ORI TEMP,(1<<EERE)
	STS EECR,TEMP
	LDS TEMP,EEDR
	RET
;--------------------------------------------------------
_BLDR__EEPROM_WRITE_BYTE:
;--------------------------------------------------------
;Считываем байт из EEPROM
;IN: X-адрес в EEPROM
;OUT: TEMP-значение
;--------------------------------------------------------
_BLDR__EEPROM_WRITE_BYTE__LOOP:
	PUSH TEMP
	LDS TEMP,EECR
	ANDI TEMP,(1<<EEPE)
	BRNE _BLDR__EEPROM_WRITE_BYTE__LOOP
	STS EEARH,TEMP_H
	STS EEARL,TEMP_L
	POP TEMP
	PUSH TEMP
	STS EEDR,TEMP
	LDS TEMP,EECR
	ORI TEMP,(1<<EEMPE)
	STS EECR,TEMP
	ORI TEMP,(1<<EEPE)
	STS EECR,TEMP
	POP TEMP
	RET
;--------------------------------------------------------
_BLDR__PORT_OFFSETS:
;--------------------------------------------------------
;Возвращаем адреса для PORTx, DDRx,
;а также пин в виде числа(1,2,4,8,16,32,64,128)
;IN: ACCUM сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: TEMP_H-адрес регистра PORTx,
;TEMP_L-адрес регистра DDRx,
;ACCUM-пин в виде числа
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	MOV TEMP,ACCUM
	SWAP ACCUM
	ANDI ACCUM,0x0f
	MOV TEMP_H,ACCUM
	LSL ACCUM
	ADD ACCUM,TEMP_H
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM TEMP_H,Z+
	LPM TEMP_L,Z
	ANDI TEMP,0x0f
	MCALL _BLDR__BITNUM_TO_NUM
	MOV ACCUM,TEMP
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_BLDR__PORT_SET_HI:
;--------------------------------------------------------
;Устанавливаем порт в HI
;IN: TEMP-адрес порта, ACCUM-пин в виде числа
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	CLR ZH
	MOV ZL,TEMP
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_BLDR__PORT_SET_LO:
;--------------------------------------------------------
;Устанавливаем порт в LO
;IN: TEMP-адрес порта, ACCUM-пин в виде числа
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	CLR ZH
	MOV ZL,TEMP
	LD TEMP,Z
	COM ACCUM
	AND TEMP,ACCUM
	ST Z,TEMP
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_BLDR__BITNUM_TO_NUM_TABLE:
	.db	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
;--------------------------------------------------------
_BLDR__BITNUM_TO_NUM:
;--------------------------------------------------------
;Конвертирование байта в HEX(два ASCII символа)
;IN: TEMP - номер бита(0-7)
;OUT: TEMP - число(1,2,4,8,16,32,64,128)
;--------------------------------------------------------
	PUSH_Z
	LDI ZH,high(_BLDR__BITNUM_TO_NUM_TABLE*2)
	LDI ZL,low(_BLDR__BITNUM_TO_NUM_TABLE*2)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LPM TEMP,Z
	POP_Z
	RET
;--------------------------------------------------------
_BLDR_CRC8_BLOCK:
;--------------------------------------------------------
;Подсчет CRC для блока данных
;IN: Z-адрес на блок данных,TEMP_L-длина
;OUT: ACCUM - сумма
;--------------------------------------------------------
	CLR ACCUM
_BLDR_CRC8_BLOCK__BYTES_LOOP:
	LD TEMP,Z+
	MCALL _BLDR_CRC8
	DEC TEMP_L
	BRNE _BLDR_CRC8_BLOCK__BYTES_LOOP
	RET

;--------------------------------------------------------
_BLDR_CRC8:
;--------------------------------------------------------
;Подсчет CRC
;IN: TEMP-значение,ACCUM-сумма
;OUT: ACCUM-сумма
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP_H

	LDI TEMP_H,0x8C
	EOR ACCUM,TEMP
	LDI LOOP_CNTR,0x08
_BLDR_CRC8__LOOP:
	LSR ACCUM
	BRCC PC+0x02
	EOR ACCUM,TEMP_H
	DEC LOOP_CNTR
	BRNE _BLDR_CRC8__LOOP

	POP TEMP_H
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_BLDR__DO_SMP:
;--------------------------------------------------------
;Выполняю команду SPM
;IN: Z-адрес FLAHS,r1/r0-слово,TEMP-значение SPMCSR
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SPMCSR
	SBRC TEMP,SPMEN
	RJMP PC-0x03
	POP TEMP

	STS SPMCSR,TEMP
	SPM
	RET

	.ORG APPLICATION_BLOCK
.endif

