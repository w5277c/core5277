;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_OCR
.else
.set DEF_DRIVER_SD_GET_OCR = 1

.include	"./core/drivers/sd/sd_get_r3r7.inc"

;--------------------------------------------------------
_DRV_SD_OP_GET_OCR:
;--------------------------------------------------------
;Получить OCR
;IN: X-адрес на буффер
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_L

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD58
	PUSH_X
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R3R7
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GET_OCR__ERROR
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_OP_GET_OCR__END
_DRV_SD_OP_GET_OCR__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_GET_OCR__END:
	SBI PORTB,SS & 0x0f

	POP TEMP_L
	POP ACCUM
	RET
.endif
