;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SD_LOG_CID
.else
.set DEF_C5_SD_LOG_CID = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_byte.inc"
.include	"./core/log/log_cr.inc"
.include	"./core/log/log_char.inc"
.include	"./core/log/log_str_el.inc"
.include	"./core/log/log_str.inc"
.include	"./core/log/log_strn.inc"
.include	"./core/log/log_bytes.inc"
.include	"./core/log/log_numx8.inc"
.include	"./core/log/log_numx16.inc"

_DRV_SD_LOG_STR_CID:
	.db "CID register:",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_CID_FIELDS:
	.db " MID:0x",0x00," OID:",0x00," PNM:",0x00," PRV:",0x00," PSN:",0x00," MDT:",0x00

;--------------------------------------------------------
DRV_SD_LOG_CID:
;Логирование CID
;IN: Z-адрес на блок данных
;--------------------------------------------------------
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	LDI_Y _DRV_SD_LOG_STR_CID|0x8000
	MCALL C5_LOG_STR
	LDI_Y _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x00
	MCALL C5_LOG_STR_EL
	LDD TEMP,Z+0x00
	MCALL C5_LOG_BYTE
	MCALL C5_LOG_CR

	LDI TEMP,0x01
	MCALL C5_LOG_STR_EL
	LDD TEMP,Z+0x01
	MCALL C5_LOG_CHAR
	LDD TEMP,Z+0x02
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CR

	LDI TEMP,0x02
	MCALL C5_LOG_STR_EL
	LDI YL,0x03
	LDI YH,0x00
	ADD YL,ZL
	ADC YH,ZH
	LDI TEMP,0x05
	MCALL C5_LOG_STRN
	MCALL C5_LOG_CR

	LDI_Y _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x03
	MCALL C5_LOG_STR_EL
	LDD TEMP,Z+0x08
	MCALL C5_LOG_BYTE
	MCALL C5_LOG_CR

	LDI TEMP,0x04
	MCALL C5_LOG_STR_EL
	LDI YL,0x09
	LDI YH,0x00
	ADD YL,ZL
	ADC YH,ZH
	LDI TEMP,0x04
	MCALL C5_LOG_BYTES
	MCALL C5_LOG_CR

	LDI_Y _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x04
	MCALL C5_LOG_STR_EL
	LDD TEMP_H,Z+0x0d
	LDD TEMP_L,Z+0x0e
	SWAP TEMP_H
	ANDI TEMP_H,0xf0
	MOV TEMP,TEMP_L
	SWAP TEMP
	ANDI TEMP,0x0f
	OR TEMP_H,TEMP
	ANDI TEMP_L,0x0f
	MCALL C5_LOG_NUMx8
	LDI TEMP,'.'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_H
	LDI TEMP_H,high(2000)
	LDI TEMP_L,low(2000)
	ADD TEMP_L,TEMP
	CLR TEMP
	ADC TEMP_H,TEMP
	MCALL C5_LOG_NUMX16
	MCALL C5_LOG_CR

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Y
	POP_Z
	RET
.endif
.endif
