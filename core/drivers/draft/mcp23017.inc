;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.11.2024	konstantin@5277.ru		Начало
;-----------------------------------------------------------------------------------------------------------------------
;Расширитель портов, с I2C входом
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции:
;DRV_OP_GET_PORT универсальная процедура получения значения из порта(см. VPORT),
;DRV_OP_SET_PORT универсальная процедура записи значения в порт(см. VPORT),
;DRV_OP_GET чтение значения из регистра,
;DRV_OP_SET запись значения в регистр,
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_MCP23017
.else
.set DEF__C5_DRV_MCP23017 = 1
.message "Included driver MCP23017 v0.1"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_MCP23017_PORTA					= 0x00	;Номер порта A
	.EQU	DRV_MCP23017_PORTB					= 0x01	;Номер порта B
	;---												;По умолчанию используется BANK=0
	.EQU	DRV_MCP23017_IODIR					= 0x00	;IODIR-регистр направления(0-выход)
	.EQU	DRV_MCP23017_IPOL					= 0x02	;IPOL-регистр инвертирования(1-инвертируем)
	.EQU	DRV_MCP23017_GPINTEN				= 0x04	;GPINTEN-регистр разрешения прерываний(1-прерывания разрешены)
	.EQU	DRV_MCP23017_DEFVAL					= 0x06	;DEFVAL-значение для сравнения, с последующим прерыванием при совпадении
	.EQU	DRV_MCP23017_INTCON					= 0x08	;INTCON-режим прерываний(сравниваем:0-с предыдущим значением,1-с DEFVAL)
	.EQU	DRV_MCP23017_IOCON					= 0x0a	;IOCONF-регистр конфигурации
	.EQU		DRV_MCP23017_IOCON_INTPOL		= 0x01	;INPOL-полярность вывода INT(активен:0-низкий,1-высокий)
	.EQU		DRV_MCP23017_IOCON_ODR			= 0x02	;ODR-Режим вывода INT(0-отвечает INPOL,1-вывод отключен)
	.EQU		DRV_MCP23017_IOCON_DISSLW		= 0x04	;DISSLW-скорсоть наростания порта SDA(пока не понятно)
	.EQU		DRV_MCP23017_IOCON_SEQOP		= 0x05	;SEQOP-инкрементация адреса регистров(0-инкрементация включена,1-адрес не меняется)
	.EQU		DRV_MCP23017_IOCON_MIRROR		= 0x06	;MIRROR-объединение порта INTA и INTB(0-соединены друг с другом, 1-независимы)
	.EQU		DRV_MCP23017_IOCON_BANK			= 0x07	;BANK-режим работы с блоками(сортировка:0-по регистрам, 1-по портам)
	.EQU	DRV_MCP23017_GPPUA					= 0x0c	;GPPUA-регистр подтяжки 100К к VCC(0-отключен)
	.EQU	DRV_MCP23017_INTF					= 0x0e	;INTF-регистр флагов прерывания(0-пин не участвует в прерывании), только для чтения
	.EQU	DRV_MCP23017_INTCAP					= 0x10	;INTCAP-регистр значения GPIO в момент прерывания(0-низкий уровень), только для чтения, сбрасывает прерывание.
	.EQU	DRV_MCP23017_GPIO					= 0x12	;GPIO-регистр значения порта(0-низкий уровень), чтение сбрасывает прерывание.
	.EQU	DRV_MCP23017_OLAT					= 0x14	;OLAT-регистр для записи значения в GPIO в режиме входа, прменяются при переходе в режим выхода

	;---
	.EQU	_DRV_MCP23017_I2C_ADDR				= 0x00	;1Б-адресс устройства
	.EQU	_DRV_MCP23017_I2C_DRV_ID			= 0x01	;1Б-ИД драйвера I2C master
	.EQU	_DRV_MCP23017_I2C_BUFFER			= 0x02	;2Б-буфер для i2c
	;---
	.EQU	_DRV_MCP23017_RAM_SIZE				= 0x04

