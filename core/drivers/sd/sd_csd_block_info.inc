;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SD_CSD_BLOCK_INFO
.else
.set DEF_C5_SD_CSD_BLOCK_INFO = 1

;--------------------------------------------------------
DRV_SD_CSD_BLOCK_INFO:
;Возвращаем данные по блоку
;IN: Z-адрес на блок данных
;OUT: TEMP_H/L - размер блока для чтения,
;TEMP_EH/EL - размер блока для записи,
;FLAGS-поддержка partial (5-бит чтение, 6-бит запись),
;ошибка (7 бит)
;--------------------------------------------------------
	PUSH TEMP

	CLR FLAGS

	LDD TEMP,Z+0x05
	ANDI TEMP,0x0f
	CPI TEMP,0x09
	BRNE PC+0x04
	LDI TEMP_H,high(512)
	LDI TEMP_L,low(512)
	RJMP DRV_SD_CSD_BLOCK_INFO__READ_BL_LEN_OK
	CPI TEMP,0x0a
	BRNE PC+0x04
	LDI TEMP_H,high(1024)
	LDI TEMP_L,low(1024)
	RJMP DRV_SD_CSD_BLOCK_INFO__READ_BL_LEN_OK
	CPI TEMP,0x0b
	BRNE PC+0x04
	LDI TEMP_H,high(2048)
	LDI TEMP_L,low(2048)
	RJMP DRV_SD_CSD_BLOCK_INFO__READ_BL_LEN_OK
	ORI FLAGS,(1<<0x07)
	RJMP DRV_SD_CSD_BLOCK_INFO__END
DRV_SD_CSD_BLOCK_INFO__READ_BL_LEN_OK:
	LDD TEMP,Z+0x06
	SBRC TEMP,0x07
	ORI FLAGS,(1<<0x05)

	PUSH TEMP_L
	LDD TEMP,Z+0x0c
	LDD TEMP_L,Z+0x0d
	LSL TEMP_L
	ROL TEMP
	LSL TEMP_L
	ROL TEMP
	POP TEMP_L
	ANDI TEMP,0x0f
	CPI TEMP,0x09
	BRNE PC+0x04
	LDI TEMP_EH,high(512)
	LDI TEMP_EL,low(512)
	RJMP DRV_SD_CSD_BLOCK_INFO__WRITE_BL_LEN_OK
	CPI TEMP,0x0a
	BRNE PC+0x04
	LDI TEMP_EH,high(1024)
	LDI TEMP_EL,low(1024)
	RJMP DRV_SD_CSD_BLOCK_INFO__WRITE_BL_LEN_OK
	CPI TEMP,0x0b
	BRNE PC+0x04
	LDI TEMP_EH,high(2048)
	LDI TEMP_EL,low(2048)
	RJMP DRV_SD_CSD_BLOCK_INFO__WRITE_BL_LEN_OK
	ORI FLAGS,(1<<0x07)
	RJMP DRV_SD_CSD_BLOCK_INFO__END
DRV_SD_CSD_BLOCK_INFO__WRITE_BL_LEN_OK:
	LDD TEMP,Z+0x0d
	SBRC TEMP,0x05
	ORI FLAGS,(1<<0x06)
DRV_SD_CSD_BLOCK_INFO__END:
	POP TEMP
	RET
.endif

