;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.04.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_FORMAT
.else
.set DEF_DRIVER_FS_FORMAT = 1

.include	"./core/drivers/fs/_fs_name_check.inc"
.include	"./mem/ram_fill16.inc"
.include	"./str/str_len.inc"
.include	"./str/strn_copy.inc"
.include	"./core/drivers/sd/sd_write_block.inc"

;--------------------------------------------------------
_DRV_FS_OP_FORMAT:
;--------------------------------------------------------
;Создаем новую FS (большинство полей содержит 0x00)
;IN: Y-адрес на переменные,X-RAM адрес на название диска
;OUT: TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH FLAGS

	;Проверка имени
	MCALL STR_LEN
	MOV LOOP_CNTR,TEMP
	MCALL _DRV_FS_NAME_CHECK
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_FORMAT__END

	PUSH_X
	LDD XH,Y+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Y+_DRV_FS_BUFFER_ADDR+0x01
	LDI TEMP,0x00
	LDI TEMP_H,high(DRV_FS_BLOCK_SIZE)
	LDI TEMP_L,low(DRV_FS_BLOCK_SIZE)
	MCALL RAM_FILL16
	MOVW ZL,XL
	POP_X

	;Номер свободного элемента
	STD Z+DRV_FS_ITEM_LEN+0x00,C0x00
	LDI TEMP,0x01
	STD Z+DRV_FS_ITEM_LEN+0x01,TEMP

	LDD TEMP,Y+_DRV_FS_RTC_DRV_ID
	LDI ACCUM,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x05
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	STD Z+DRV_FS_ITEM_CTIME+0x00,TEMP_EH
	STD Z+DRV_FS_ITEM_CTIME+0x01,TEMP_EL
	STD Z+DRV_FS_ITEM_CTIME+0x02,TEMP_H
	STD Z+DRV_FS_ITEM_CTIME+0x03,TEMP_L
	LDI TEMP,DRV_FS_TYPE_DIR
	STD Z+DRV_FS_ITEM_OPT,TEMP

	PUSH_X
	PUSH_Z
	POP_X
	POP_Z
	ADIW XL,DRV_FS_ITEM_NAME
	MCALL STRN_COPY

	;Вызываем драйвер SD для записи блока
	LDD TEMP,Y+_DRV_FS_SD_DRV_ID
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI TEMP,DRV_RESULT_OK
	CPI ACCUM,0x00
	BREQ PC+0x02
	LDI TEMP,DRV_RESULT_ERROR

_DRV_FS_OP_FORMAT__END:
	POP FLAGS
	POP ACCUM
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_X
	RET
.endif