;--------------------------------------------------------
DRV_MCP23017_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-адрес устройства, TEMP_L-ид драйвера I2C
;--------------------------------------------------------
	LDI ACCUM,_DRV_MCP23017_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_MCP23017_I2C_ADDR,TEMP_H
	STD Y+_DRV_MCP23017_I2C_DRV_ID,TEMP_L

	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET
	MOVW ZL,YL
	ADIW ZL,_DRV_MCP23017_I2C_BUFFER

	CPI FLAGS,DRV_OP_GET_PORT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_MCP23017_GET_PORT
	RJMP _DRV_MCP23017_PROC__END
	CPI FLAGS,DRV_OP_SET_PORT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_MCP23017_SET_PORT
	RJMP _DRV_MCP23017_PROC__END
	CPI FLAGS,DRV_OP_GET
	BRNE PC+0x03
	RCALL _DRV_MCP23017_I2C_READ
	RJMP _DRV_MCP23017_PROC__END
	CPI FLAGS,DRV_OP_SET
	BRNE PC+0x03
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_PROC__END

	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_MCP23017_PROC__END:
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_MCP23017_GET_PORT:
;--------------------------------------------------------
;Получаем значение от MCP23017(GPIO,IODIR A/B)
;IN: TEMP_H-номер порта(_DRV_MCP23017_PORT...),
;TEMP_L-тип порта(VPORT_REG_...), Y-адрес на переменные,
;Z-адрес на I2C буфер
;OUT: TEMP-код результата DRV_RESULT_..., ACCUM-значение
;--------------------------------------------------------
	PUSH LOOP_CNTR

	CPI TEMP_L,PORTx
	BREQ PC+0x03
	CPI TEMP_L,PINx
	BRNE _DRV_MCP23017_GET_PORT__NO_PORT
	LDI LOOP_CNTR,DRV_MCP23017_GPIO
	ADD LOOP_CNTR,TEMP_H
	RCALL _DRV_MCP23017_I2C_READ
	RJMP _DRV_MCP23017_GET_PORT__END
_DRV_MCP23017_GET_PORT__NO_PORT:
	CPI TEMP_L,DDRx
	BRNE _DRV_MCP23017_GET_PORT__NO_DDR
	LDI LOOP_CNTR,DRV_MCP23017_IODIR
	ADD LOOP_CNTR,TEMP_H
	RCALL _DRV_MCP23017_I2C_READ
	RJMP _DRV_MCP23017_GET_PORT__END
_DRV_MCP23017_GET_PORT__NO_DDR:
	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_MCP23017_GET_PORT__END:
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_MCP23017_SET_PORT:
;--------------------------------------------------------
;Записываем значение в MCP23017(GPIO,IODIR A/B)
;IN: ACCUM-значение, TEMP_H-номер порта
;(_DRV_MCP23017_PORT...), TEMP_L-тип порта (PINx,...)
;Y-адрес на переменные, Z-адрес на I2C
;буфер
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM

	CPI TEMP_L,PORTx
	BRNE _DRV_MCP23017_SET_PORT__NO_PORT
	LDI LOOP_CNTR,DRV_MCP23017_OLAT
	ADD LOOP_CNTR,TEMP_H
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_SET_PORT__END
_DRV_MCP23017_SET_PORT__NO_PORT:
	CPI TEMP_L,DDRx
	BRNE _DRV_MCP23017_SET_PORT__NO_DDR
	LDI LOOP_CNTR,DRV_MCP23017_IODIR
	ADD LOOP_CNTR,TEMP_H
	COM ACCUM
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_SET_PORT__END
_DRV_MCP23017_SET_PORT__NO_DDR:
	CPI TEMP_L,PINx
	BRNE _DRV_MCP23017_SET_PORT__NO_PIN
	LDI LOOP_CNTR,DRV_MCP23017_IPOL
	ADD LOOP_CNTR,TEMP_H
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_SET_PORT__END
_DRV_MCP23017_SET_PORT__NO_PIN:
	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_MCP23017_SET_PORT__END:
	POP ACCUM
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_DRV_MCP23017_I2C_READ:
;--------------------------------------------------------
;IN:LOOP_CNTR-номер регистра, Z-адрес на I2C буфер
;OUT:TEMP-DRV_RESULT_..., ACCUM-значение
;--------------------------------------------------------
	PUSH_T16

	STD Z+0x00,LOOP_CNTR

	LDD ACCUM,Y+_DRV_MCP23017_I2C_ADDR
	LDD TEMP,Y+_DRV_MCP23017_I2C_DRV_ID
	LDI TEMP_H,0x01
	LDI TEMP_L,0x01
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE PC+0x02
	LDD ACCUM,Z+0x00

	POP_T16
	RET

;--------------------------------------------------------
_DRV_MCP23017_I2C_WRITE:
;--------------------------------------------------------
;IN:LOOP_CNTR-номер регистра, ACCUM-значение, Z-адрес на
;I2C буфер
;OUT:TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_T16
	PUSH ACCUM

	STD Z+0x00,LOOP_CNTR
	STD Z+0x01,ACCUM

	LDD ACCUM,Y+_DRV_MCP23017_I2C_ADDR
	LDD TEMP,Y+_DRV_MCP23017_I2C_DRV_ID
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	POP ACCUM
	POP_T16
	RET

.ENDIF



