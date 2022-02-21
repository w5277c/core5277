;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.10.2020	w5277c@gmail.com			Начало, черновой вариант
;12.10.2020	w5277c@gmail.com			Первый успешный тест для I2C
;22.02.2022	w5277c@gmail.com			Частично успешный тест для 4bit шины(необходима большая пауза, почему?)
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер символьных LCD экранов на базе HD44780 с поддержкой PCF8574(I2C интерфейс)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_HD44780
.else
.set DEF__C5_DRV_HD44780 = 1
.message "Included driver HD44780 v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/drivers/_video.inc"
.include "./io/port_mode_out.inc"
.include "./io/port_set_hi.inc"
.include "./io/port_set_lo.inc"
.include "./io/port_set_byte.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_HD44780_PCF8574_I2C_ADDR			= 0x27		;Адрес устройства на шине I2C
	;---FLAGS---
	.EQU	DRV_HD44780_FL_C							= 0x00		;Отображение курсора: 0-нет,1-да
	.EQU	DRV_HD44780_FL_B							= 0x01		;Мигание: 0-нет,1-да
	.EQU	DRV_HD44780_FL_SC							= 0x02		;Автоматический сдвиг: 0-курсор,1-дисплей
	.EQU	DRV_HD44780_FL_RL							= 0x03		;Напраление авт. сдвига: 0-влево,1-вправо
	.EQU	DRV_HD44780_FL_DL							= 0x04		;Ширина шины: 0-4б, 1-8б
	.EQU	DRV_HD44780_FL_N							= 0x05		;Кол-во строк: 0-1,1-2
	.EQU	DRV_HD44780_FL_F							= 0x06		;Размер шрифта: 0-5x8,1-5x10
	.EQU	DRV_HD44780_FL_I2C						= 0x07		;Тип шины: 0-прямое подключение,1-I2C (через PCF8574)
	;---I2C-BITS---
	.EQU	DRV_HD44780_I2C_BL						= 0x03		;Подсветка
	.EQU	DRV_HD44780_I2C_E							= 0x02		;Доступность шины(1-доступна)
	.EQU	DRV_HD44780_I2C_RW						= 0x01		;Чтение/запись(0-запись)
	.EQU	DRV_HD44780_I2C_RS						= 0x00		;Команда/данные(0-команда)
	;---OFFSETS---
	.EQU	_DRV_HD44780_I2C_BUFFER					= 0x00		;1B состояние пинов управления 3-BL,2-E,1-RW,0-RS(для I2C)
	.EQU	_DRV_HD44780_PORT							= 0x01		;1B ид драйвера I2C или P[A/B/C/D/E/F...]0
	.EQU	_DRV_HD44780_RS_PORT						= 0x02		;1B RS PORT
	.EQU	_DRV_HD44780_E_PORT						= 0x03		;1B E PORT
	.EQU	_DRV_HD44780_FLAGS						= 0x04		;1B флаги
	.EQU	_DRV_HD44780_RAM_SIZE					= 0x05
;--------------------------------------------------------
DRV_HD44780_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-PORT или ид драйвера I2C, TEMP_H-прот RS,
;TEMP_L-порт E, FLAGS-параметры(DRV_HD44780_FL_...)
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_HD44780_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	STD Y+_DRV_HD44780_PORT,ACCUM
	STD Y+_DRV_HD44780_RS_PORT,TEMP_H
	STD Y+_DRV_HD44780_E_PORT,TEMP_L
	STD Y+_DRV_HD44780_FLAGS,FLAGS

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP _DRV_HD44780_INIT__PORTS_SKIP
	;Инициализирую 4 или 8 портов для данных
	ANDI ACCUM,0xf0
	LDI LOOP_CNTR,0x08
_DRV_HD44780_INIT__LNIBBLE_LOOP:
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	INC ACCUM
	CPI LOOP_CNTR,0x05
	BRNE PC+0x03
	SBRS FLAGS,DRV_HD44780_FL_DL
	RJMP _DRV_HD44780_INIT__LNIBBLE_LOOP_END
	DEC LOOP_CNTR
	BRNE _DRV_HD44780_INIT__LNIBBLE_LOOP
