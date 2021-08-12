;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SD_LOG_CID
.else
.set DEF_C5_SD_LOG_CID = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_byte.inc"
.include	"./core/io/out_cr.inc"
.include	"./core/io/out_char.inc"
.include	"./core/io/out_str_el.inc"
.include	"./core/io/out_str.inc"
.include	"./core/io/out_strn.inc"
.include	"./core/io/out_bytes.inc"
.include	"./core/io/out_num8.inc"
.include	"./core/io/out_num16.inc"

_DRV_SD_LOG_STR_CID:
	.db "CID register:",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_CID_FIELDS:
	.db " MID:0x",0x00," OID:",0x00," PNM:",0x00," PRV:",0x00," PSN:",0x00," MDT:",0x00

;--------------------------------------------------------
DRV_SD_LOG_CID:
;Логирование CID
;IN: X-адрес на блок данных
;--------------------------------------------------------
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	MOVW YL,XL

	LDI_Z _DRV_SD_LOG_STR_CID|0x8000
	MCALL C5_OUT_STR
	LDI_Z _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x00
	MCALL C5_OUT_STR_EL
	LDD TEMP,Y+0x00
	MCALL C5_OUT_BYTE
	MCALL C5_OUT_CR

	LDI TEMP,0x01
	MCALL C5_OUT_STR_EL
	LDD TEMP,Y+0x01
	MCALL C5_OUT_CHAR
	LDD TEMP,Y+0x02
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_CR

	LDI TEMP,0x02
	MCALL C5_OUT_STR_EL
	LDI_Z 0x0003
	ADD ZL,YL
	ADC ZH,YH
	LDI TEMP,0x05
	MCALL C5_OUT_STRN
	MCALL C5_OUT_CR

	LDI_Z _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x03
	MCALL C5_OUT_STR_EL
	LDD TEMP,Y+0x08
	MCALL C5_OUT_BYTE
	MCALL C5_OUT_CR

	LDI TEMP,0x04
	MCALL C5_OUT_STR_EL
	LDI_Z 0x0009
	ADD ZL,YL
	ADC ZH,YH
	LDI TEMP,0x04
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR

	LDI_Z _DRV_SD_LOG_STR_CID_FIELDS|0x8000
	LDI TEMP,0x05
	MCALL C5_OUT_STR_EL
	LDD TEMP_H,Y+0x0d

	LDD TEMP_L,Y+0x0e
	SWAP TEMP_H
	ANDI TEMP_H,0xf0
	MOV TEMP,TEMP_L
	SWAP TEMP
	ANDI TEMP,0x0f
	OR TEMP_H,TEMP
	ANDI TEMP_L,0x0f
	MOV TEMP,TEMP_L
	MCALL C5_OUT_NUMx8
	LDI TEMP,'.'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_H
	LDI TEMP_H,high(2000)
	LDI TEMP_L,low(2000)
	ADD TEMP_L,TEMP
	CLR TEMP
	ADC TEMP_H,TEMP
	MCALL C5_OUT_NUM16
	MCALL C5_OUT_CR

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Y
	POP_Z
	RET
.endif
.endif
