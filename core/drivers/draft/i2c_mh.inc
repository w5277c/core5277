;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.11.2024	konstantin@5277.ru		За основу взят i2c_h.inc(не тестировано)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_I2C_MH
.else
.set DEF__C5_DRV_I2C_MH = 1
.message "Included driver I2C(hardware) master v0.1"

.include "./core/wait.inc"
.include	"./core/drivers/_i2c.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_I2C_MH_ADDR						= 0x00			;1B-Адрес подчиненного устройства
	.EQU	_DRV_I2C_MH_LED_PORT					= 0x01			;1B-Порт индикатора активности
	.EQU	_DRV_I2C_MH_POS						= 0x02			;1B-Позиция в буфере
	.EQU	_DRV_I2C_MH_TX_LEN					= 0x03			;1B-Размер данных передачи
	.EQU	_DRV_I2C_MH_RX_LEN					= 0x04			;1B-Размер данных приема
	.EQU	_DRV_I2C_MH_BUFFER_SIZE				= 0x05			;1B-Размер буфера
	.EQU	_DRV_I2C_MH_STATE						= 0x06			;1B-Статус
	.EQU	_DRV_I2C_MH_BUFFER					= 0x07			;xB-Буфер

;--------------------------------------------------------
DRV_I2C_MH_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-FREQ, TEMP_L-размер буфера
;(1<TEMP_EL<0x100-_DRV_I2C_SH_RAM_SIZE),
;FLAGS-порт инд. передачи(0xff-не исп)
;--------------------------------------------------------
	;Подаем питание на TWI
	_C5_POWER_ON PRTWI

	STS TWAR,C0x00

	;Выделяем память
	LDI ACCUM,_DRV_I2C_MH_BUFFER
	ADD ACCUM,TEMP_L
	MCALL C5_RAM_REALLOC

	;Инициализируем порт LED
	STD Y+_DRV_I2C_MH_LED_PORT,FLAGS
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO

	;Настройка порта SDA
	LDI ACCUM,SDA
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	;Настройка порта SCL
	LDI ACCUM,SCL
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO													;Точно LO?

	;Задаем частоту
	STS TWBR,XL
	;Prescaller x1
	LDI TEMP,(0<<TWPS1)|(0<<TWPS0)
	STS TWSR,TEMP

	LDI TEMP_H,high(_DRV_I2C_MH_EVENT)
	LDI TEMP_L,low(_DRV_I2C_MH_EVENT)
	LDI ACCUM,C5_IR_TWI
	MCALL C5_IR_VECTOR_SET

	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;IN: ACCUM-адрес устройства, TEMP_H-длина данных на
