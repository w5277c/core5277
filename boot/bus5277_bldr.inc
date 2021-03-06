;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;21.02.2021  w5277c@gmail.com			Начало
;11.03.2021  w5277c@gmail.com			Относительные переходы, но осталась зависимость от типа МК
;03.04.2021  w5277c@gmail.com			Багфиксинг
;04.04.2021  w5277c@gmail.com			Багфиксинг
;05.04.2021  w5277c@gmail.com			Багфиксинг(не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_BOOTLADER
.error "Bootloader already included!"
.else
.set DEF_BOOTLADER = 1

.if REPORT_INCLUDES == 0x01
.message "included BLDR Bus5277"
.endif

.ORG BOOT_512W_ADDR
	;---
	.SET	EEPROM_MSETS_OFFSET							= 0x08	;Смещение на основные настройки
	;---
	.EQU	EEPROM_RESERVED1								= 0x00	;8Б зарезервировано, первая страница, не безопасна
	.SET	EEPROM_DEV_TYPE								= 0x08	;2B тип устройства
	.SET	EEPROM_DEV_SERIAL								= 0x0a	;8Б серийного номера
	.SET	EEPROM_PCB_VERSION							= 0x12	;1Б Версия PCB
	.SET	EEPROM_FW_VERSION								= 0x13	;1Б Версия прошивки
	.SET	EEPROM_BUS_ADDR								= 0x14	;1Б Адрес устройства на шине
	.SET	EEPROM_BLDR_ERR_CNTR							= 0x15	;1Б Счетчик ошибок, изначально равен 64, при нуле блокируем работу загрузчика
	.SET	EEPROM_BLDR_ENABLED							= 0x16	;1Б Признак работы бутлоадера(0x52)
	;--адреса портоа---
	.SET	EEPROM_BLDR_TX_PORT							= 0x17	;Адрес DDR, адрес PORT и значение байта для порта TX
	.SET	EEPROM_BLDR_RX_PORT							= 0x1a	;Адрес DDR, адрес PORT и значение байта для порта RX
	.SET	EEPROM_BLDR_DR_PORT							= 0x1d	;Адрес DDR, адрес PORT и значение байта(0xff-не используется) для порта DR
	;---выравниваем по размеру страницы 8Б---
.ifdef EEPROM_BLDR_SECURE_DATA_SIZE
	.EQU	EEPROM_BLDR_SECURE_DATA						= 0x20	;Смещение для опциональных данных процедуры расшифровки данных прошивки
	.SET	EEPROM_ESETS_OFFSET							= EEPROM_BLDR_SECURE_DATA+EEPROM_BLDR_SECURE_DATA_SIZE	;Смещение доступное для нужд основной программы
.else
	.SET	EEPROM_ESETS_OFFSET							= 0x20	;Смещение доступное для нужд основной программы
.endif

	.EQU	_BLDR_UART_BUFFER_LEN						= 0x28	;40B - максимальная длина буфера UART
	.EQU	_BLDR_FLASH_BUFFER							= RAMSTART+0x28;FLASH_PAGESIZE - размер буфера

	.EQU	_APPLICATION_FLASH_SIZE						= (FLASH_SIZE-1024)
	.EQU	_BLDR_BOCKS_QNT								= _APPLICATION_FLASH_SIZE/FLASH_PAGESIZE
	;---регистры---
;	.DEF	B00												= r2
;	.EQU	BUS_ADDR											= r25		;PID
;	.EQU	ERR_CNTR											= r23		;TRY_CNTR

	;---BUS5277-OFFSETS---
	.EQU	_BLDR_BUS5277_PACKET_HEADER				= 0x00	;3B - заголовок пакета
	.EQU	_BLDR_BUS5277_PACKET_HEADER_ADDR			= 0x00
	.EQU	_BLDR_BUS5277_PACKET_HEADER_SIZE			= 0x01
	.EQU	_BLDR_BUS5277_PACKET_HEADER_CODE			= 0x02
	.EQU	_BLDR_BUS5277_PACKET_DATA					= 0x03
	;---BUS5277-REQUEST-CODES---
	.EQU	_BLDR_BUS5277_FUNC_INFO						= 0x00	;Чтение информации об устройстве
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_MODE				= 0x0d	;Перейти в режим бутлоадера
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_WRITE			= 0x0e	;Записать блока данных во флеш, передаем номер 32б блока в странице или смещение во FLASH
	.EQU	_BLDR_BUS5277_FUNC_BTLDR_DONE				= 0x0f	;Выйти из режима бутлоадера
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
	RJMP PC+0x03
.dw 0x5277,0xb100
	;Инициализация стека
	LDI TEMP,high(RAMEND)
	STS SPH,TEMP
	LDI TEMP,low(RAMEND)
	STS SPL,TEMP

	LDI_X EEPROM_BLDR_ENABLED
	RCALL _BLDR__EEPROM_READ_BYTE
	CPI TEMP,0x52
	BREQ PC+0x03
	MJMP 0x0000

BLDR_UNCONDITIONAL_START:
	;Запрет прерываний
	CLI

	LDI TEMP,0x00
	MOV r2,TEMP
	LDI TEMP,0xff
	MOV r3,TEMP

	CLR ZH
	;TX-OUT,HI
	LDI_X EEPROM_BLDR_TX_PORT+0x02
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ACCUM,TEMP
	LDI_X EEPROM_BLDR_TX_PORT+0x01
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	LDI_X EEPROM_BLDR_TX_PORT+0x00
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	;RX-IN,HI
	LDI_X EEPROM_BLDR_RX_PORT+0x02
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ACCUM,TEMP
	LDI_X EEPROM_BLDR_RX_PORT+0x01
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	LDI_X EEPROM_BLDR_RX_PORT+0x00
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	LD TEMP,Z
	AND TEMP,ACCUM
	ST Z,TEMP

	;DR-OUT,LO
	LDI_X EEPROM_BLDR_DR_PORT+0x02
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV r6,TEMP
	CP r6,r3
	BREQ PC+0x0f
	LDI_X EEPROM_BLDR_DR_PORT+0x01
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	MOV r5,TEMP
	LD TEMP,Z
	AND TEMP,r6
	ST Z,TEMP
	LDI_X EEPROM_BLDR_DR_PORT+0x00
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV ZL,TEMP
	MOV r4,TEMP
	LD TEMP,Z
	OR TEMP,r6
	ST Z,TEMP

	LDI_X EEPROM_BUS_ADDR
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV r25,TEMP

	LDI_X EEPROM_BLDR_ERR_CNTR
	RCALL _BLDR__EEPROM_READ_BYTE
	MOV r23,TEMP
	CP r23,r2
	BREQ PC+0x00

	;Инициализация UART
	LDI TEMP,(1<<RXC0)|(1<<TXC0)
	STS UCSR0A,TEMP
	STS UCSR0B,r2
	LDI TEMP,low((1<<URSEL0)|(1<<UCSZ01)|(1<<UCSZ00))		;URSEL необходим для ATmega16, в остальных должен быть равен 0
	STS UCSR0C,TEMP
	LDI TEMP,51/(16/CORE_FREQ)
	STS UBRR0L,TEMP
	STS UBRR0H,r2

_BLDR_START__RX_START:
	;Прием UART
	LDI TEMP,0x00
	STS UCSR0A,TEMP
	LDI TEMP,(1<<RXEN0)
	STS UCSR0B,TEMP

_BLDR_START__RX_LOOP1:
	CLR TEMP_L															;OFFSET
	LDI_Z RAMSTART
_BLDR_START__RX_LOOP2:
	LDI_Y 0x00
	LDS TEMP,UCSR0A	;2
	ADIW YL,0x01		;2
	;Проверка на таймаут
	CPI YH,0x06		;1													;чуть больше 1 октета на скорости 19200 (цикл 9 тактов)
	BREQ _BLDR_START__RX_TIMEOUT	;1
	SBRS TEMP,RXC0	;1
	RJMP PC-0x06	;2
	LDS TEMP,UDR0
	ST Z+,TEMP
	INC TEMP_L
	CPI TEMP_L,_BLDR_UART_BUFFER_LEN
	BREQ _BLDR_START__RX_LOOP1
	RJMP _BLDR_START__RX_LOOP2
_BLDR_START__RX_TIMEOUT:
	CPI TEMP_L,0x04
	BRCS _BLDR_START__RX_LOOP1

	;Выключаю UART
	LDS TEMP,UCSR0B
	ANDI TEMP,~((1<<TXEN0)|(1<<RXEN0))
	STS UCSR0B,TEMP

	LDI_Z RAMSTART
	;Проверка на соответствие адреса
	LDD TEMP,Z+_BLDR_BUS5277_PACKET_HEADER_ADDR
	CP TEMP,r25
	BRNE _BLDR_START__RX_START
	;Проверка на соответствие длины пакета и принятых данных
	LDD TEMP,Z+_BLDR_BUS5277_PACKET_HEADER_SIZE
	CP TEMP,TEMP_L
	BRNE _BLDR_START__RX_START

	LDD TEMP_H,Z+_BLDR_BUS5277_PACKET_HEADER_CODE
	;Проверка на CRC8
	DEC TEMP_L
	RCALL _BLDR_CRC8_BLOCK
	LD TEMP,Z
	CP TEMP,ACCUM
	BRNE _BLDR_START__RX_START
	INC TEMP_L
	LDI_Y RAMSTART

	;Анализ на код функции
	CPI TEMP_H,_BLDR_BUS5277_FUNC_INFO
	BRNE _BLDR_START__NOT_INFO

		ST Y+,r2
		LDI TEMP,0x04+0x10											;MAGIC(2)+TYPE(2)+SERIAL(8)+PCB_VER(1)+FW_VER(1)+BUS_ADDR(1)+ERR_CNTR(1)+_BLDR_BOCKS_QNT(1)+FLASH_PAGESIZE/8(1)
		ST Y+,TEMP
		LDI TEMP,_BLDR_BUS5277_RESP_OK
		ST Y+,TEMP
		LDI TEMP,0x77
		ST Y+,TEMP
		LDI TEMP,0x52
		ST Y+,TEMP

		LDI_X EEPROM_DEV_TYPE
		LDI LOOP_CNTR,0x0e
_BLDR_START__MODE_LOOP:
		RCALL _BLDR__EEPROM_READ_BYTE
		ST Y+,TEMP
		ADIW XL,0x01
		DEC LOOP_CNTR
		BRNE _BLDR_START__MODE_LOOP

		LDI TEMP,_BLDR_BOCKS_QNT
		ST Y+,TEMP
		LDI TEMP,FLASH_PAGESIZE/8
		ST Y+,TEMP

		LDI_Z RAMSTART
		LDI TEMP_L,0x03+0x12
		RCALL _BLDR_CRC8_BLOCK
		ST Y,ACCUM
		INC TEMP_L
		RCALL _BLDR__UART_TX_START
		RJMP _BLDR_START__RX_START

_BLDR_START__NOT_INFO:
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_WRITE
	BRNE _BLDR_START__NOT_WRITE
		CPI TEMP_L,0x03+0x01+0x01+0x20+0x01						;HEADER(3)+FLASH_BLOCK(1)+BUFFER BLOCK(1)+DATA(32)+CRC(1)
		BRNE _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDD ACCUM,Y+_BLDR_BUS5277_PACKET_DATA+0x00			;FLASH BLOCK NUM
		CPI ACCUM,_BLDR_BOCKS_QNT
		BRCC _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDD TEMP_L,Y+_BLDR_BUS5277_PACKET_DATA+0x01			;BUFFER BLOCK 0-3 или 0-7
		CPI TEMP_L,FLASH_PAGESIZE/32
		BRCC _BLDR_RESP__ILLEGAL_DATA_ADDR

		CPI ACCUM,0x00
		BRNE PC+0x04
		CPI TEMP_L,0x00
		BRNE PC+0x02
		NOP

		ADIW YL,_BLDR_BUS5277_PACKET_DATA+0x02

		;Вычисляю смещение в буфере с учетом номера блока
		PUSH TEMP_L
		LDI_Z _BLDR_FLASH_BUFFER
		CLR TEMP_H
		SWAP TEMP_L
		ANDI TEMP_L,0xf0
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

		CP TEMP_L,r2
		BREQ _BLDR__WRITE_PAGE_TO_FLASH

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		RCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

_BLDR_RESP__ILLEGAL_DATA_ADDR:
		LDI TEMP_H,_BLDR_BUS5277_RESP_ILLEGAL_DATA_ADDR
		RCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

_BLDR_START__NOT_WRITE:
	CPI TEMP_H,_BLDR_BUS5277_FUNC_BTLDR_DONE
	BRNE _BLDR_START__NOT_DONE
		LDI_Y _APPLICATION_FLASH_SIZE
		LDI ACCUM,0x00
_BLDR_START__DONE_LOOP:
		LD TEMP,-Y
		RCALL _BLDR_CRC8
		CP YH,r2
		BRNE _BLDR_START__DONE_LOOP
		CP YL,r2
		BRNE _BLDR_START__DONE_LOOP

		LDI_Z RAMSTART
		LDD TEMP,Z+_BLDR_BUS5277_PACKET_DATA+0x00
		CP TEMP,ACCUM
		BREQ PC+0x09
		;CRC не совпался
		CP r23,r2
		BREQ PC+0x00
		DEC r23
		MOV TEMP,r23
		LDI_X EEPROM_BLDR_ERR_CNTR
		RCALL _BLDR__EEPROM_WRITE_BYTE
		RJMP _BLDR_RESP__ILLEGAL_DATA_ADDR

		LDI_X EEPROM_BLDR_ENABLED
		LDI TEMP,0x00
		RCALL _BLDR__EEPROM_WRITE_BYTE

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		RCALL _BLDR__MAKE_RESP
		MJMP 0x0000

_BLDR_START__NOT_DONE:
		RJMP _BLDR_RESP__ILLEGAL_DATA_ADDR

;--------------------------------------------------------
_BLDR__WRITE_PAGE_TO_FLASH:
;--------------------------------------------------------
;Блок процедуры записи страницы FLASH
;IN: ACCUM-комер страницы
;--------------------------------------------------------
		LDI_Z _BLDR_FLASH_BUFFER
		PUSH ACCUM
		;Влючаем признак бутлоадера, теперь выход возможен только после полной прошивки
		LDI XH,high(EEPROM_BLDR_ENABLED)
		LDI XL,low(EEPROM_BLDR_ENABLED)
		RCALL _BLDR__EEPROM_READ_BYTE
		CPI TEMP,0x52
		BREQ PC+0x03
		LDI TEMP,0x52
		RCALL _BLDR__EEPROM_WRITE_BYTE

.ifdef DEF_BOOTLADER_SECURE
		LDI_X EEPROM_BLDR_SECURE_DATA
		_BLDR_SECURE
.endif

		;Вычисляем адрес в FLASH
		POP ZH
		CLR ZL
		LDI TEMP,(0x100-FLASH_PAGESIZE)/0x40
		DEC TEMP
		BRCS _BLDR__WRITE_PAGE_TO_FLASH__Z_COMP_SKIP
_BLDR__WRITE_PAGE_TO_FLASH__Z_COMP_LOOP:
		LSR ZH
		ROR ZL
		DEC TEMP
		BRNE _BLDR__WRITE_PAGE_TO_FLASH__Z_COMP_LOOP
_BLDR__WRITE_PAGE_TO_FLASH__Z_COMP_SKIP:

		LDI TEMP,(1<<PGERS)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP

		PUSH_Z
		LDI_Y _BLDR_FLASH_BUFFER
		LDI LOOP_CNTR,FLASH_PAGESIZE/2
		LDI TEMP,(1<<SPMEN)
_BLDR__WRITE_PAGE_TO_FLASH__WLOOP:
		LD r0,Y+
		LD r1,Y+
		RCALL _BLDR__DO_SMP
		ADIW ZL,0x02
		DEC LOOP_CNTR
		BRNE _BLDR__WRITE_PAGE_TO_FLASH__WLOOP
		POP_Z

		LDI TEMP,(1<<PGWRT)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP

		LDI_Y _BLDR_FLASH_BUFFER
		LDI LOOP_CNTR,FLASH_PAGESIZE
_BLDR__WRITE_PAGE_TO_FLASH__RLOOP:
		LPM r0,Z+
		LD r1,Y+
		CP r0,r1
		BREQ PC+0x06
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP
		LDI TEMP_H,_BLDR_BUS5277_RESP_REJECT
		RCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START
		DEC LOOP_CNTR
		BRNE _BLDR__WRITE_PAGE_TO_FLASH__RLOOP

_BLDR__WRITE_PAGE_TO_FLASH__RWWLOOP:
		LDS TEMP,SPMCSR
		SBRS TEMP,RWWSB
		RJMP _BLDR__WRITE_PAGE_TO_FLASH__DONE
		LDI TEMP,(1<<RWWSRE)|(1<<SPMEN)
		RCALL _BLDR__DO_SMP
		RJMP _BLDR__WRITE_PAGE_TO_FLASH__RWWLOOP

_BLDR__WRITE_PAGE_TO_FLASH__DONE:

		LDI TEMP_H,_BLDR_BUS5277_RESP_OK
		RCALL _BLDR__MAKE_RESP
		RJMP _BLDR_START__RX_START

;--------------------------------------------------------
_BLDR__MAKE_RESP:
;--------------------------------------------------------
;TEMP_H-code(если включен 6 бит, то в тело записываем
;1 байт из ACCUM)
;--------------------------------------------------------
	LDI_Y RAMSTART
	ST Y+,r2
	LDI TEMP_L,0x04
	SBRC TEMP_H,0x06
	INC TEMP_L
	ST Y+,TEMP_L
	ST Y+,TEMP_H
	SBRC TEMP_H,0x06
	ST Y+,ACCUM
	LDI_Z RAMSTART
	DEC TEMP_L
	RCALL _BLDR_CRC8_BLOCK
	ST Y,ACCUM
	INC TEMP_L
;--------------------------------------------------------
_BLDR__UART_TX_START:
;--------------------------------------------------------
;IN:TEMP_L-длина данных
;--------------------------------------------------------
	;DR TURN ON
	CP r6,r3
	BREQ PC+0x06
	CLR YH
	MOV YL,r5
	LD TEMP,Y
	OR TEMP,r6
	ST Y,TEMP

	LDI TEMP,(1<<TXEN0)
	STS UCSR0B,TEMP
	LDI_Y RAMSTART
_BLDR__UART_TX_LOOP:
	LDS TEMP,UCSR0A
	SBRS TEMP,UDRE0
	RJMP _BLDR__UART_TX_LOOP
	CPI TEMP_L,0x00
	BREQ _BLDR__UART_TX_STOP
	LD TEMP,Y+
	STS UDR0,TEMP
	DEC TEMP_L
	RJMP _BLDR__UART_TX_LOOP
_BLDR__UART_TX_STOP:
	LDI_Y 0x1800
_BLDR__UART_TX_STOP_WAIT:
	SBIW YL,0x01
	BRNE _BLDR__UART_TX_STOP_WAIT

	STS UCSR0B,r2
	;DR TURN OFF
	CP r6,r3
	BREQ PC+0x06
	CLR YH
	MOV YL,r5
	LD TEMP,Y
	AND TEMP,r6
	ST Y,TEMP
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
_BLDR_CRC8_BLOCK:
;--------------------------------------------------------
;Подсчет CRC для блока данных
;IN: Z-адрес на блок данных,TEMP_L-длина
;OUT: ACCUM - сумма
;--------------------------------------------------------
	PUSH TEMP_L
	CLR ACCUM
_BLDR_CRC8_BLOCK__BYTES_LOOP:
	LD TEMP,Z+
	RCALL _BLDR_CRC8
	DEC TEMP_L
	BRNE _BLDR_CRC8_BLOCK__BYTES_LOOP
	POP TEMP_L
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
;IN: Z-адрес FLASH,r1/r0-слово,TEMP-значение SPMCSR
;--------------------------------------------------------
	PUSH TEMP
_BLDR__DO_SMP_WAIT_SPM:
	LDS TEMP,SPMCSR
	SBRC TEMP,SPMEN
	RJMP _BLDR__DO_SMP_WAIT_SPM

_BLDR__DO_SMP_WAIT_EE:
	LDS TEMP,EECR
	SBRC TEMP,EEPE
	RJMP _BLDR__DO_SMP_WAIT_EE

	POP TEMP
	STS SPMCSR,TEMP
	SPM
	RET

	.ORG APPLICATION_BLOCK
.endif

