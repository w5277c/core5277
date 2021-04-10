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
;OUT: ACCUM-R1 ответ
;Y-адрес на буфер
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH_X

	MOV YH,XH
	MOV YL,XL

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD58
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R3R7
	SBI PORTB,SS & 0x0f
_DRV_SD_OP_GET_OCR__END:

	POP_X
	POP TEMP_L
	POP TEMP
	RET
.endif
