;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_CSD
.else
.set DEF_DRIVER_SD_GET_CSD = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_block.inc"

;--------------------------------------------------------
_DRV_SD_OP_GET_CSD:
;--------------------------------------------------------
;Получить CSD
;IN: X-адрес на буффер
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD9
	PUSH_X
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GET_CSD__ERROR
	LDI TEMP_H,0x00
	LDI TEMP_L,0x10
	MCALL _DRV_SD_GET_BLOCK
	RJMP _DRV_SD_OP_GET_CSD__END
_DRV_SD_OP_GET_CSD__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_GET_CSD__END:
	SBI PORTB,SS & 0x0f

	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
.endif
