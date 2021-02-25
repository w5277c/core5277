;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;21.02.2021  w5277c@gmail.com			Начало(не оттестировано)
;-----------------------------------------------------------------------------------------------------------------------

;1)Функционал UART_H 19200 8n1(прием 40 байт максимум, отправка 5 байт максимум)
;2)Подсчет CRC8
;3)Запросы:
;	1)Записать блок 32Б (addr(1Б) + size(1Б) + code(1б) + data(номер блока(1Б)+данные32Б) + crc(1Б)
;	2)Записать страницу (addr(1Б) + size(1Б) + code(1б) + data(адрес страницы в байтах(2Б)) + crc(1Б)
;	2)Выйти из режима BOOTLOADER'а (addr(1Б) + size(1Б) + code(1б) + crc(1Б)

;Для тестирования
.include			"./devices/atmega328.inc"

.ORG 0x01c0
	;---EEPROM-DATA---
	.EQU	_BLDR_EEPROM_LED_PORT						= 0x0080	;1B - Порт индикации
	.EQU	_BLDR_EEPROM_BOOT_ENABLE					= 0x0081	;1B - 0x52-активируем загрузчик
	.EQU	_BLDR_EEPROM_DR_PORT							= 0x0082	;1B - Порт для задания направления прием/передача
	.EQU	_BLDR_EEPROM_BUS5277_ADDR					= 0x0083	;1B - Адрес на шине

	;---CONSTANTS---
	.EQU	_BLDR_UART_BUFFER_LEN						= 0x28	;40B - максимальная длина буфера UART
	.EQU	_BLDR_FLASH_BUFFER							= RAMSTART+0x28;FLASH_PAGESIZE_B - размер буфера

	;---BUS5277-OFFSETS---
	.EQU	_BLDR_BUS5277_PACKET_HEADER				= 0x00	;3B - заголовок пакета
	.EQU	_BLDR_BUS5277_PACKET_HEADER_ADDR			= 0x00
	.EQU	_BLDR_BUS5277_PACKET_HEADER_SIZE			= 0x01
	.EQU	_BLDR_BUS5277_PACKET_HEADER_CODE			= 0x02
	.EQU	_BLDR_BUS5277_PACKET_DATA					= 0x03
	;---BUS5277-REQUEST-CODES---
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

	LDI_X _BLDR_EEPROM_BOOT_ENABLE
	MCALL _BLDR__EEPROM_READ_BYTE
	CPI TEMP,0x52
	BREQ PC+0x03
	MJMP 0x0000

	LDI_X _BLDR_EEPROM_LED_PORT
	MCALL _BLDR__EEPROM_READ_BYTE
	MOV TEMP_EH,TEMP
	LDI_X _BLDR_EEPROM_DR_PORT
	MCALL _BLDR__EEPROM_READ_BYTE
	MOV TEMP_EL,TEMP
	LDI_X _BLDR_EEPROM_BUS5277_ADDR
	MCALL _BLDR__EEPROM_READ_BYTE
	MOV PID,TEMP

	;Инициализация портов
	;DR-OUT,LO
	MOV ACCUM,TEMP_EL
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_LO
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_HI
;	;LED-OUT,LO
	MOV ACCUM,TEMP_EH
	CPI ACCUM,0xff
	BREQ _BLDR_START__LED_PORT_INIT_SKIP
	MCALL _BLDR__PORT_OFFSETS
	MOV TEMP,TEMP_H
	MCALL _BLDR__PORT_SET_LO
	MOV TEMP,TEMP_L
	MCALL _BLDR__PORT_SET_HI
_BLDR_START__LED_PORT_INIT_SKIP:
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
	LDI TEMP,0x00
	STS UCSR0B,TEMP
	LDI TEMP,low((1<<URSEL0)|(1<<UCSZ01)|(1<<UCSZ00))		;URSEL необходим для ATmega16, в остальных должен быть равен 0
	STS UCSR0C,TEMP
	LDI TEMP,51/(16/CORE_FREQ)										;19200
	STS UBRR0L,TEMP
	LDI TEMP,0x00
	STS UBRR0H,TEMP

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
	LDS TEMP,UCSR0B
	ANDI TEMP,~((1<<TXEN0)|(1<<RXEN0))
	STS UCSR0B,TEMP
	LDI_Y RAMSTART
	LDD TEMP,Y+_BLDR_BUS5277_PACKET_HEADER_ADDR
	CP TEMP,PID
	BRNE _BLDR_START__RX_START
	LDD TEMP,Y+_BLDR_BUS5277_PACKET_HEADER_SIZE
	CP TEMP,TEMP_L
	BRNE _BLDR_START__RX_START
	LDD TEMP_H,Y+_BLDR_BUS5277_PACKET_HEADER_CODE
	MCALL _BLDR_CRC8_BLOCK
	ADD ZL,TEMP_L
	CLR TEMP
	ADC ZH,TEMP
	LD TEMP,Y
	CP TEMP,ACCUM
	BRNE _BLDR_START__RX_START

	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_WRITE
	BREQ PC+0x02
	RJMP _BLDR_START__NOT_WRITE
		CPI TEMP_L,0x03+0x01+0x20+0x01				;HEADER(3)+FLASH BUFFER BLOCK(1)+DATA(32)+CRC(1)
		BRNE _BLDR__WRITE_PAGE_TO_FLASH
		LDD TEMP_L,Y+_BLDR_BUS5277_PACKET_DATA+0x00			;Номер блока 0-3-7
		ADIW YL,_BLDR_BUS5277_PACKET_DATA
		;Вычисляю смещение в буфере с учетом номера блока
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
		PUSH_Z
