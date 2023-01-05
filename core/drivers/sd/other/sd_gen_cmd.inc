;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GEN_CMD
.else
.set DEF_DRIVER_SD_GEN_CMD = 1

.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_block.inc"

;--------------------------------------------------------
_DRV_SD_OP_GEN_CMD:
;--------------------------------------------------------
;Чтение/запись данных контроллера
;IN: X-адрес на буфер,Y-адрес на переменные
;ACCUM-тип операции(0x00-запись,0x01-чтение)
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD56
	PUSH_X
	ST X+,TEMP
	ST X+,C0x00
	ST X+,C0x00
	ST X+,C0x00
	ST x+,ACCUM
	POP_X
	LDI TEMP_L,0x05

	MCALL _DRV_SD__SEND_CMD

	MOV TEMP,ACCUM
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GEN_CMD__ERROR

	SBRS TEMP,0x00
	RJMP _DRV_SD_OP_GEN_CMD__WRITE

	LDI TEMP_H,high(0x200)
	LDI TEMP_L,low(0x200)
	MCALL _DRV_SD_GET_BLOCK
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_SD_OP_GEN_CMD__END

_DRV_SD_OP_GEN_CMD__WRITE:
	LDI TEMP,DRV_RESULT_UNSUPPORTED
	RJMP _DRV_SD_OP_GEN_CMD__END
_DRV_SD_OP_GEN_CMD__ERROR:
	LDI TEMP,DRV_RESULT_ERROR
_DRV_SD_OP_GEN_CMD__END:
	SBI PORTB,SS & 0x0f

	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
.endif
