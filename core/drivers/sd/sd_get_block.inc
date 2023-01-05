;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_BLOCK
.else
.set DEF_DRIVER_SD_GET_BLOCK = 1

.include	"./core/time32_mark.inc"
.include	"./core/time32_delta.inc"
.include	"./conv/crc16_xmodem.inc"
.include	"./conv/crc16_block.inc"
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/io/out_word.inc"
		.include	"./core/io/out_char.inc"
		.include	"./core/io/out_ramdump.inc"
		.include	"./core/io/out_str.inc"
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_BLOCK:
;--------------------------------------------------------
;Прием блока данных
;IN: X-адрес на выделенную память,Y-адрес на переменные
;TEMP_H,L-максимальная длина данных
;OUT: TEMP_H,L-длина полученных данных
;ACCUM-0x00 успешное выполнение
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_Y
	PUSH_Z
	PUSH_X

	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	LDI TRY_CNTR,0x00
_DRV_SD_GET_BLOCK__BEGIN_LOOP:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xfe
	BREQ _DRV_SD_GET_BLOCK__DATA
_DRV_SD_GET_BLOCK__NO_ERROR:
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_BLOCK__BEGIN_LOOP
	RJMP _DRV_SD_GET_BLOCK__FAIL

_DRV_SD_GET_BLOCK__DATA:
	MCALL _DRV_SD_BYTE_RECV
	ST X+,TEMP
	SBIW ZL,0x01
	BRNE _DRV_SD_GET_BLOCK__DATA
	;Получаем CRC
	MCALL _DRV_SD_BYTE_RECV
	MOV ACCUM,TEMP
	MCALL _DRV_SD_BYTE_RECV
	POP_X

	PUSH_X
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI_Z CRC16_XMODEM
	MCALL CRC16_BLOCK
	CP ACCUM,TEMP_EH
	BRNE _DRV_SD_GET_BLOCK__FAIL
	CP TEMP,TEMP_EL
	BRNE _DRV_SD_GET_BLOCK__FAIL

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Z
		LDI_Z _DRV_SD_LOG_STR_BLOCK|0x8000
		MCALL C5_OUT_STR
		LDI TEMP,'['
		MCALL C5_OUT_CHAR
		MCALL C5_OUT_WORD
		LDI TEMP,']'
		MCALL C5_OUT_CHAR
		MOVW ZL,XL
		MCALL C5_OUT_RAMDUMP
		POP_Z
	.endif
.endif
	LDI ACCUM,0x00
	RJMP _DRV_SD_GET_BLOCK__END
_DRV_SD_GET_BLOCK__FAIL:
	LDI ACCUM,0xff
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
_DRV_SD_GET_BLOCK__END:

	POP_X
	POP_Z
	POP_Y
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP
	POP TRY_CNTR
	RET
.endif
