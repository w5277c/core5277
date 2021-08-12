;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.06.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер контроллера pcd8544(48x48 LCD controller), испольуется для Nokia5110
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_IR
.else
.set DEF__C5_DRV_IR = 1
.message "Included driver IR v0.1"

.include "./core/drivers/_video.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.SET	_DRV_PCD8544_BUFFER						= 0x00	;240B - видео буфер (48*5)
	.EQU	_DRV_PCD8544_DC_PORT						= 0xf0	;1B - Адрес порта DC
	.EQU	_DRV_PCD8544_DC_PIN						= 0xf1	;1B - Число для пина DC
	.EQU	_DRV_PCD8544_DIN_PORT					= 0xf2	;1B - Адрес порта DIN
	.EQU	_DRV_PCD8544_DIN_PIN						= 0xf3	;1B - Число для пина DIN
	.EQU	_DRV_PCD8544_CLK_PORT					= 0xf4	;1B - Адрес порта CLK
	.EQU	_DRV_PCD8544_CLK_PIN						= 0xf5	;1B - Число для пина CLK
	.EQU	_DRV_PCD8544_Y_BITPOS					= 0xf6	;1B - Текущий бит в Y позиции
	;---
	.EQU	_DRV_PCD8544_BUFFER_SIZE				= 0xf0	;
	.EQU	_DRV_PCD8544_RAM_SIZE					= 0xf7	;

	.EQU	_DRV_PCD8544_CMD_SET_Y					= 0b00100000
	.EQU	_DRV_PCD8544_CMD_SET_X					= 0b01000000
	;---
	.EQU	_DRV_PCD8544_GRX_XSIZE					= 84
	.EQU	_DRV_PCD8544_GRX_YSIZE					= 48

;--------------------------------------------------------
DRV_PCD8544_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_EH-RST порт, TEMP_EL-DC порт, TEMP_H-Din порт,
;TEMP_L-Clk порт, ACCUM-ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_PCD8544_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Z+_DRV_PCD8544_SYNC_BIT_ID,FLAGS

	PUSH TEMP_L
	PUSH TEMP_H
	MOV ACCUM,TEMP_EL
	MCALL PORT_MODE_OUT
	MCALL PORT_OFFSETS
	STD Z+_DRV_PCD8544_DC_PORT,TEMP_H
	STD Z+_DRV_PCD8544_DC_PIN,ACCUM

	POP ACCUM
	MCALL PORT_MODE_OUT
	MCALL PORT_OFFSETS
	STD Z+_DRV_PCD8544_DIN_PORT,TEMP_H
	STD Z+_DRV_PCD8544_DIN_PIN,ACCUM

	POP ACCUM
	MCALL PORT_MODE_OUT
	MCALL PORT_OFFSETS
	STD Z+_DRV_PCD8544_CLK_PORT,TEMP_H
	STD Z+_DRV_PCD8544_CLK_PIN,ACCUM

	MOV ACCUM,TEMP_EH
	MCALL PORT_MODE_OUT
	MCALL PORT_SET_LO
	MCALL C5_WAIT
	MCALL PORT_SET_HI

	STD Z+_DRV_PCD8544_Y_BITPOS,C0x00

	POP TEMP
	LDI ACCUM,0x14|0x80												;25Hz
	LDI TEMP_H,high(_DRV_PCD8544_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_PCD8544_TIMER_EVENT)
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	MCALL C5_READY
;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;OUT: ACCUM-результат DRV_VIDEO_RESULT_x
;--------------------------------------------------------
	PUSH_Z

	LDI_Z _DRV_PCD8544_OP_END
	PUSH ZL
	PUSH ZH
	PUSH FLAGS
	LSL FLAGS
	LDI_Z _DRV_PCD8544_OP_JUMP_TABLE
	ADD ZL,FLAGS
	CLR FLAGS
	ADC ZH,FLAGS
	POP FLAGS
	PUSH ZL
	PUSH ZH
	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	RET
_DRV_PCD8544_OP_END:
	POP_Z
	RET

_DRV_PCD8544_OP_JUMP_TABLE:
	MJMP _DRV_PCD8544_PROC_GET_MODES
	MJMP _DRV_PCD8544_PROC_SET_MODE
	MJMP _DRV_PCD8544_PROC_GET_SIZE
	MJMP _DRV_PCD8544_PROC_SET_XY
	MJMP _DRV_PCD8544_PROC_UNSUPPORTED
	;....

_DRV_PCD8544_PROC_UNSUPPORTED:
	LDI ACCUM,DRV_VIDEO_RESULT_UNSUPPORTED
	RET
;--------------------------------------------------------
_DRV_PCD8544_PROC_GET_MODES:
;--------------------------------------------------------
;Список поддердживаемых режимов
;IN: Z-выделенная память,
;OUT: TEMP-TXT режимы, TEMP_H/L-графические режимы
;ACCUM-результат DRV_VIDEO_RESULT_x
;--------------------------------------------------------
	LDI TEMP,0x00
	LDI TEMP_H,high(1<<DRV_VIDEO_MODE_GRX1B)
	LDI TEMP_L,low(1<<DRV_VIDEO_MODE_GRX1B)
	LDI ACCUM,DRV_VIDEO_RESULT_OK
	RET

;--------------------------------------------------------
_DRV_PCD8544_PROC_SET_MODE:
;--------------------------------------------------------
;Усановка режима
;IN: Z-выделенная память, TEMP-режим
;(номер бита, включеный 7бит-графический режим)
;OUT: ACCUM-результат DRV_VIDEO_RESULT_x
;--------------------------------------------------------
	PUSH_X
	PUSH_Y

	CPI TEMP,DRV_VIDEO_MODE_GRX1B|0x80
	BRNE _DRV_PCD8544_PROC_SET_MODE__UNSUPPORTED

	LDI_X 0x0000
	LDI_Y 0x0000
	MCALL _DRV_PCD8544_PROC_SET_XY

	LDI ACCUM,DRV_VIDEO_RESULT_OK
	RJMP _DRV_PCD8544_PROC_SET_MODE__END
_DRV_PCD8544_PROC_SET_MODE__UNSUPPORTED:
	LDI ACCUM,DRV_VIDEO_RESULT_UNSUPPORTED
_DRV_PCD8544_PROC_SET_MODE__END:

	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_PCD8544_PROC_GET_SIZE:
;--------------------------------------------------------
;Список поддердживаемых режимов
;IN: Z-выделенная память,
;OUT: X/Y-размер экрана
;ACCUM-результат DRV_VIDEO_RESULT_x
;--------------------------------------------------------
	LDI_X 0x0054
	LDI_Y 0x0030
	LDI ACCUM,DRV_VIDEO_RESULT_OK
	RET

;--------------------------------------------------------
_DRV_PCD8544_PROC_SET_XY:
;--------------------------------------------------------
;Установка позиции
;IN: Z-выделенная память, X/Y-размер экрана
;OUT: ACCUM-результат DRV_VIDEO_RESULT_x
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM

	CPI XH,0x00
	BRNE _DRV_PCD8544_PROC_SET_XY__UNSUPPORTED
	CPI XL,0x54
	BRCC _DRV_PCD8544_PROC_SET_XY__UNSUPPORTED
	CPI YH,0x00
	BRNE _DRV_PCD8544_PROC_SET_XY__UNSUPPORTED
	CPI YL,0x30
	BRCC _DRV_PCD8544_PROC_SET_XY__UNSUPPORTED

	MOV TEMP,YL
	LSR YL
	LSR YL
	LSR YL
	MOV ACCUM,YL
	LSL ACCUM
	LSL ACCUM
	LSL ACCUM
	SUB TEMP,ACCUM
	STD Z+_DRV_PCD8544_Y_BITPOS,TEMP

	LDI TEMP,_DRV_PCD8544_CMD_SET_Y
	OR TEMP,YL
	MCALL _DRV_PCD8544_WRITE_DATA
	LDI TEMP,_DRV_PCD8544_CMD_SET_X
	OR TEMP,XL
	MCALL _DRV_PCD8544_WRITE_DATA

	LDI ACCUM,DRV_VIDEO_RESULT_OK
	RJMP _DRV_PCD8544_PROC_SET_XY_END

_DRV_PCD8544_PROC_SET_XY_UNSUPPORTED:
	LDI ACCUM,DRV_VIDEO_RESULT_UNSUPPORTED
_DRV_PCD8544_PROC_SET_XY_END:

	POP ACCUM
	POP TEMP
	POP_Y
	RET


