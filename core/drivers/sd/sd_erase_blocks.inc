;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_ERASE_BLOCKS
.else
.set DEF_DRIVER_SD_ERASE_BLOCKS = 1

.include	"./core/time32_mark.inc"
.include	"./core/time32_delta.inc"
.include	"./core/drivers/sd/sd_get_status.inc"
.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_r1b5.inc"

;--------------------------------------------------------
_DRV_SD_OP_ERASE_BLOCKS:
;--------------------------------------------------------
;Стираем несколько блоков
;IN: Z-адрес на переменные
;TEMP_EH/EL/H/L-номер 1-го блока,
;Y-количество блоков(0==65536 блоков)!!!
;OUT: ACCUM-R1 ответ
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X
	PUSH_Y

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD32
	PUSH_X
	ST X+,TEMP
	ST X+,TEMP_EH
	ST X+,TEMP_EL
	ST X+,TEMP_H
	ST x+,TEMP_L
	POP_X
	PUSH TEMP_L
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	MCALL _DRV_SD_GET_R1
	POP TEMP_L
	CPI ACCUM,0x00
	BREQ PC+0x02
	RJMP _DRV_SD_OP_ERASE_BLOCKS__END

	CLR TEMP
	SBIW YL,0x01
	ADD TEMP_L,YL
	ADC TEMP_H,YH
	ADC TEMP_EL,TEMP
	ADC TEMP_EH,TEMP
	LDI TEMP,_DRV_SD_CMD33
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
	BREQ PC+0x02
	RJMP _DRV_SD_OP_ERASE_BLOCKS__END

	LDI TEMP,_DRV_SD_CMD38
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x52
	ST X+,TEMP
	ST X+,TEMP
	ST X+,TEMP
	ST X,TEMP
	POP_X
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD

	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_MARK
	MCALL _DRV_SD_GET_R1b
	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_DELTA
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__FAIL

	MCALL _DRV_SD_WAIT
	MCALL _DRV_SD_OP_GET_STATUS
	CPI TEMP_H,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__FAIL
	CPI TEMP_L,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__FAIL
	LDI ACCUM,0x00
	RJMP _DRV_SD_OP_ERASE_BLOCKS__END
_DRV_SD_OP_ERASE_BLOCKS__FAIL:
	LDI ACCUM,0xff
_DRV_SD_OP_ERASE_BLOCKS__END:
	SBI PORTB,SS & 0x0f

	POP_Y
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP
	RET
.endif
