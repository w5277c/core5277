;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_SET_BLOCK
.else
.set DEF_DRIVER_SD_SET_BLOCK = 1

.include	"./conv/crc16_xmodem.inc"
.include	"./conv/crc16_block.inc"
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/io/out_str.inc"
		.include	"./core/io/out_char.inc"
		.include	"./core/io/out_word.inc"
		.include	"./core/io/out_ramdump.inc"
		.include	"./core/io/out_cr.inc"
_DRV_SD_LOG_STR_CRC:
		.db "CRC:",0x00
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_SET_BLOCK:
;--------------------------------------------------------
;Передача блока данных
;IN: X-адрес на буффер
;TEMP_H,L-длина данных
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_X

	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI_Z CRC16_XMODEM
	MCALL CRC16_BLOCK

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
		LDI_Z _DRV_SD_LOG_STR_CRC|0x8000
		MCALL C5_OUT_STR
		PUSH TEMP_H
		PUSH TEMP_L
		MOV TEMP_H,TEMP_EH
		MOV TEMP_L,TEMP_EL
		MCALL C5_OUT_WORD
		POP TEMP_L
		POP TEMP_H
		MCALL C5_OUT_CR
		POP_Z
	.endif
.endif

	LDI TEMP,0xfe
	MCALL _DRV_SD_BYTE_SEND

_DRV_SD_SET_BLOCK__DATA:
	LD TEMP,X+
	MCALL _DRV_SD_BYTE_SEND
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	CPI TEMP_H,0x00
	BRNE _DRV_SD_SET_BLOCK__DATA
	CPI TEMP_L,0x00
	BRNE _DRV_SD_SET_BLOCK__DATA
	;Пишем CRC
	MOV TEMP,TEMP_EH
	MCALL _DRV_SD_BYTE_SEND
	MOV TEMP,TEMP_EL
	MCALL _DRV_SD_BYTE_SEND

	POP_X
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP ACCUM
	RET
.endif
