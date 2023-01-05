;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_WRITE_BLOCK
.else
.set DEF_DRIVER_SD_WRITE_BLOCK = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_set_block.inc"
.include	"./core/drivers/sd/sd_get_dataresp_token.inc"
.include	"./core/drivers/sd/sd_get_status.inc"

;--------------------------------------------------------
_DRV_SD_OP_WRITE_BLOCK:
;--------------------------------------------------------
;Пишем блок данных
;IN: X-адрес на буффер,Y-адрес на переменные
;TEMP_EH/EL/H/L-номер блока
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X

	CBI PORTB,SS & 0x0f
	PUSH_X
	LDI XH,0x00
	LDI XL,_DRV_SD_WRITE_CMD_BUFFER
	ADD XL,YL
	ADC XH,YH
	LDI TEMP,_DRV_SD_CMD24
	PUSH_X
	ST X+,TEMP
	ST X+,TEMP_EH
	ST X+,TEMP_EL
	ST X+,TEMP_H
	ST X+,TEMP_L
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	POP_X

	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_WRITE_BLOCK__ERROR
	LDI TEMP_H,high(0x200)
	LDI TEMP_L,low(0x200)
	MCALL _DRV_SD_SET_BLOCK
	MCALL _DRV_SD_GET_DATARESP_TOKEN
	CPI ACCUM,_DRV_SD_DR_TOKEN_ACCEPTED
	BRNE _DRV_SD_OP_WRITE_BLOCK__FAIL

	MCALL _DRV_SD_OP_GET_STATUS
	CPI TEMP_H,0x00
	BRNE _DRV_SD_OP_WRITE_BLOCK__ERROR
	CPI TEMP_L,0x00
	BRNE _DRV_SD_OP_WRITE_BLOCK__ERROR
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_OP_WRITE_BLOCK__END

_DRV_SD_OP_WRITE_BLOCK__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_WRITE_BLOCK__END:
	SBI PORTB,SS & 0x0f

	POP_X
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	POP LOOP_CNTR
	RET
.endif
