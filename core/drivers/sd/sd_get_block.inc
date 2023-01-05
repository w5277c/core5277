;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_BLOCK
.else
.set DEF_DRIVER_SD_GET_BLOCK = 1

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
;IN: X-адрес на буффер, TEMP_H,L-длина данных
;OUT: TEMP=DRV_RESULT_...
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
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
	RJMP _DRV_SD_GET_BLOCK__ABSENT

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
	BRNE _DRV_SD_GET_BLOCK__INVALID_DATA
	CP TEMP,TEMP_EL
	BRNE _DRV_SD_GET_BLOCK__INVALID_DATA

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
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_GET_BLOCK__END
_DRV_SD_GET_BLOCK__INVALID_DATA:
	LDI TEMP,DRV_RESULT_INVALID_DATA
	RJMP _DRV_SD_GET_BLOCK__END
_DRV_SD_GET_BLOCK__ABSENT:
	LDI TEMP,DRV_RESULT_ABSENT
_DRV_SD_GET_BLOCK__END:

	POP_X
	POP_Z
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	POP TRY_CNTR
	RET
.endif
