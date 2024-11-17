;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.11.2024	konstantin@5277.ru		Начало
;-----------------------------------------------------------------------------------------------------------------------
;Расширитель портов, с I2C входом
;-----------------------------------------------------------------------------------------------------------------------
;Поддерживаемые операции DRV_OP_GET, DRV_OP_SET
;-----------------------------------------------------------------------------------------------------------------------
;TODO процедуры инициализации МК, процедуры инициализации прерываний, и их обработчики

.ifdef DEF__C5_DRV_MCP23017
.else
.set DEF__C5_DRV_MCP23017 = 1
.message "Included driver MCP23017 v0.1"

;.include "./core/drivers/_io_expander.inc"


;---CONSTANTS--------------------------------------------
	.EQU	_DRV_MCP23017_PORTA							= 0x00	;Номер порта A
	.EQU	_DRV_MCP23017_PORTB							= 0x01	;Номер порта B
	;---
	.EQU	_DRV_MCP23017_IODIR							= 0x00	;IODIR-регистр направления(0-выход)
	.EQU	_DRV_MCP23017_IPOL							= 0x01	;IPOL-регистр инвертирования(1-инвертируем)
	.EQU	_DRV_MCP23017_GPINTEN						= 0x02	;GPINTEN-регистр разрешения прерываний(1-прерывания разрешены)
	.EQU	_DRV_MCP23017_DEFVAL							= 0x03	;DEFVAL-значение для сравнения, с последующим прерыванием при совпадении
	.EQU	_DRV_MCP23017_INTCON							= 0x04	;INTCON-режим прерываний(сравниваем:0-с предыдущим значением,1-с DEFVAL)
	.EQU	_DRV_MCP23017_IOCON							= 0x05	;IOCONF-регистр конфигурации
	.EQU		_DRV_MCP23017_IOCON_BANK				= 0x00	;BANK-режим работы с блоками(сортировк:0-по регистрам, 1-по портам)
	.EQU		_DRV_MCP23017_IOCON_MIRROR				= 0x01	;MIRROR-объединение порта INTA и INTB(0-соединены друг с другом, 1-независимы)
	.EQU		_DRV_MCP23017_IOCON_SEQOP				= 0x02	;SEQOP-инкрементация адреса регистров(0-инкрементация включена,1-адрес не меняется)
	.EQU		_DRV_MCP23017_IOCON_DISSLW				= 0x03	;DISSLW-скорсоть наростания порта SDA(пока не понятно)
	.EQU		_DRV_MCP23017_IOCON_ODR					= 0x05	;ODR-Редим вывода INT(0-отвечает INPOL,1-вывод отключен)
	.EQU		_DRV_MCP23017_IOCON_INTPOL				= 0x06	;INPOL-полярность вывода INT(активен:0-низкий,1-высокий)
	.EQU	_DRV_MCP23017_GPPUA							= 0x06	;GPPUA-регистр подтяжки 100К к VCC(0-отключен)
	.EQU	_DRV_MCP23017_INTF							= 0x07	;INTF-регистр флагов прерывания(0-пин не участвует в прерывании), только для чтения
	.EQU	_DRV_MCP23017_INTCAP							= 0x08	;INTCAP-регистр значения GPIO в момент прерывания(0-низкий уровень), только для чтения, сбрасывает прерывание.
	.EQU	_DRV_MCP23017_GPIO							= 0x09	;GPIO-регистр значения порта(0-низкий уровень), чтение сбрасывает прерывание.
	.EQU	_DRV_MCP23017_OLAT							= 0x0a	;OLAT-регистр для записи значения в GPIO в режиме входа, прменяются при переходе в режим выхода
	;---
	.EQU	_DRV_MCP23017_I2C_ADDR						= 0x00	;1Б-адресс устройства
	.EQU	_DRV_MCP23017_I2C_DRV_ID					= 0x01	;1Б-ИД драйвера I2C master
	.EQU	_DRV_MCP23017_I2C_BUFFER					= 0x02	;2Б-адрес выделенной памяти в I2C драйвере
	;---
	.EQU	_DRV_MCP23017_RAM_SIZE						= 0x04

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

	PUSH_Z
	PUSH_Y
	PUSH PID
	MOV PID,TEMP_L
	MCALL C5_RAM_OFFSET
	MOVW ZL,YL
	POP PID
	POP_Y
	STD Y+_DRV_MCP23017_I2C_BUFFER+0x00,ZH
	STD Y+_DRV_MCP23017_I2C_BUFFER+0x01,ZL						;TODO оптимизировать
	POP_Z

	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET
	LDD ZH,Y+_DRV_MCP23017_I2C_BUFFER+0x00
	LDD ZL,Y+_DRV_MCP23017_I2C_BUFFER+0x01

	CPI FLAGS,DRV_OP_GET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_MCP23017_GET
	RJMP _DRV_MCP23017_PROC__END
	CPI FLAGS,DRV_OP_SET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL_DRV_MCP23017_SET
	RJMP _DRV_MCP23017_PROC__END
	LDI TEMP,_DRV_RESULT_UNSUPPORTED