_DRV_HD44780_INIT__LNIBBLE_LOOP_END:
	;Инициализация RS
	MOV ACCUM,TEMP_H
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	;Инициализация E
	MOV ACCUM,TEMP_L
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
_DRV_HD44780_INIT__PORTS_SKIP:

	STD Y+_DRV_HD44780_I2C_BUFFER,C0x00

	MCALL C5_READY
;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции,
;OUT: TEMP-код ответа(DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_VIDEO_OP_INIT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_INIT
	RJMP _DRV_HD44780__END

	CPI FLAGS,DRV_VIDEO_OP_GET_MODES
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_GET_MODES
	RJMP _DRV_HD44780__END

	CPI FLAGS,DRV_VIDEO_OP_GET_MODE
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_GET_MODE
	RJMP _DRV_HD44780__END

;	CPI FLAGS,DRV_VIDEO_OP_GET_SIZE
;	BRNE PC+0x01+_MCALL_SIZE+0x01
;	MCALL _DRV_BUS5277_PROC_MASTER
;	RJMP _DRV_HD44780__END

	CPI FLAGS,DRV_VIDEO_OP_SET_XY
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_SET_XY
	RJMP _DRV_HD44780__END

	CPI FLAGS,DRV_VIDEO_OP_SET_SYMBOL
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_SET_SYMBOL
	RJMP _DRV_HD44780__END

	CPI FLAGS,DRV_VIDEO_OP_SET_TEXT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_HD44780_PROC_SET_TEXT
	RJMP _DRV_HD44780__END

	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_HD44780__END:
	POP_Y
	RET

;--------------------------------------------------------
_DRV_HD44780_PROC_INIT:
;--------------------------------------------------------
;Подготавливем экран к работе
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH FLAGS
	PUSH ACCUM

	LDD FLAGS,Y+_DRV_HD44780_FLAGS

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,8															;>15ms
	MCALL C5_WAIT_2MS
	LDI ACCUM,0x30
	MCALL _DRV_HD44780__SEND_CMDNIBBLE
	LDI TEMP,3															;>4.1ms
	MCALL C5_WAIT_2MS
	MCALL _DRV_HD44780__SEND_CMDNIBBLE
	LDI TEMP,1															;>100us
	MCALL C5_WAIT_2MS
	MCALL _DRV_HD44780__SEND_CMDNIBBLE

	SBRC FLAGS,DRV_HD44780_FL_DL
	RJMP _DRV_HD44780_PROC_INIT__8BIT_MODE
	;4 BIT MODE
	LDI ACCUM,0x20
	MCALL _DRV_HD44780__SEND_CMDNIBBLE
_DRV_HD44780_PROC_INIT__8BIT_MODE:

	;FUNCTION SET
	LDI ACCUM,0x20
	SBRC FLAGS,DRV_HD44780_FL_DL
	ORI ACCUM,0x10
	SBRC FLAGS,DRV_HD44780_FL_N
	ORI ACCUM,0x08
	SBRC FLAGS,DRV_HD44780_FL_F
	ORI ACCUM,0x04
	MCALL _DRV_HD44780__SEND_CMDBYTE
	;DISPLAY OFF
	LDI ACCUM,0x08
	SBRC FLAGS,DRV_HD44780_FL_C
	ORI ACCUM,0x02
	SBRC FLAGS,DRV_HD44780_FL_B
	ORI ACCUM,0x01
	MCALL _DRV_HD44780__SEND_CMDBYTE
	;DISPLAY CLEAR
	LDI ACCUM,0x01
	MCALL _DRV_HD44780__SEND_CMDBYTE
	;ENTRY MODE SET
	LDI ACCUM,0x04
	MCALL _DRV_HD44780__SEND_CMDBYTE
	;DISPLAY ON
	LDI ACCUM,0x0C
	SBRC FLAGS,DRV_HD44780_FL_C
	ORI ACCUM,0x02
	SBRC FLAGS,DRV_HD44780_FL_B
	ORI ACCUM,0x01
	MCALL _DRV_HD44780__SEND_CMDBYTE

	LDI TEMP,DRV_RESULT_OK
	POP ACCUM
	POP FLAGS
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_HD44780_PROC_GET_MODES:
;--------------------------------------------------------
;Возвращаем поддерживаемые режимы
;OUT: TEMP_H-DRV_VIDEO_MODE_GRX...,
;TEMP_L-DRV_VIDEO_MODE_TXT..., TEMP-DRV_RESULT_...
;--------------------------------------------------------
_DRV_HD44780_PROC_GET_MODE:
;--------------------------------------------------------
;Возвращаем текущий режим
;OUT: TEMP_H-DRV_VIDEO_MODE_GRX...,
;TEMP_L-DRV_VIDEO_MODE_TXT..., TEMP-DRV_RESULT_...
;--------------------------------------------------------
	LDI TEMP_H,0x00
	LDI TEMP_L,DRV_VIDEO_MODE_TXT1B
	LDI TEMP,DRV_RESULT_OK
	RET

;--------------------------------------------------------
;_DRV_HD44780_PROC_GET_SIZE:
;--------------------------------------------------------
;Возвращаем размер рабочей области
;OUT: TEMP_H-X(width), TEMP_L-Y(height),
;TEMP-DRV_RESULT_...
;--------------------------------------------------------
	;TODO
	LDI TEMP,DRV_RESULT_OK
	RET

;--------------------------------------------------------
_DRV_HD44780_PROC_SET_XY:
;--------------------------------------------------------
;Задаем текущую позицию
;IN: TEMP_H-X(width),TEMP_L-Y(height)
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH FLAGS
	PUSH ACCUM

	LDD FLAGS,Y+_DRV_HD44780_FLAGS
	CPI TEMP_L,0x00
	SBRS FLAGS,DRV_HD44780_FL_N
	BRNE _DRV_HD44780_PROC_SET_XY__ERROR
	CPI TEMP_L,0x02
	BRCC _DRV_HD44780_PROC_SET_XY__ERROR
	CPI TEMP_H,40														;Допустимо, но позиция может быть вне экрана
	BRCC _DRV_HD44780_PROC_SET_XY__ERROR

	LDI ACCUM,0x80
	CPI TEMP_L,0x01
	BRNE PC+0x02
	SUBI ACCUM,(0x100-40)
	ADD ACCUM,TEMP_H
	MCALL _DRV_HD44780__SEND_CMDBYTE

	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_HD44780_PROC_SET_XY__END
_DRV_HD44780_PROC_SET_XY__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_HD44780_PROC_SET_XY__END:
	POP ACCUM
	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_HD44780_PROC_SET_SYMBOL:
;--------------------------------------------------------
;Выводим символ на экран
;IN: ACCUM-символ
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH FLAGS

	LDD FLAGS,Y+_DRV_HD44780_FLAGS
	MCALL _DRV_HD44780__SEND_DATABYTE

	LDI TEMP,DRV_RESULT_OK

	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_HD44780_PROC_SET_TEXT:
;--------------------------------------------------------
;Задаем текущую позицию
;IN: Z-SRC адрес, LOOP_CNTR-длина строки
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH FLAGS
	PUSH LOOP_CNTR

	LDD FLAGS,Y+_DRV_HD44780_FLAGS

	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80

	CPI LOOP_CNTR,0x00
	BREQ _DRV_HD44780_PROC_SET_TEXT__ERROR

	MOV TEMP,ZH
_DRV_HD44780_PROC_SET_TEXT__LOOP:
	ANDI ZH,0x7f
	SBRS TEMP,0x07
	LD ACCUM,Z+
	SBRC TEMP,0x07
	LPM ACCUM,Z+
	MCALL _DRV_HD44780__SEND_DATABYTE
	DEC LOOP_CNTR
	BRNE _DRV_HD44780_PROC_SET_TEXT__LOOP

	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_HD44780_PROC_SET_TEXT__END
_DRV_HD44780_PROC_SET_TEXT__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_HD44780_PROC_SET_TEXT__END:
	POP LOOP_CNTR
	POP FLAGS
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_DRV_HD44780__SEND_CMDNIBBLE:
;--------------------------------------------------------
;Передаем старший ниббл команды
;IN: ACCUM-старший ниббл команды(биты 3,2,1,0),
;FLAGS-параметры(DRV_HD44780_FL_x)
;--------------------------------------------------------
	RCALL _DRV_HD44780__RS_LO
	SBRS FLAGS,DRV_HD44780_FL_DL
	RJMP _DRV_HD44780__SEND_NIBBLE
	RJMP _DRV_HD44780__SEND_CMDBYTE_8BIT

;--------------------------------------------------------
_DRV_HD44780__SEND_CMDBYTE:
;--------------------------------------------------------
;Передаем байт команды
;IN: ACCUM-байт команды,FLAGS-параметры(DRV_HD44780_FL_x)
;--------------------------------------------------------
	PUSH ACCUM

	RCALL _DRV_HD44780__RS_LO
	SBRS FLAGS,DRV_HD44780_FL_DL
	RJMP _DRV_HD44780__SEND_CMDBYTE_4BIT
_DRV_HD44780__SEND_CMDBYTE_8BIT:
	RCALL _DRV_HD44780__E_HI
	RCALL _DRV_HD44780__WAIT_20US
	PUSH TEMP
	MOV TEMP,ACCUM
	LDD ACCUM,Y+_DRV_HD44780_PORT
	MCALL PORT_SET_BYTE
	POP TEMP
	RCALL _DRV_HD44780__E_LO
	RCALL _DRV_HD44780__WAIT_20US
	RJMP _DRV_HD44780__SEND_CMDBYTE_FINISH
_DRV_HD44780__SEND_CMDBYTE_4BIT:

	RCALL _DRV_HD44780__SEND_NIBBLE
	SWAP ACCUM
	RCALL _DRV_HD44780__SEND_NIBBLE

_DRV_HD44780__SEND_CMDBYTE_FINISH:
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_HD44780__SEND_DATABYTE:
;--------------------------------------------------------
;Передаем байт данных(символ)
;IN: ACCUM-байт данных,FLAGS-параметры(DRV_HD44780_FL_x)
;--------------------------------------------------------
;TODOоптимизровать вместе с _DRV_HD44780__SEND_CMDBYTE
	PUSH ACCUM

	RCALL _DRV_HD44780__RS_HI
	SBRS FLAGS,DRV_HD44780_FL_DL
	RJMP _DRV_HD44780__SEND_DATABYTE_4BIT
_DRV_HD44780__SEND_DATABYTE_8BIT:
	RCALL _DRV_HD44780__E_HI
	RCALL _DRV_HD44780__WAIT_20US
	PUSH TEMP
	MOV TEMP,ACCUM
	LDD ACCUM,Y+_DRV_HD44780_PORT
	MCALL PORT_SET_BYTE
	POP TEMP
	RCALL _DRV_HD44780__E_LO
	RCALL _DRV_HD44780__WAIT_20US
	RJMP _DRV_HD44780__SEND_DATABYTE_FINISH
_DRV_HD44780__SEND_DATABYTE_4BIT:

	RCALL _DRV_HD44780__SEND_NIBBLE
	SWAP ACCUM
	RCALL _DRV_HD44780__SEND_NIBBLE

_DRV_HD44780__SEND_DATABYTE_FINISH:
	POP ACCUM
	RET


;--------------------------------------------------------
_DRV_HD44780__SEND_NIBBLE:
;--------------------------------------------------------
;Передаем старший ниббл команды
;IN: ACCUM-старший ниббл команды(биты 7,6,5,4),
;FLAGS-параметры(DRV_HD44780_FL_x)
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	ANDI ACCUM,0xf0

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP _DRV_HD44780__SEND_NIBBLE_I2C
_DRV_HD44780__SEND_NIBBLE_4BIT:
	SWAP ACCUM
	PUSH_Z
	PUSH ACCUM
	LDD ACCUM,Y+_DRV_HD44780_PORT
	MCALL PORT_OFFSETS
	POP ACCUM
	LDS TEMP_EL,SREG
	CLI
	MOV ZH,C0x00
	MOV ZL,TEMP_H
	LD TEMP,Z
	ANDI TEMP,0xf0
	OR TEMP,ACCUM
	ST Z,TEMP
	STS SREG,TEMP_EL
	POP_Z
	RCALL _DRV_HD44780__E_HI
	RCALL _DRV_HD44780__WAIT_20US
	RCALL _DRV_HD44780__E_LO
	RCALL _DRV_HD44780__WAIT_20US
	RJMP _DRV_HD44780__SEND_NIBBLE_END
_DRV_HD44780__SEND_NIBBLE_I2C:
	LDD TEMP,Y+_DRV_HD44780_I2C_BUFFER
	ANDI TEMP,0x0f
	OR TEMP,ACCUM
	STD Y+_DRV_HD44780_I2C_BUFFER,TEMP
	RCALL _DRV_HD44780__E_HI
	RCALL _DRV_HD44780__I2C_EXEC
	RCALL _DRV_HD44780__E_LO
	RCALL _DRV_HD44780__I2C_EXEC
_DRV_HD44780__SEND_NIBBLE_END:

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_DRV_HD44780__I2C_EXEC:
;--------------------------------------------------------
	LDD TEMP,Y+_DRV_HD44780_I2C_BUFFER
	ORI TEMP,0x08
	STD Y+_DRV_HD44780_I2C_BUFFER,TEMP

	LDD TEMP,Y+_DRV_HD44780_PORT
	MOVW ZL,YL
	LDI ACCUM,_DRV_HD44780_PCF8574_I2C_ADDR
	LDI TEMP_H,0x01
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	RCALL _DRV_HD44780__WAIT_20US
	RET

;--------------------------------------------------------
_DRV_HD44780__RS_HI:
;--------------------------------------------------------
	PUSH ACCUM

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP PC+0x02+_MCALL_SIZE+0x01

	LDD ACCUM,Y+_DRV_HD44780_RS_PORT
	MCALL PORT_SET_HI
	RJMP PC+0x04

	LDD ACCUM,Y+_DRV_HD44780_I2C_BUFFER
	ORI ACCUM,(1<<DRV_HD44780_I2C_RS)
	STD Y+_DRV_HD44780_I2C_BUFFER,ACCUM

	POP ACCUM
	RET
;--------------------------------------------------------
_DRV_HD44780__RS_LO:
;--------------------------------------------------------
	PUSH ACCUM

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP PC+0x02+_MCALL_SIZE+0x01

	LDD ACCUM,Y+_DRV_HD44780_RS_PORT
	MCALL PORT_SET_LO
	RJMP PC+0x04

	LDD ACCUM,Y+_DRV_HD44780_I2C_BUFFER
	ANDI ACCUM,~(1<<DRV_HD44780_I2C_RS)
	STD Y+_DRV_HD44780_I2C_BUFFER,ACCUM

	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_HD44780__E_HI:
;--------------------------------------------------------
	PUSH ACCUM

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP PC+0x02+_MCALL_SIZE+0x01

	LDD ACCUM,Y+_DRV_HD44780_E_PORT
	MCALL PORT_SET_HI
	RJMP PC+0x04

	LDD ACCUM,Y+_DRV_HD44780_I2C_BUFFER
	ORI ACCUM,(1<<DRV_HD44780_I2C_E)
	STD Y+_DRV_HD44780_I2C_BUFFER,ACCUM

	POP ACCUM
	RET
;--------------------------------------------------------
_DRV_HD44780__E_LO:
;--------------------------------------------------------
	PUSH ACCUM

	SBRC FLAGS,DRV_HD44780_FL_I2C
	RJMP PC+0x02+_MCALL_SIZE+0x01

	LDD ACCUM,Y+_DRV_HD44780_E_PORT
	MCALL PORT_SET_LO
	RJMP PC+0x04

	LDD ACCUM,Y+_DRV_HD44780_I2C_BUFFER
	ANDI ACCUM,~(1<<DRV_HD44780_I2C_E)
	STD Y+_DRV_HD44780_I2C_BUFFER,ACCUM

	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_HD44780__WAIT_20US:
;--------------------------------------------------------
;Выдерживаем паузу, для готовности устройства
;--------------------------------------------------------
;TODO Работает только с большой паузой, в 250us вместо максимум 40us. Почему?
	PUSH TEMP
	LDI TEMP,250;80
	MCALL _C5_WAIT_500NS
	LDI TEMP,250
 	MCALL _C5_WAIT_500NS

	POP TEMP
	RET
.endif