;передачу,TEMP_L-длина данных для приема.
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Y
	PUSH_T16
	PUSH ACCUM

	MCALL C5_RAM_OFFSET

	;Записываем адрес подчиненного
	STD Y+_DRV_I2C_MH_ADDR,ACCUM
	;Записываем длину данных для передачи
	STD Y+_DRV_I2C_MH_TX_LEN,TEMP_H
	;Записываем длину данных для приема
	STD Y+_DRV_I2C_MH_RX_LEN,TEMP_L
	;Обнуляем позицию в блоке данных
	STD Y+_DRV_I2C_MH_POS,C0x00
	;Записываем состояние(передачи или приема если нечего передавать)
	LDI TEMP,DRV_I2C_STATE_TR
	CPI TEMP_H,0x00
	BRNE PC+0x02
	LDI TEMP,DRV_I2C_STATE_RC
	STD Y+_DRV_I2C_MH_STATE,TEMP

	;Включаю индикатор активности I2C
	LDD ACCUM,Y+_DRV_I2C_MH_LED_PORT
	MCALL PORT_SET_HI

	;Выключаю индикатор активности I2C_H
	LDD ACCUM,Y+_DRV_I2C_MH_LED_PORT
	MCALL PORT_SET_LO
	;Получаю результат операции
	LDD TEMP,Y+_DRV_I2C_MH_STATE

	LDI TEMP,(1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
_DRV_I2C_MH__PROC_WAIT_LOOP:
	LDD TEMP,Y+_DRV_I2C_MH_STATE
	CPI TEMP,0x80
	BRCC PC+0x01+_MCALL_SIZE+0x01
	MCALL C5_WAIT
	RJMP _DRV_I2C_MH__PROC_WAIT_LOOP

	LDI TEMP,(0<<TWEN)
	STS TWCR,TEMP

	LDD ACCUM,Y+_DRV_I2C_MH_LED_PORT
	MCALL PORT_SET_LO

	POP ACCUM
	POP_T16
	POP_Y
	RET

;--------------------------------------------------------
_DRV_I2C_MH_EVENT:
;--------------------------------------------------------
;Событие от шины
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM
	PUSH_Y
	PUSH_Z

	MCALL C5_RAM_OFFSET

	;Читаем состояние
	LDD TEMP,Y+_DRV_I2C_MH_STATE
	CPI TEMP,0x80
	BRCS PC+0x05
	LDI TEMP,(0<<TWEN)
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END

	;Получаем состояние TWI
	LDS TEMP,TWSR
	ANDI TEMP,0xf8

	;TWI проверка на старт передачи или приема
	CPI TEMP,TWI_MTR_START
	BREQ _DRV_I2C_MH_EVENT__START
	CPI TEMP,TWI_MTR_NEXT_START
	BRNE _DRV_I2C_MH_EVENT__NO_START
_DRV_I2C_MH_EVENT__START:
	LDD ACCUM,Y+_DRV_I2C_MH_ADDR
	LDD TEMP,Y+_DRV_I2C_MH_STATE
	CPI TEMP,DRV_I2C_STATE_RC
	BRNE PC+0x02
	ORI ACCUM,0x01
	STS TWDR,ACCUM
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__NO_START:
	;TWI проверка на передачу байта
	CPI TEMP,TWI_MT_SLA_W_ACK
	BREQ _DRV_I2C_MH_EVENT__TR_NEXT
	CPI TEMP,TWI_MT_DATA_ACK
	BRNE _DRV_I2C_MH_EVENT__NO_TR_NEXT
_DRV_I2C_MH_EVENT__TR_NEXT:
	;Считываем позицию в блоке данных
	LDD TEMP,Y+_DRV_I2C_MH_POS
	MOVW ZL,YL
	ADD ZL,TEMP
	ADC ZH,C0x00
	LD ACCUM,Z
	STS TWDR,ACCUM
	;Проверяем на конец передачи
	LDD ACCUM,Y+_DRV_I2C_MH_TX_LEN
	CP TEMP,ACCUM
	BREQ _DRV_I2C_MH_EVENT__RC
	;Инкрементируем и записываем новую позицию в блоке данных
	INC TEMP
	STD Y+_DRV_I2C_MH_POS,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__RC:
	;Все данные переданы, обнуляем смещение
	STD Y+_DRV_I2C_MH_POS,C0x00
	;Проверяем на необходимость принять данные
	LDD TEMP,Y+_DRV_I2C_MH_RX_LEN
	CPI TEMP,0x00
	BREQ _DRV_I2C_MH_EVENT__SKIP_RC
	;Переходим в режим приема
	LDI TEMP,DRV_I2C_STATE_RC
	STD Y+_DRV_I2C_MH_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTA)		;START
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__SKIP_RC:
	;Все данные отправлены, прием не нужен
	LDI TEMP,DRV_RESULT_OK
	STD Y+_DRV_I2C_MH_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO)		;STOP
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__NO_TR_NEXT:
	;TWI проверка на прием байта
	CPI TEMP,TWI_MR_SLA_R_ACK
	BREQ _DRV_I2C_MH_EVENT__RC_ACK
	CPI TEMP,TWI_MR_ACK
	BRNE _DRV_I2C_MH_EVENT__NO_RC_NEXT
_DRV_I2C_MH_EVENT__RC_NEXT:
	;Считываем позицию в блоке данных
	LDD TEMP,Y+_DRV_I2C_MH_POS
	MOVW ZL,YL
	ADD ZL,TEMP
	ADC ZH,C0x00
	LDS ACCUM,TWDR
	ST Z,ACCUM
	;Инкрементируем позицию
	INC TEMP
	;Проверяем на конец приема
	LDD ACCUM,Y+_DRV_I2C_MH_RX_LEN
	CP TEMP,ACCUM
	BREQ _DRV_I2C_MH_EVENT__RC_NEXT_END
	;Записываем новую позицию в блоке данных
	STD Y+_DRV_I2C_MH_POS,TEMP
_DRV_I2C_MH_EVENT__RC_ACK:
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA)		;ACK
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__RC_NEXT_END:
	;Все данные получены
	LDI TEMP,DRV_RESULT_OK
	STD Y+_DRV_I2C_MH_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)					;NACK
	STS TWCR,TEMP
	RJMP _DRV_I2C_MH_EVENT__END
_DRV_I2C_MH_EVENT__NO_RC_NEXT:
	;Ошибка
	LDI TEMP,DRV_RESULT_ERROR
	STD Y+_DRV_I2C_MH_STATE,TEMP
	LDI TEMP,(1<<TWSTO)
	STS TWCR,TEMP
_DRV_I2C_MH_EVENT__END:
	POP_Z
	POP_Y
	POP ACCUM
	POP TEMP
	RET
.endif