_DRV_MCP23017_PROC__END:
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
_DRV_MCP23017_GET:
;--------------------------------------------------------
;Получаем значение от MCP23017(GPIO,IODIR A/B)
;IN: TEMP_H-номер порта(_DRV_MCP23017_PORT...),
;TEMP_L-тип порта(VPORT_REG_...), Y-адрес на переменные,
;Z-адрес на I2C буфер
;OUT: TEMP-код результата DRV_RESULT_..., ACCUM-значение
;--------------------------------------------------------
	PUSH FLAGS
	PUSH TEMP_H

	SWAP TEMP_H

	CPI TEMP_L,VPORT_REG_PORT
	BREQ PC+0x03
	CPI TEMP_L,VPORT_REG_PIN
	BRNE _DRV_MCP23017_GET__NO_PORT
	LDI FLAGS,_DRV_MCP23017_GPIO
	OR FLAGS,TEMP_H
	RCALL _DRV_MCP23017_I2C_READ
	RJMP _DRV_MCP23017_GET__END
_DRV_MCP23017_GET__NO_PORT:
	CPI TEMP_L,VPORT_REG_DDR
	BRNE _DRV_MCP23017_GET__NO_DDR
	LDI FLAGS,_DRV_MCP23017_IODIR
	OR FLAGS,TEMP_H
	RCALL _DRV_MCP23017_I2C_READ
	RJMP _DRV_MCP23017_GET__END
_DRV_MCP23017_GET__NO_DDR:
	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_MCP23017_GET__END:
	POP TEMP_H
	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_MCP23017_SET:
;--------------------------------------------------------
;Записываем значение в MCP23017(GPIO,IODIR A/B)
;IN: ACCUM-значение, TEMP_H-номер порта
;(_DRV_MCP23017_PORT...), TEMP_L-тип порта
;(VPORT_REG_...), Y-адрес на переменные, Z-адрес на I2C
;буфер
;OUT: TEMP-код результата DRV_RESULT_...
;--------------------------------------------------------
	PUSH FLAGS
	PUSH TEMP_H
	PUSH ACCUM

	SWAP TEMP_H

	CPI TEMP_L,VPORT_REG_PORT
	BRNE _DRV_MCP23017_SET__NO_PORT
	LDI FLAGS,_DRV_MCP23017_GPIO
	OR FLAGS,TEMP_H
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_GET__END
_DRV_MCP23017_SET__NO_PORT:
	CPI TEMP_L,VPORT_REG_DDR
	BRNE _DRV_MCP23017_SET__NO_DDR
	LDI FLAGS,_DRV_MCP23017_IODIR
	OR FLAGS,TEMP_H
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_GET__END
_DRV_MCP23017_GET__NO_DDR:
	CPI TEMP_L,VPORT_REG_PIN
	BRNE _DRV_MCP23017_SET__NO_PIN
	LDI FLAGS,_DRV_MCP23017_IPOL
	OR FLAGS,TEMP_H
	RCALL _DRV_MCP23017_I2C_WRITE
	RJMP _DRV_MCP23017_GET__END
_DRV_MCP23017_SET__NO_PIN:
	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_MCP23017_SET__END:
	POP ACCUM
	POP TEMP_H
	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_MCP23017_I2C_READ:
;--------------------------------------------------------
;IN:FLAGS-номер регистра, Z-адрес на I2C буфер
;OUT:TEMP-DRV_RESULT_..., ACCUM-значение
;--------------------------------------------------------
	PUSH_T16

	STD Z+0x00,FLAGS

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
;IN:FLAGS-номер регистра, ACCUM-значение, Z-адрес на
;I2C буфер
;OUT:TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_T16
	PUSH ACCUM

	STD Z+0x00,FLAGS
	STD Z+0x01,ACCUM

	LDD ACCUM,Y+_DRV_MCP23017_I2C_ADDR
	LDD TEMP,Y+_DRV_MCP23017_I2C_DRV_ID
	LDI TEMP_H,0x02
	LDI TEMP_L,0x00
	MCALL C5_EXEC

	POP ACCUM
	POP_T16
	RET





