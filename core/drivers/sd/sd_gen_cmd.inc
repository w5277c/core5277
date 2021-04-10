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
;IN: X-адрес на буфер,Z-адрес на переменные
;ACCUM-тип операции(0x00-запись,0x01-чтение)
;OUT: ACCUM-R1 ответ
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_X
	PUSH_Y

	CBI PORTB,SS & 0x0f
	LDI TEMP,_DRV_SD_CMD56
	PUSH_X
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	ST X+,TEMP
	ST X+,TEMP
	ST x+,ACCUM
	POP_X
	LDI TEMP_L,0x05

	MCALL _DRV_SD__SEND_CMD

	MOV TEMP,ACCUM
	MCALL _DRV_SD_GET_R1
	CPI ACCUM,0x00
	BRNE _DRV_SD_OP_GEN_CMD__FAIL

	SBRS TEMP,0x00
	RJMP _DRV_SD_OP_GEN_CMD__WRITE

	LDI TEMP_H,high(512)
	LDI TEMP_L,low(512)
	MCALL _DRV_SD_GET_BLOCK
	RJMP _DRV_SD_OP_GEN_CMD__END

_DRV_SD_OP_GEN_CMD__WRITE:
	LDI ACCUM,0xff
_DRV_SD_OP_GEN_CMD__FAIL:
_DRV_SD_OP_GEN_CMD__END:
	SBI PORTB,SS & 0x0f

	POP_Y
	POP_X
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
