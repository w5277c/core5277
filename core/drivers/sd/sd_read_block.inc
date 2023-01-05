;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_READ_BLOCK
.else
.set DEF_DRIVER_SD_READ_BLOCK = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_block.inc"

;--------------------------------------------------------
_DRV_SD_OP_READ_BLOCK:
;--------------------------------------------------------
;Читаем блок данных
;IN: Y-адрес на переменные, X-адрес на буфер
;TEMP_EH/EL/H/L-номер блока
;OUT: ACCUM-R1 ответ
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD17
	PUSH_X
	ST X+,TEMP
	ST X+,TEMP_EH
	ST X+,TEMP_EL
	ST X+,TEMP_H
	ST x+,TEMP_L
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_READ_BLOCK__FAIL

	LDI TEMP_H,high(0x200)
	LDI TEMP_L,low(0x200)
	MCALL _DRV_SD_GET_BLOCK
	RJMP _DRV_SD_OP_READ_BLOCK__END
_DRV_SD_OP_READ_BLOCK__FAIL:
_DRV_SD_OP_READ_BLOCK__END:
	SBI PORTB,SS & 0x0f

	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
