;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.06.2021	w5277c@gmail.com			Начало (не тестировано)
;04.07.2021	w5277c@gmail.com			Z->Y, необходимо тестирование
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер аппаратного SPI
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_SPI_H
.else
.set DEF__C5_DRV_SPI_H = 1
.message "Included driver hardware SPI master v0.2"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./core/wait.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_SPI_H_SRC_ADDR						= 0x00		;2B - адрес блока данных для передачи
	.EQU	_DRV_SPI_H_DST_ADDR						= 0x02		;2B - адрес блока данных для приема
	;---
	.EQU	_DRV_SPI_H_POS								= 0x04		;1B - Позиция
	.EQU	_DRV_SPI_H_TX_LEN							= 0x05		;1B - Длина данных для передачи
	.EQU	_DRV_SPI_H_RX_LEN							= 0x06		;1B - Длина данных для приема
	;---

	;---MODE-(LEADING-EDGE)---
	.EQU	DRV_SPI_H_MODE_MASTER					= (1<<MSTR)
	.EQU	DRV_SPI_H_MODE_SLAVE						= (0<<MSTR)
	.EQU	DRV_SPI_H_MODE_SM_RI						= (0<<CPOL)|(0<<CPHA)
	.EQU	DRV_SPI_H_MODE_ST_RI						= (0<<CPOL)|(1<<CPHA)
	.EQU	DRV_SPI_H_MODE_SM_FL						= (1<<CPOL)|(0<<CPHA)
	.EQU	DRV_SPI_H_MODE_ST_FL						= (1<<CPOL)|(1<<CPHA)
	;---PRESCALLER---
	.EQU	DRV_SPI_H_PRESC_4							= (0<<SPR1)|(0<<SPR0)
	.EQU	DRV_SPI_H_PRESC_16						= (0<<SPR1)|(1<<SPR0)
	.EQU	DRV_SPI_H_PRESC_64						= (1<<SPR1)|(0<<SPR0)
	.EQU	DRV_SPI_H_PRESC_128						= (1<<SPR1)|(1<<SPR0)
	;---DATA-ORDER---
	.EQU	DRV_SPI_H_DO_LSB_FIRST					= (1<<DORD)
	.EQU	DRV_SPI_H_DO_MSB_FIRST					= (0<<DORD)

	.EQU	_DRV_SPI_H_RAM_SIZE						= 0x07		;7 байт необходимо выделить

;--------------------------------------------------------
DRV_SPI_H_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP-DRV_SPI_H_MODE_MASTER/SLAVE|
;TEMP-DRV_SPI_H_MODE_x|DRV_SPI_H_PRESC_x|DRV_SPI_H_DO_x
;--------------------------------------------------------
	LDI ACCUM,_DRV_SPI_H_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Задаем режим работы
	ORI TEMP,(1<<SPIE)
	STS SPCR,TEMP

	;Устанавливаю вектор прерывания
	LDI ACCUM,C5_IR_SPI
	LDI TEMP_H,high(_DRV_SPI_H_EVENT)
	LDI TEMP_L,low(_DRV_SPI_H_EVENT)
	MCALL C5_IR_VECTOR_SET

	;Настраиваем порты
	SBRS TEMP,MSTR
	RJMP DRV_SPI_H_INIT__SLAVE
DRV_SPI_H_INIT__MASTER:
	LDI ACCUM,MISO
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	LDI ACCUM,MOSI
	MCALL PORT_MODE_OUT
	LDI ACCUM,SCK
	MCALL PORT_MODE_OUT
	RJMP DRV_SPI_H_INIT__END
DRV_SPI_H_INIT__SLAVE:
	LDI ACCUM,MISO
	MCALL PORT_MODE_OUT
	LDI ACCUM,MOSI
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	LDI ACCUM,SCK
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI
	LDI ACCUM,SS
	MCALL PORT_MODE_IN
	MCALL PORT_SET_HI

DRV_SPI_H_INIT__END:
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,TEMP_H-TX длина,
;TEMP_L-RX длина
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	;Запоминаем SRC
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_SPI_H_SRC_ADDR+0x00,ZH
	STD Y+_DRV_SPI_H_SRC_ADDR+0x01,ZL
	;Запоминаем DST
	STD Y+_DRV_SPI_H_DST_ADDR+0x00,XH
	STD Y+_DRV_SPI_H_DST_ADDR+0x01,XL
	;Запминаем длину TX и RX
	STD Y+_DRV_SPI_H_TX_LEN,TEMP_H
	STD Y+_DRV_SPI_H_RX_LEN,TEMP_L
	;Сброс позиции
	STD Z+_DRV_SPI_H_POS,C0x00
	;Включаем SPI
	_C5_POWER_ON PRSPI
	CLI
	LDS TEMP,SPCR
	ORI TEMP,(1<<SPE)
	STS SPCR,TEMP
	SEI

	MCALL _DRV_SPI_H_EVENT

	;Жду окончания передачи
_DRV_SPI_H_PROC__WAIT:
	LDD TEMP,Y+_DRV_SPI_H_POS
	CP TEMP,TEMP_H
	BRCS PC+0x04
	BRNE PC+0x06
	CP TEMP,TEMP_L
	BRCC _DRV_SPI_H_PROC__DONE
	MCALL C5_WAIT
	RJMP _DRV_SPI_H_PROC__WAIT
_DRV_SPI_H_PROC__DONE:
	;Выключаем SPI
	CLI
	LDS TEMP,SPCR
	ANDI TEMP,~(1<<SPE)
	STS SPCR,TEMP
	SEI
	_C5_POWER_OFF PRSPI

	POP TEMP
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_SPI_H_EVENT:
;--------------------------------------------------------
;Обработчик прерывания
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	LDS TEMP,SREG
	PUSH TEMP
	CLI

	MCALL C5_RAM_OFFSET

	;Считываю позицию в буфере
	LDD TEMP,Y+_DRV_SPI_H_POS
	LDD TEMP_H,Y+_DRV_SPI_H_TX_LEN
	LDD TEMP_L,Y+_DRV_SPI_H_RX_LEN

	CP TEMP,TEMP_H
	BRCC _DRV_SPI_H_NEXT_BYTE__TX_SKIP
	PUSH_Z
	LDD ZH,Y+_DRV_SPI_H_SRC_ADDR+0x00
  	LDD ZL,Y+_DRV_SPI_H_SRC_ADDR+0x01
	MOV ACCUM,ZH
	ANDI ZH,0x7f
	ADD ZL,TEMP
	ADC ZH,C0x00
	SBRS ACCUM,0x07
	LD ZL,Z
	SBRC ACCUM,0x07
	LPM ZL,Z
	STS SPDR,ZL
	POP_Z
_DRV_SPI_H_NEXT_BYTE__TX_SKIP:
	CP TEMP,TEMP_L
	BRCC _DRV_SPI_H_NEXT_BYTE__RX_SKIP
	LDS TEMP_L,SPDR
	PUSH_Z
	LDD ZH,Y+_DRV_SPI_H_DST_ADDR+0x00
  	LDD ZL,Y+_DRV_SPI_H_DST_ADDR+0x01
	MOV ACCUM,ZH
	ADD ZL,TEMP
	ADC ZH,C0x00
	ST Z,TEMP_L
	POP_Z
_DRV_SPI_H_NEXT_BYTE__RX_SKIP:

	;Инкрементируем позицию
	INC TEMP
	STD Y+_DRV_SPI_H_POS,TEMP

	POP TEMP
	STS SREG,TEMP

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
