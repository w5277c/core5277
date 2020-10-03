;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;31.05.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO судя по дампу шины, TWI читает на один байт больше, хотя по коду все ок.
;Как будто данные в TWDR появляются с опозданием в 1 байт
;Возможно причина в TWI_MR_SLA_R_ACK я его пропускаю, если не пропускать то в TWDR получаю 0xd1

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/io/port_mode_in.inc"
.include	"./inc/io/port_mode_out.inc"
.include	"./inc/io/port_set_lo.inc"
.include	"./inc/io/port_set_hi.inc"
.include	"./inc/core/wait.inc"

;---CONSTANTS--------------------------------------------
	;---FREQ---
	.EQU	DRV_I2C_FREQ_50KHZ					= 0x98			;for 50kHz  ---(16000000/50000  - 16)/(2*4^0) = 152
	.EQU	DRV_I2C_FREQ_100KHZ					= 0x48			;for 100kHz ---(16000000/100000 - 16)/(2*4^0) = 72
	.EQU	DRV_I2C_FREQ_200KHZ					= 0x20			;for 200kHz ---(16000000/200000 - 16)/(2*4^0) = 32
	.EQU	DRV_I2C_FREQ_400KHZ					= 0x0c			;for 400kHz ---(16000000/400000 - 16)/(2*4^0) = 12
	;---DRIVER-STATES---
	.EQU	DRV_I2C_ST_DONE						= 0x00
	.EQU	DRV_I2C_ST_TR							= 0x01
	.EQU	DRV_I2C_ST_RC							= 0x02
	;---TWI-STATES---
	.EQU	TWI_MTR_START							= 0x08			;M-master,S-slave,T-transfer,R-receive
	.EQU	TWI_MTR_NEXT_START					= 0x10
	.EQU	TWI_MT_SLA_W_ACK						= 0x18
	.EQU	TWI_MT_SLA_W_NACK						= 0x20
	.EQU	TWI_MT_DATA_ACK						= 0x28
	.EQU	TWI_MT_DATA_NACK						= 0x30
	.EQU	TWI_MTR_ARBIT_LOST					= 0x38
	.EQU	TWI_MR_SLA_R_ACK						= 0x40
	.EQU	TWI_MR_SLA_R_NACK						= 0x48
	.EQU	TWI_MR_ACK								= 0x50
	.EQU	TWI_MR_NACK								= 0x58
	;---STATES---
	.EQU	DRV_I2C_STATE_TR						= 0x01
	.EQU	DRV_I2C_STATE_RC						= 0x02
	.EQU	DRV_I2C_STATE_OK						= 0x80
	.EQU	DRV_I2C_STATE_ERROR					= 0x81
	;---VARS---
	.EQU	_DRV_I2C_ADDR							= 0x00			;1B - адрес I2C(0 - режим мастера)
	.EQU	_DRV_I2C_LED_PORT						= 0x01			;1B - порт индикатора активности
	.EQU	_DRV_I2C_POSITION						= 0x02			;1B - позиция в блоке данных
	.EQU	_DRV_I2C_TX_LEN						= 0x03			;1B - размер данных для передачи
	.EQU	_DRV_I2C_RX_LEN						= 0x04			;1B - размер данных для приема
	.EQU	_DRV_I2C_SRC_ADDR						= 0x05			;2b - адрес блока данных для передачи
	.EQU	_DRV_I2C_DST_ADDR						= 0x07			;2b - адрес блока данных для приема
	.EQU	_DRV_I2C_STATE							= 0x09			;1b - состояние/результат
	;---
	.EQU	_DRV_I2C_RAM_SIZE						= 0x0a
;--------------------------------------------------------
DRV_I2C_INIT:
;--------------------------------------------------------
;Инициализация
;IN: XH - I2C адрес(для режима ведомого), XL - FREQ,
;YH - порт инд. передачи(0xff-не исп)
;--------------------------------------------------------
	;Подаем питание на TWI
	_CORE5277_PRR_RESET PRTWI

	;Записываем в TWAR I2C адрес (в режиме ведомого !=0)
	LSL XH
	STS TWAR,XH

	LDI ACCUM,_DRV_I2C_RAM_SIZE
	MCALL CORE5277_RAM_REALLOC

	;Инициализируем порт LED
	STD Z+_DRV_I2C_LED_PORT,YH
	MOV ACCUM,YH
	CPI ACCUM,0xff
	BREQ PC+0x07
	MCALL CORE5277_PORT_MODE_OUT
	MCALL CORE5277_PORT_SET_LO

	;Настройка порта SDA
	LDI ACCUM,SDA
	MCALL CORE5277_PORT_MODE_IN
	MCALL CORE5277_PORT_SET_LO

	CPI XH,0x00
	BRNE _DRV_I2C_INIT__SLAVE_MODE

	;Настройка порта SCL(master mode)
	LDI ACCUM,SCL
	MCALL CORE5277_PORT_MODE_OUT
	MCALL CORE5277_PORT_SET_LO
	RJMP _DRV_I2C_INIT__DONE

_DRV_I2C_INIT__SLAVE_MODE:
	;Настройка порта SCL(slave mode)
	LDI ACCUM,SCL
	MCALL CORE5277_PORT_MODE_IN
	MCALL CORE5277_PORT_SET_LO

_DRV_I2C_INIT__DONE:
	;Задаем частоту
	STS TWBR,XL
	;Prescaller x1
	LDI TEMP,(0<<TWPS1)|(0<<TWPS0)
	STS TWSR,TEMP

	LDI TEMP_H,high(_DRV_I2C_EVENT)
	LDI TEMP_L,low(_DRV_I2C_EVENT)
	LDI ACCUM,CORE5277_IR_TWI
	MCALL CORE5277_IR_VECTOR_SET

	MCALL CORE5277_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;ACCUM - адрес устройства(0 - в режиме ведомого)
;TEMP_H - длина данных на запись,
;TEMP_L - длина данных для чтения.
;X - адрес на блок для чтения
;OUT: TEMP - STATE
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	MCALL CORE5277_RAM_OFFSET

	;Записываем адрес устройства(режим ведущего)
	LSL ACCUM
	STD Z+_DRV_I2C_ADDR,ACCUM
	;Записываем длину данных для передачи
	STD Z+_DRV_I2C_TX_LEN,TEMP_H
	;Записываем длину данных для приема
	STD Z+_DRV_I2C_RX_LEN,TEMP_L
	;Обнуляем позицию в блоке данных
	CLR TEMP
	STD Z+_DRV_I2C_POSITION,TEMP
	;Записываем адрес данных для передачи
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_I2C_SRC_ADDR+0x00,YH
	STD Z+_DRV_I2C_SRC_ADDR+0x01,YL
	;Записываем адрес данных для приема
	STD Z+_DRV_I2C_DST_ADDR+0x00,XH
	STD Z+_DRV_I2C_DST_ADDR+0x01,XL
	;Записываем состояние(передачи или приема если нечего передавать)
	LDI TEMP,DRV_I2C_STATE_TR
	CPI TEMP_H,0x00
	BRNE PC+0x02
	LDI TEMP,DRV_I2C_STATE_RC
	STD Z+_DRV_I2C_STATE,TEMP

	;Включаю индикатор активности I2C
	LDD ACCUM,Z+_DRV_I2C_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x05
	MCALL CORE5277_PORT_SET_HI
	;Смотрим режим по ACCUM
	LDS TEMP,TWAR
	CPI TEMP,0x00
	BRNE PC+0x04
	MCALL _DRV_I2C_MASTER_PROC
	RJMP _DRV_I2C_PROC_END
	MCALL _DRV_I2C_SLAVE_PROC
_DRV_I2C_PROC_END:

	;Выключаю индикатор активности I2C
	LDD ACCUM,Z+_DRV_I2C_LED_PORT
	CPI ACCUM,0xff
	BREQ PC+0x05
	MCALL CORE5277_PORT_SET_LO
	;Получаю результат операции
	LDD TEMP,Z+_DRV_I2C_STATE

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET

