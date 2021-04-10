;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_RESET
.else
.set DEF_DRIVER_SD_RESET = 1

.include	"./core/drivers/sd/sd_get_r1.inc"

;--------------------------------------------------------
_DRV_SD_OP_RESET:
;--------------------------------------------------------
;Получить CID
;IN: X-адрес на выделенную память
;OUT: ACCUM-R1 ответ
;Y-адрес на буфер
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L

	CBI PORTB,SS & 0x0f

	LDI TEMP,_DRV_SD_CMD0
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
	MCALL _DRV_SD_GET_R1

	SBI PORTB,SS & 0x0f

	POP TEMP_L
	POP TEMP
	RET
.endif