_BLDR_START__WRITE_FILL_FLASH_BUFFER:
		LD TEMP,Y+
		ST Z+,TEMP
		DEC LOOP_CNTR
		BRNE _BLDR_START__WRITE_FILL_FLASH_BUFFER
		POP_Z
		LDI TEMP_L,FLASH_PAGESIZE_B/16
		MCALL _BLDR_CRC8_BLOCK
		LDI TEMP_H,_BLDR_BUS5277_RESP_OK|0x40
		RJMP _BLDR__UART_TX_START
_BLDR__WRITE_PAGE_TO_FLASH:
		CPI TEMP_L,0x03+0x02+0x01				;HEADER(3)+FLASH OFFSET(2)+CRC(1)
		BREQ PC+0x03
		LDI TEMP_H,_BLDR_BUS5277_RESP_ILLEGAL_DATA_ADDR
		RJMP _BLDR__UART_TX_START
		CPI TEMP_L,high(BOOT_1024B_ADDR)
		BRCS PC+0x03
		LDI TEMP_H,_BLDR_BUS5277_RESP_ILLEGAL_DATA_ADDR
		RJMP _BLDR__UART_TX_START
		MOV ZH,TEMP_L
		LDD ZL,Y+_BLDR_BUS5277_PACKET_DATA+0x01
		;---Блок процедуры записи страницы FLASH---
		;LED TURN ON
		CPI TEMP_EH,0xff
		BREQ PC+0x07
		MOV ACCUM,TEMP_EH
		MCALL _BLDR__PORT_OFFSETS
		MOV TEMP,TEMP_H
		MCALL _BLDR__PORT_SET_HI

		LDI TEMP,(1<<PGERS)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		MCALL _BLDR__DO_SMP

		PUSH_Z
		LDI_Y _BLDR_FLASH_BUFFER
		LDI LOOP_CNTR,FLASH_PAGESIZE_B/2
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
		LDI LOOP_CNTR,FLASH_PAGESIZE_B
_BLDR__WRITE_PAGE_TO_FLASH__RLOOP:
		LPM r0,Z+
		LD r1,Y+
		CP r0,r1
		BREQ PC+0x03
		LDI TEMP_H,_BLDR_BUS5277_RESP_REJECT
		RJMP _BLDR__UART_TX_START
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
		CPI TEMP_EH,0xff
		BREQ PC+0x07
		MOV ACCUM,TEMP_EH
		MCALL _BLDR__PORT_OFFSETS
		MOV TEMP,TEMP_H
		MCALL _BLDR__PORT_SET_LO

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		RJMP _BLDR__UART_TX_START

_BLDR_START__NOT_WRITE:
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_DONE
	BRNE _BLDR_START__NOT_DONE
		LDI_X _BLDR_EEPROM_BOOT_ENABLE
		LDI TEMP,0x00
		MCALL _BLDR__EEPROM_WRITE_BYTE
		MJMP 0x0000
_BLDR_START__NOT_DONE:
	LDI TEMP_H,_BLDR_BUS5277_RESP_ILLEGAL_COMMAND
	RJMP _BLDR__UART_TX_START
_BLDR__UART_TX_START:
;TEMP_H-code(если включен 6 бит, то в тело записываем 1 байт из ACCUM)
	LDI_Y RAMSTART
	LDI TEMP,0x00
	ST Y+,TEMP
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
	LDI TEMP,0x00
	STS UCSR0B,TEMP
	RJMP _BLDR_START__RX_START
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
	PUSH TEMP_H
	PUSH LOOP_CNTR
	CLR ACCUM
	LDI TEMP_H,0x8C
_BLDR_CRC8_BLOCK__BYTES_LOOP:
	LD TEMP,Z+
	EOR ACCUM,TEMP
	LDI LOOP_CNTR,0x08
_BLDR_CRC8_BLOCK__BITES_LOOP:
	LSR ACCUM
	BRCC PC+0x02
	EOR ACCUM,TEMP_H
	DEC LOOP_CNTR
	BRNE _BLDR_CRC8_BLOCK__BITES_LOOP
	DEC TEMP_L
	BRNE _BLDR_CRC8_BLOCK__BYTES_LOOP
	POP LOOP_CNTR
	POP TEMP_H
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

;для тестирования
MAIN:
_C5_IR:
_C5_TIMER_A_IR:
_C5_TIMER_B_IR:
	RET
