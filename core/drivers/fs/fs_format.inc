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

;--------------------------------------------------------
_DRV_FS_OP_FORMAT:
;--------------------------------------------------------
;Создаем новую FS (большинство полей содержит 0x00)
;IN: Y-адрес на переменные,X-RAM адрес на название диска,
;TEMP_H/L-размер блоков под ROOT(1-65535)
;OUT: TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH_T32
	PUSH LOOP_CNTR
	PUSH FLAGS

	;Проверка имени
	MCALL STR_LEN
	MOV LOOP_CNTR,TEMP
	MCALL _DRV_FS_NAME_CHECK
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_FORMAT__END

	PUSH_T16
	PUSH_X
	LDD XH,Y+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Y+_DRV_FS_BUFFER_ADDR+0x01
	LDI TEMP,0x00
	LDI TEMP_H,high(DRV_FS_BLOCK_SIZE)
	LDI TEMP_L,low(DRV_FS_BLOCK_SIZE)
	MCALL RAM_FILL16
	MOVW ZL,XL
	POP_X
	POP_T16

	;Номер свободного элемента
	LDI TEMP,0x52
	STD Z+DRV_FS_ROOT_MAGIC+0x00,TEMP
	LDI TEMP,0x77
	STD Z+DRV_FS_ROOT_MAGIC+0x01,TEMP
	LDI TEMP,0x01
	STD Z+DRV_FS_ROOT_VERSION,TEMP
	LDI TEMP,0x10
	STD Z+DRV_FS_ROOT_CURRENT_BLK+0x00,C0x00
	STD Z+DRV_FS_ROOT_CURRENT_BLK+0x01,C0x00
	STD Z+DRV_FS_ROOT_CURRENT_BLK+0x02,TEMP_H
	STD Z+DRV_FS_ROOT_CURRENT_BLK+0x03,TEMP_L
	STD Z+DRV_FS_ROOT_USED_BLKS+0x00,C0x00
	STD Z+DRV_FS_ROOT_USED_BLKS+0x01,C0x00
	STD Z+DRV_FS_ROOT_USED_BLKS+0x02,TEMP_H
	STD Z+DRV_FS_ROOT_USED_BLKS+0x03,TEMP_L
	STD Z+DRV_FS_ROOT_ROOT_BLKS+0x00,TEMP_H
	STD Z+DRV_FS_ROOT_ROOT_BLKS+0x01,TEMP_L

	LDD TEMP,Y+_DRV_FS_RTC_DRV_ID
	LDI FLAGS,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_FORMAT__END
	STD Z+DRV_FS_ROOT_ETIME+0x00,TEMP_EH
	STD Z+DRV_FS_ROOT_ETIME+0x01,TEMP_EL
	STD Z+DRV_FS_ROOT_ETIME+0x02,TEMP_H
	STD Z+DRV_FS_ROOT_ETIME+0x03,TEMP_L
	LDI TEMP,DRV_FS_TYPE_DIR
	STD Z+DRV_FS_ROOT_OPT,TEMP

	PUSH_X
	PUSH_Z
	POP_X
	POP_Z
	ADIW XL,DRV_FS_ITEM_NAME
	LDI LOOP_CNTR,0x0c
	MCALL STRN_COPY

	;Вызываем драйвер диска для записи блока
	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_WRITE
	LDI_T32 0x00000000
	MCALL C5_EXEC

_DRV_FS_OP_FORMAT__END:
	POP FLAGS
	POP LOOP_CNTR
	POP_T32
	POP_Z
	POP_X
	RET
.endif
