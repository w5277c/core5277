;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_ERASE_BLOCKS
.else
.set DEF_DRIVER_SD_ERASE_BLOCKS = 1

.include	"./core/drivers/sd/sd_get_status.inc"
.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_r1b.inc"

;--------------------------------------------------------
_DRV_SD_OP_ERASE_BLOCKS:
;--------------------------------------------------------
;Стираем несколько блоков
;IN: TEMP_EH/EL/H/L-номер 1-го блока,
;Z-количество блоков(0==65536 блоков)!!!
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X

	CBI PORTB,SS & 0x0f
	PUSH TEMP_L
	PUSH_X
	MOVW XL,YL
	ADIW XL, _DRV_SD_CMD_BUFFER
	LDI TEMP,_DRV_SD_CMD32
	ST X+,TEMP
	ST X+,TEMP_EH
	ST X+,TEMP_EL
	ST X+,TEMP_H
	ST x+,TEMP_L
	SBIW XL,0x05
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	POP_X
	MCALL _DRV_SD_GET_R1
	POP TEMP_L

	CPI ACCUM,0x00
	BREQ PC+0x02
	RJMP _DRV_SD_OP_ERASE_BLOCKS__ERROR

	SBIW ZL,0x01
	ADD TEMP_L,ZL
	ADC TEMP_H,ZH
	ADC TEMP_EL,C0x00
	ADC TEMP_EH,C0x00
	PUSH_X
	MOVW XL,YL
	ADIW XL, _DRV_SD_CMD_BUFFER
	LDI TEMP,_DRV_SD_CMD33
	ST X+,TEMP
	ST X+,TEMP_EH
	ST X+,TEMP_EL
	ST X+,TEMP_H
	ST x+,TEMP_L
	SBIW XL,0x05
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	POP_X
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BREQ PC+0x02
	RJMP _DRV_SD_OP_ERASE_BLOCKS__ERROR

	LDI TEMP,_DRV_SD_CMD38
	PUSH_X
	MOVW XL,YL
	ADIW XL, _DRV_SD_CMD_BUFFER
	LDI TEMP,_DRV_SD_CMD38
	ST X+,TEMP
	LDI TEMP,0x52
	ST X+,TEMP
	ST X+,TEMP
	ST X+,TEMP
	ST X+,TEMP
	SBIW XL,0x05
	LDI TEMP_L,0x05
	MCALL _DRV_SD__SEND_CMD
	POP_X
	MCALL _DRV_SD_GET_R1b
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__ERROR

	;MCALL _DRV_SD_WAIT
	MCALL _DRV_SD_OP_GET_STATUS
	CPI TEMP_H,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__ERROR
	CPI TEMP_L,0x00
	BRNE _DRV_SD_OP_ERASE_BLOCKS__ERROR
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_OP_ERASE_BLOCKS__END
_DRV_SD_OP_ERASE_BLOCKS__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_ERASE_BLOCKS__END:
	SBI PORTB,SS & 0x0f

	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	RET
.endif
