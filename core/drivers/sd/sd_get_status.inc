;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_STATUS
.else
.set DEF_DRIVER_SD_GET_STATUS = 1

.include	"./core/drivers/sd/sd_get_r2.inc"

;--------------------------------------------------------
_DRV_SD_OP_GET_STATUS:
;--------------------------------------------------------
;Получить состояние карты
;IN: X-адрес на выделенную память
;OUT: TEMP_H/L-R2 ответ
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD13
	PUSH_X
	ST X+,TEMP
	CLR TEMP
	ST X+,TEMP
	ST X+,TEMP
	ST X+,TEMP
	ST x+,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R2
	SBI PORTB,SS & 0x0f

	POP TEMP
	POP ACCUM
	RET
.endif
