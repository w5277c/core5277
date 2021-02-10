;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_SET_DUMMY_BLOCK
.else
.set DEF_DRIVER_SD_SET_DUMMY_BLOCK = 1

;--------------------------------------------------------
_DRV_SD_SET_DUMMY_BLOCK:
;--------------------------------------------------------
;Передача блока данных
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Y

	LDI YH,high(512)
	LDI YL,low(512)

	LDI TEMP,0xfe
	MCALL _DRV_SD_BYTE_SEND

	LDI TEMP,0x52
_DRV_SD_SET_DUMMY_BLOCK__DATA:
	MCALL _DRV_SD_BYTE_SEND
	SBIW YL,0x01
	BRNE _DRV_SD_SET_DUMMY_BLOCK__DATA
	;Пишем CRC
	LDI TEMP,0x19
	MCALL _DRV_SD_BYTE_SEND
	LDI TEMP,0x73
	MCALL _DRV_SD_BYTE_SEND

	POP_Y
	POP TEMP
	RET
.endif
