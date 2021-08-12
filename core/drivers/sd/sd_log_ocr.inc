;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SD_LOG_OCR
.else
.set DEF_C5_SD_LOG_OCR = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_str.inc"
.include	"./core/io/out_str_el.inc"
.include	"./core/io/out_cr.inc"

_DRV_SD_LOG_STR_OCR:
	.db "OCR register:",0x0d,0x0a,0x00
_DRV_SD_LOG_STR_OCR_VOLTAGES:
	.db " 2.7-2.8",0x00," 2.8-2.9",0x00," 2.9-3.0",0x00," 3.0-3.1",0x00," 3.1-3.2",0x00," 3.2-3.3",0x00," 3.3-3.4",0x00," 3.4-3.5",0x00," 3.5-3.6",0x00
_DRV_SD_LOG_STR_OCR_24:
	.db " Switching to 1.8V Accepted (S18A)",0x00
_DRV_SD_LOG_STR_OCR_27:
	.db " Over 2TB support Status (CO2T)",0x00
_DRV_SD_LOG_STR_OCR_29:
	.db " UHS-II Card Status",0x00
_DRV_SD_LOG_STR_OCR_30:
	.db " Card Capacity Status (CCS)",0x00
_DRV_SD_LOG_STR_OCR_31:
	.db " Card power up status bit (busy)",0x00

;--------------------------------------------------------
DRV_SD_LOG_OCR:
;Логирование OCR
;IN: X-адрес на блок данных
;--------------------------------------------------------
	PUSH_Z
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM

	MOVW YL,XL

	LDI_Z _DRV_SD_LOG_STR_OCR|0x8000
	MCALL C5_OUT_STR

	LDD TEMP,Y+0x02
	SBRS TEMP,0x07
	RJMP PC+0x05
	LDI_Z _DRV_SD_LOG_STR_OCR_VOLTAGES|0x8000
	LDI TEMP,0x00
	MCALL C5_OUT_STR_EL
	MCALL C5_OUT_CR

	LDD ACCUM,Y+0x01
	LDI TEMP,0x01
_DRV_SD_LOG_OCR__VOLTAGES_LOOP:
	LSR ACCUM
	BRCC PC+0x05
	MCALL C5_OUT_STR_EL
	MCALL C5_OUT_CR
	INC TEMP
	CPI TEMP,0x09
	BRNE _DRV_SD_LOG_OCR__VOLTAGES_LOOP

	LDD TEMP,Y+0x00
	SBRS TEMP,0x07
	BRCC PC+0x07
	LDI_Z _DRV_SD_LOG_STR_OCR_24|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR
	SBRS TEMP,0x04
	BRCC PC+0x07
	LDI_Z _DRV_SD_LOG_STR_OCR_27|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR
	SBRS TEMP,0x02
	BRCC PC+0x07
	LDI_Z _DRV_SD_LOG_STR_OCR_29|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR
	SBRS TEMP,0x01
	BRCC PC+0x07
	LDI_Z _DRV_SD_LOG_STR_OCR_30|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR
	SBRS TEMP,0x00
	BRCC PC+0x07
	LDI_Z _DRV_SD_LOG_STR_OCR_31|0x8000
	MCALL C5_OUT_STR
	MCALL C5_OUT_CR

	POP ACCUM
	POP TEMP
	POP_Y
	POP_Z
	RET
.endif
.endif
