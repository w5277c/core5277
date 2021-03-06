;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.06.2021  w5277c@gmail.com			Начало (не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер аппаратного SPI (режим мастера)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_SPI_MH
.else
.set DEF__C5_DRV_SPI_MH = 1
.message "Included driver hardware SPI master v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./core/wait.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_SPI_MH_SRC_ADDR						= 0x00		;2B - адрес блока данных для передачи
	.EQU	_DRV_SPI_MH_DST_ADDR						= 0x02		;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_SPI_MH_POS							= 0x04		;1B - Позиция
	.EQU	_DRV_SPI_MH_TX_LEN						= 0x05		;1B - Длина данных для передачи
	.EQU	_DRV_SPI_MH_RX_LEN						= 0x06		;1B - Длина данных для приема
	;---

	;---MODE-(LEADING-EDGE)---
	.EQU	DRV_SPI_MH_MODE_SM_RI					= (0<<CPOL)|(0<<CPHA)
	.EQU	DRV_SPI_MH_MODE_ST_RI					= (0<<CPOL)|(1<<CPHA)
	.EQU	DRV_SPI_MH_MODE_SM_FL					= (1<<CPOL)|(0<<CPHA)
	.EQU	DRV_SPI_MH_MODE_ST_FL					= (1<<CPOL)|(1<<CPHA)
	;---PRESCALLER---
	.EQU	DRV_SPI_MH_PRESC_4						= (0<<SPR1)|(0<<SPR0)
	.EQU	DRV_SPI_MH_PRESC_16						= (0<<SPR1)|(1<<SPR0)
	.EQU	DRV_SPI_MH_PRESC_64						= (1<<SPR1)|(0<<SPR0)
	.EQU	DRV_SPI_MH_PRESC_128						= (1<<SPR1)|(1<<SPR0)
	;---DATA-ORDER---
	.EQU	DRV_SPI_MH_DO_LSB_FIRST					= (1<<DORD)
	.EQU	DRV_SPI_MH_DO_MSB_FIRST					= (0<<DORD)

	.EQU	_DRV_SPI_MH_RAM_SIZE						= 0x07		;7 байт необходимо выделить

;--------------------------------------------------------
DRV_SPI_MH_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP-DRV_SPI_MH_MODE_x|DRV_SPI_MH_PRESC_x|
;DRV_SPI_MH_DO_x
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	LDI ACCUM,_DRV_SPI_MH_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Задаем режим работы
	ORI TEMP,(1<<MSTR)|(1<<SPIE)
	STS SPCR,TEMP

	;Настраиваем порты
	LDI ACCUM,MISO
	MCALL PORT_MODE_IN
	LDI ACCUM,MOSI
	MCALL PORT_MODE_OUT
	LDI ACCUM,SCK
	MCALL PORT_MODE_OUT

	;Устанавливаю вектор прерывания
	LDI ACCUM,C5_IR_SPI
	LDI TEMP_H,high(_DRV_SPI_MH_EVENT)
	LDI TEMP_L,low(_DRV_SPI_MH_EVENT)
	MCALL C5_IR_VECTOR_SET

	POP ACCUM
	POP TEMP
	POP_Z
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y-src адрес, X-dst адрес,
;TEMP_H-TX длина, TEMP_L-RX длина
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	;Запоминаем SRC
	SBRS YH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL YL
	ROL YH
	ORI YH,0x80
	STD Z+_DRV_SPI_MH_SRC_ADDR+0x00,YH
	STD Z+_DRV_SPI_MH_SRC_ADDR+0x01,YL
	;Запоминаем DST
	STD Z+_DRV_SPI_MH_DST_ADDR+0x00,XH
	STD Z+_DRV_SPI_MH_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Z+_DRV_SPI_MH_TX_LEN,TEMP_H
	STD Z+_DRV_SPI_MH_RX_LEN,TEMP_L
	;Сброс позиции
	CLR TEMP
	STD Z+_DRV_SPI_MH_POS,TEMP
	;Включаем SPI
	CLI
	LDS TEMP,SPCR
	ORI TEMP,(1<<SPE)
	STS SPCR,TEMP
	SEI
	;Жду окончания передачи
_DRV_SPI_MH_PROC__WAIT:
	LDD TEMP,Z+_DRV_SPI_MH_POS
	CP TEMP_H,TEMP
	BRCC PC+0x03
	CP TEMP_L,TEMP
	BRCS _DRV_SPI_MH_PROC__DONE
	MCALL C5_WAIT
	RJMP _DRV_SPI_MH_PROC__WAIT
_DRV_SPI_MH_PROC__DONE:
	;Выключаем SPI
	CLI
	LDS TEMP,SPCR
	ANDI TEMP,~(1<<SPE)
	STS SPCR,TEMP
	SEI

	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_SPI_MH_EVENT:
;--------------------------------------------------------
;Обработчик прерывания
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	;Считываю позицию в буфере
	LDD TEMP,Z+_DRV_SPI_MH_POS
	LDD TEMP_H,Z+_DRV_SPI_MH_TX_LEN
	LDD TEMP_L,Z+_DRV_SPI_MH_RX_LEN

	CP TEMP_H,TEMP
	BRCS _DRV_SPI_MH_EVENT__TX_SKIP
	PUSH_Z
	LDD ACCUM,Z+_DRV_SPI_MH_SRC_ADDR+0x00
  	LDD ZL,Z+_DRV_SPI_MH_SRC_ADDR+0x01
	MOV ZH,ACCUM
	ANDI ZH,0x7f
	ADD ZL,TEMP
	ADC ZH,C0x00
	SBRS ACCUM,0x07
	LD ZL,Z
	SBRC ACCUM,0x07
	LPM ZL,Z
	STS SPDR,ZL
	POP_Z
_DRV_SPI_MH_EVENT__TX_SKIP:
	CP TEMP_L,TEMP
	BRCS _DRV_SPI_MH_EVENT__RX_SKIP
	LDS TEMP_L,SPDR
	PUSH_Z
	LDD ACCUM,Z+_DRV_SPI_MH_DST_ADDR+0x00
  	LDD ZL,Z+_DRV_SPI_MH_DST_ADDR+0x01
	MOV ZH,ACCUM
	ADD ZL,TEMP
	ADC ZH,C0x00
	ST Z,TEMP_L
	POP_Z
_DRV_SPI_MH_EVENT__RX_SKIP:

	;Инкрементируем позицию
	CP TEMP_H,TEMP
	BRCC PC+0x03
	CP TEMP_L,TEMP
	BRCS PC+0x02
	INC TEMP

_DRV_SPI_MH_EVENT__END:
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif