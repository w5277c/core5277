;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.10.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_WH1602_PCF8574
.else
.set DEF__C5_DRV_WH1602_PCF8574 = 1
.message "Included driver WH1602_PCF8574 v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"../core/drivers/_video.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_WH1602_PCF8574_ADDR				= 0x27		;Адрес устройства на шине I2C
	;---
	.EQU	_DRV_WH1602_PCF8574_BUFFER				= 0x00		;4B буффер
	.EQU	_DRV_WH1602_PCF8574_DRV					= 0x04		;1B ид драйвера I2C
	.EQU	_DRV_WH1602_PCF8574_RAM_SIZE			= 0x05
;--------------------------------------------------------
DRV_WH1602_PCF8574_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-ид драйвера I2C
;--------------------------------------------------------
	LDI ACCUM,_DRV_WH1602_PCF8574_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_WH1602_PCF8574_DRV,TEMP_H

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
	MCALL _DRV_WH1602_PCF8574_PROC_INIT
	RJMP _DRV_WH1602_PCF8574__END

	CPI FLAGS,DRV_VIDEO_OP_GET_MODES
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_WH1602_PCF8574_PROC_GET_MODES
	RJMP _DRV_WH1602_PCF8574__END

	CPI FLAGS,DRV_VIDEO_OP_GET_MODE
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_WH1602_PCF8574_PROC_GET_MODE
	RJMP _DRV_WH1602_PCF8574__END

	CPI FLAGS,DRV_VIDEO_OP_GET_SIZE
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUS5277_PROC_MASTER
	RJMP _DRV_WH1602_PCF8574__END

	CPI FLAGS,DRV_VIDEO_OP_SET_XY
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUS5277_PROC_MASTER
	RJMP _DRV_WH1602_PCF8574__END

	CPI FLAGS,DRV_VIDEO_OP_SET_TEXT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUS5277_PROC_MASTER
	RJMP _DRV_WH1602_PCF8574__END

	LDI TEMP,DRV_RESULT_UNSUPPORTED

_DRV_WH1602_PCF8574__END:
	POP_Y
	RET

;--------------------------------------------------------
_DRV_WH1602_PCF8574_PROC_INIT:
;--------------------------------------------------------
;Подготавливем экран к работе
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,8															;>15ms
	MCALL C5_WAIT_2MS
	LDI TEMP,0x30
	MCALL _DRV_WH1602_PCF8574__SEND_CMDNIBBLE
	LDI TEMP,3															;>4.1ms
	MCALL C5_WAIT_2MS
	MCALL _DRV_WH1602_PCF8574__SEND_CMDNIBBLE
	LDI TEMP,1															;>100us
	MCALL C5_WAIT_2MS
	MCALL _DRV_WH1602_PCF8574__SEND_CMDNIBBLE
	;4 bit bus mode
	LDI TEMP,0x20
	MCALL _DRV_WH1602_PCF8574__SEND_CMDNIBBLE
	;2 строки, 5x8 font
	LDI TEMP,0x28
	MCALL _DRV_WH1602_PCF8574__SEND_CMDBYTE
	;DISPLAY OFF
	LDI TEMP,0x08
	MCALL _DRV_WH1602_PCF8574__SEND_CMDBYTE
	;CURSOR SHIFT RIGHT
	LDI TEMP,0x14
	MCALL _DRV_WH1602_PCF8574__SEND_CMDBYTE
	;DISPLAY CLEAR
	LDI TEMP,0x01
	MCALL _DRV_WH1602_PCF8574__SEND_CMDBYTE
	;DISPLAY ON
	LDI TEMP,0x0C
	MCALL _DRV_WH1602_PCF8574__SEND_CMDBYTE

	LDI TEMP,DRV_RESULT_OK
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_WH1602_PCF8574_PROC_GET_MODES:
;--------------------------------------------------------
;Возвращаем поддерживаемые режимы
;OUT: TEMP_H-DRV_VIDEO_MODE_GRX...,
;TEMP_L-DRV_VIDEO_MODE_TXT..., TEMP-DRV_RESULT_...
;--------------------------------------------------------
_DRV_WH1602_PCF8574_PROC_GET_MODE:
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
_DRV_WH1602_PCF8574_PROC_GET_SIZE:
;--------------------------------------------------------
;Возвращаем размер рабочей области
;OUT: TEMP_H-X(width), TEMP_L-Y(height),
;TEMP-DRV_RESULT_...
;--------------------------------------------------------
	LDI TEMP_H,0x10
	LDI TEMP_L,0x02
	LDI TEMP,DRV_RESULT_OK
	RET

;--------------------------------------------------------
_DRV_WH1602_PCF8574_PROC_SET_XY:
;--------------------------------------------------------
;Задаем текущую позицию
;IN: TEMP_H-X(width),TEMP_L-Y(height)
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	RET


_DRV_WH1602_PCF8574__SEND_CMDNIBBLE:
	MCALL _DRV_WH1602_PCF8574__WAIT_20US
	RET
_DRV_WH1602_PCF8574__SEND_CMDBYTE:
	MCALL _DRV_WH1602_PCF8574__WAIT_20US
	RET
_DRV_WH1602_PCF8574__SEND_DATABYTE:
	MCALL _DRV_WH1602_PCF8574__WAIT_20US
	RET

;--------------------------------------------------------
_DRV_WH1602_PCF8574__WAIT_20US:
;--------------------------------------------------------
;Выдерживаем паузу, ожидая готовность устройства
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,40
	MCALL _C5_WAIT_500NS
	POP TEMP
	RET

