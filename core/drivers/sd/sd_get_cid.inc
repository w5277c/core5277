;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021	w5277c@gmail.com			Начало
;04.01.2023	konstantin@5277.ru		Актуализация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_CID
.else
.set DEF_DRIVER_SD_GET_CID = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_block.inc"
;--------------------------------------------------------
_DRV_SD_OP_GET_CID:
;--------------------------------------------------------
;Получить CID
;IN: X-адрес на буффер
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD10
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	SBIW XL,0x05
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GET_CID__ERROR
	LDI TEMP_H,0x00
	LDI TEMP_L,0x10
	MCALL _DRV_SD_GET_BLOCK
	RJMP _DRV_SD_OP_GET_CID__END
_DRV_SD_OP_GET_CID__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_GET_CID__END:
	SBI PORTB,SS & 0x0f

	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
.endif