;--------------------------------------------------------
_DRV_PCD8544__TIMER_EVENT:
;--------------------------------------------------------
;Обработчик прерывания по таймеру для передачи данных
;из видео буфера на экран
;--------------------------------------------------------
	PUSH_Z
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP

	;Получаю адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	;SET X=0,Y=0
	LDI ACCUM,_DRV_PCD8544_CMD_SET_X
	MCALL _DRV_PCD8544__SEND_CMD
	LDI ACCUM,_DRV_PCD8544_CMD_SET_Y
	MCALL _DRV_PCD8544__SEND_CMD
	;DATA MODE
	MCALL _DRV_PCD8544__SET_DATA_MODE
	LDI LOOP_CNTR,_DRV_PCD8544_BUFFER_SIZE
_DRV_PCD8544__TIMER_EVENT__LOOP:
	LD ACCUM,Z+
	MCALL _DRV_PCD8544__SEND_BYTE
	DEC LOOP_CNTR
	BRNE _DRV_PCD8544__TIMER_EVENT__LOOP

	;Video sync
	LDI TEMP,CFL_VIDEO_SYNC
	MCALL C5_FLAG_INVERT

	POP TEMP
	POP ACCUM
	POP LOOP_CNTR
	POP_Z
	RET

;--------------------------------------------------------
_DRV_PCD8544__SEND_CMD:
;--------------------------------------------------------
;Включаем на устройстве режим данных
;IN: Z-выделенная память,ACCUM-команда
;--------------------------------------------------------
	MCALL _DRV_PCD8544__SET_CMD_MODE
	RJMP _DRV_PCD8544__SEND_BYTE

;--------------------------------------------------------
_DRV_PCD8544__SET_DATA_MODE:
;--------------------------------------------------------
;Включаем на устройсве режим данных
;IN: Z-выделенная память
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	LDD ACCUM,Z+_DRV_PCD8544_DC_PIN
	LDI ZH,0x00
	LDD ZL,Z+_DRV_PCD8544_DC_PORT
	LD TEMP,Z
	ORI TEMP,ACCUM
	ST Z,TEMP

	POP ACCUM
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_PCD8544__SET_CMD_MODE:
;--------------------------------------------------------
;Включаем на устройсве режим данных
;IN: Z-выделенная память
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	LDD ACCUM,Z+_DRV_PCD8544_DC_PIN
	COM ACCUM
	LDI ZH,0x00
	LDD ZL,Z+_DRV_PCD8544_DC_PORT
	LD TEMP,Z
	ANDI TEMP,ACCUM
	ST Z,TEMP

	POP ACCUM
	POP TEMP
	POP_Z
	RET


;--------------------------------------------------------
_DRV_PCD8544__SEND_BYTE:
;--------------------------------------------------------
;Включаем на устройсве режим данных
;IN: Z-выделенная память, ACCUM-данные/команда
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_FT
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDS LOOP_CNTR,SREG
	PUSH LOOP_CNTR

	LDI YH,0x00
	LDD YL,Z+_DRV_PCD8544_DIN_PORT
	LDD TEMP_EH,Z+_DRV_PCD8544_DIN_PIN
	MOV TEMP_EL,TEMP_EH
	COM TEMP_EL

	LDI XH,0x00
	LDD XL,Z+_DRV_PCD8544_CLK_PORT
	LDD TEMP_H,Z+_DRV_PCD8544_CLK_PIN
	MOV TEMP_L,TEMP_H
	COM TEMP_L

	LDI LOOP_CNTR,0x08
_DRV_PCD8544__SEND_BYTE__LOOP:
	CLI
	;DIN=ACCUM 0x07 BIT
	LD TEMP,Y
	SBRC ACCUM,0x07
	OR TEMP,TEMP_EH
	SBRS ACCUM,0x07
	AND TEMP,TEMP_EL
	ST Y,TEMP
	;CLC LOW
	LD TEMP,X
	AND TEMP,TEMP_L
	ST X,TEMP
	;DELAY
	LSR ACCUM
	NOP
	NOP
	NOP
	NOP
	NOP
	;CLC HI
	LD TEMP,X
	OR TEMP,TEMP_H
	ST X,TEMP
	SEI
	DEC LOOP_CNTR
	BRNE _DRV_PCD8544__SEND_BYTE__LOOP

	POP LOOP_CNTR
	STS SREG,LOOP_CNTR
	POP LOOP_CNTR
	POP ACCUM
	POP_FT
	POP_Y
	POP_X
	RET

.endif