;--------------------------------------------------------
;Основной код, коммуникация(режим ведущего)
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;TEMP_H - длина данных на запись,
;TEMP_L - длина данных для чтения.
;X - адрес на блок для чтения
;Z - адрес на переменные
;OUT: TEMP - 0x00-успех
;--------------------------------------------------------
_DRV_I2C_MASTER_PROC:
	LDI TEMP,(1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
_DRV_I2C_MASTER_PROC__WAIT_LOOP:
	LDD TEMP,Z+_DRV_I2C_STATE
	SBRC TEMP,0x07
	RJMP _DRV_I2C_MASTER_PROC__DONE
	MCALL CORE5277_WAIT
	RJMP _DRV_I2C_MASTER_PROC__WAIT_LOOP
_DRV_I2C_MASTER_PROC__DONE:
	LDI TEMP,(0<<TWEN)
	STS TWCR,TEMP
	RET

;--------------------------------------------------------
;Основной код, коммуникация(режим ведомого)
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;TEMP_EH - длина данных на запись,
;TEMP_EL - длина данных для чтения.
;X - адрес на блок для чтения
;Z - адрес на переменные
;OUT: TEMP - 0x00-успех
;--------------------------------------------------------
_DRV_I2C_SLAVE_PROC:
	RET


;--------------------------------------------------------
_DRV_I2C_EVENT:
;--------------------------------------------------------
;Событие от шины
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Y
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L

	;;CLI
	MCALL CORE5277_RAM_OFFSET

	;Читаем состояние
	LDD TEMP,Z+_DRV_I2C_STATE
	SBRS TEMP,0x07
	RJMP PC+0x05
	LDI TEMP,(0<<TWEN)
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END

	;Получаем состояние TWI
	LDS TEMP,TWSR
	ANDI TEMP,0xf8

	;TWI проверка на старт передачи или приема
	CPI TEMP,TWI_MTR_START
	BREQ _DRV_I2C_EVENT__START
	CPI TEMP,TWI_MTR_NEXT_START
	BREQ _DRV_I2C_EVENT__START
	RJMP _DRV_I2C_EVENT__NO_START
_DRV_I2C_EVENT__START:
	LDD ACCUM,Z+_DRV_I2C_ADDR
	LDD TEMP,Z+_DRV_I2C_STATE
	CPI TEMP,DRV_I2C_STATE_RC
	BRNE PC+0x02
	ORI ACCUM,0x01
	STS TWDR,ACCUM
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__NO_START:
	;TWI проверка на передачу байта
	CPI TEMP,TWI_MT_SLA_W_ACK
	BREQ _DRV_I2C_EVENT__TR_NEXT
	CPI TEMP,TWI_MT_DATA_ACK
	BREQ _DRV_I2C_EVENT__TR_NEXT
	RJMP _DRV_I2C_EVENT__NO_TR_NEXT
_DRV_I2C_EVENT__TR_NEXT:
	;Считываем позицию в блоке данных
	LDD TEMP_L,Z+_DRV_I2C_POSITION
	;Считываем адрес данных для передачи
	LDD YH,Z+_DRV_I2C_SRC_ADDR+0x00
	LDD YL,Z+_DRV_I2C_SRC_ADDR+0x01
	MOV ACCUM,YH
	ANDI YH,0x7f
	;Добавляем смещение
	ADD YL,TEMP_L
	CLR TEMP
	ADC YH,TEMP
	;Читаем байт с памяти и передаем
	SBRC ACCUM,0x07
	RJMP PC+0x03
	LD TEMP,Y
	RJMP PC+0x08
	PUSH_Z
	MOV ZH,YH
	MOV ZL,YL
	LPM TEMP,Z
	POP_Z
	STS TWDR,TEMP
	;Проверяем на конец передачи
	LDD TEMP,Z+_DRV_I2C_TX_LEN
	CP TEMP,TEMP_L
	BREQ _DRV_I2C_EVENT__RC
	;Инкрементируем и записываем новую позицию в блоке данных
	INC TEMP_L
	STD Z+_DRV_I2C_POSITION,TEMP_L
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__RC:
	;Все данные переданы, обнуляем смещение
	CLR TEMP
	STD Z+_DRV_I2C_POSITION,TEMP
	;Проверяем на необходимость принять данные
	LDD TEMP,Z+_DRV_I2C_RX_LEN
	CPI TEMP,0x00
	BREQ _DRV_I2C_EVENT__SKIP_RC
	;Переходим в режим приема
	LDI TEMP,DRV_I2C_STATE_RC
	STD Z+_DRV_I2C_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTA)		;START
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__SKIP_RC:
	;Все данные отправлены, прием не нужен
	LDI TEMP,DRV_I2C_STATE_OK
	STD Z+_DRV_I2C_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO)		;STOP
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__NO_TR_NEXT:
	;TWI проверка на прием байта
	CPI TEMP,TWI_MR_SLA_R_ACK
	BREQ _DRV_I2C_EVENT__RC_ACK
	CPI TEMP,TWI_MR_ACK
	BREQ _DRV_I2C_EVENT__RC_NEXT
	RJMP _DRV_I2C_EVENT__NO_RC_NEXT
_DRV_I2C_EVENT__RC_NEXT:
	;Считываем позицию в блоке данных
	LDD TEMP_L,Z+_DRV_I2C_POSITION
	;Считываем адрес данных для приема
	LDD YH,Z+_DRV_I2C_DST_ADDR+0x00
	LDD YL,Z+_DRV_I2C_DST_ADDR+0x01
	;Добавляем смещение
	ADD YL,TEMP_L
	CLR TEMP
	ADC YH,TEMP
	LDS TEMP,TWDR
	ST Y,TEMP
	;Инкрементируем позицию
	INC TEMP_L
	;Проверяем на конец приема
	LDD TEMP,Z+_DRV_I2C_RX_LEN
	CP TEMP,TEMP_L
	BREQ _DRV_I2C_EVENT__RC_NEXT_END
	;Записываем новую позицию в блоке данных
	STD Z+_DRV_I2C_POSITION,TEMP_L
_DRV_I2C_EVENT__RC_ACK:
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA)		;ACK
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__RC_NEXT_END:
	;Все данные получены
	LDI TEMP,DRV_I2C_STATE_OK
	STD Z+_DRV_I2C_STATE,TEMP
	LDI TEMP,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)					;NACK
	STS TWCR,TEMP
	RJMP _DRV_I2C_EVENT__END
_DRV_I2C_EVENT__NO_RC_NEXT:
	;Ошибка
	LDI TEMP,DRV_I2C_STATE_ERROR
	STD Z+_DRV_I2C_STATE,TEMP
	LDI TEMP,(1<<TWSTO)
	STS TWCR,TEMP
_DRV_I2C_EVENT__END:
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_Y
	POP TEMP
	RET
