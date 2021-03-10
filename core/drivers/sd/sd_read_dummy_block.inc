;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_READ_DUMMY_BLOCK
.else
.set DEF_DRIVER_SD_READ_DUMMY_BLOCK = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_dummy_block.inc"

;--------------------------------------------------------
_DRV_SD_OP_READ_DUMMY_BLOCK:
;--------------------------------------------------------
;Читаем блок данных(фикция)
;IN: Z-адрес на переменные, X-адрес на буфер
;TEMP_EH/EL/H/L-номер блока
;ACCUM-ожидаемый байт в записи
;OUT: ACCUM-R1 ответ или ответ sd_get_dummy_block
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X
	PUSH_Y
	PUSH ACCUM

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
	BRNE _DRV_SD_OP_READ_DUMMY_BLOCK__FAIL
	POP ACCUM
	MCALL _DRV_SD_GET_DUMMY_BLOCK
	RJMP _DRV_SD_OP_READ_DUMMY_BLOCK__END
_DRV_SD_OP_READ_DUMMY_BLOCK__FAIL:
	POP TEMP
_DRV_SD_OP_READ_DUMMY_BLOCK__END:
	SBI PORTB,SS & 0x0f

	POP_Y
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
