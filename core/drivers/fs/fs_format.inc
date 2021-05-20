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
.include	"./str/strn_copy.inc"
.include	"./core/drivers/sd/sd_write_block.inc"

;--------------------------------------------------------
_DRV_FS_OP_FORMAT:
;--------------------------------------------------------
;Создаем новую FS (большинство полей содержит 0x00)
;IN: Z-адрес на переменные,Y-адрес на название диска
;OUT: FLAGS-результат DRV_FS_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH ACCUM

	MCALL _FS_NAME_CHECK
	CPI FLAGS,DRV_FS_RESULT_OK
	BRNE _DRV_FS_OP_FORMAT__END
	MOV LOOP_CNTR,TEMP

	PUSH_Y
	LDD XH,Z+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Z+_DRV_FS_BUFFER_ADDR+0x01
	LDI TEMP,0x00
	LDI TEMP_H,high(DRV_FS_BLOCK_SIZE)
	LDI TEMP_L,low(DRV_FS_BLOCK_SIZE)
	MCALL RAM_FILL16
	MOV YH,XH
	MOV YL,XL

	;Номер свободного элемента
	LDI TEMP,0x00
	STD Y+DRV_FS_ITEM_LEN+0x00,TEMP
	LDI TEMP,0x01
	STD Y+DRV_FS_ITEM_LEN+0x01,TEMP

	LDI TEMP,_DRV_FS_RTC_DRV_ID
	LDI ACCUM,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	STD Y+DRV_FS_ITEM_CTIME+0x00,TEMP_EH
	STD Y+DRV_FS_ITEM_CTIME+0x01,TEMP_EL
	STD Y+DRV_FS_ITEM_CTIME+0x02,TEMP_H
	STD Y+DRV_FS_ITEM_CTIME+0x03,TEMP_L
	LDI TEMP,DRV_FS_TYPE_DIR
	STD Y+DRV_FS_ITEM_OPT,TEMP
	POP_Y

	ADIW XL,DRV_FS_ITEM_NAME
	MCALL STR_COPY

	LDI TEMP,0x00
	ADIW XL,LOOP_CNTR
	CPI LOOP_CNTR,0x0c
	BREQ PC+0x02
	ST X,TEMP

	;Вызываем драйвер SD для записи блока
	LDD TEMP,_DRV_FS_SD_DRV_ID
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL C5_EXEC
	LDI FLAGS,DRV_FS_RESULT_OK
	CPI ACCUM,0x00
	BREQ PC+0x02
	LDI FLAGS,DRV_FS_RESULT_OP_FAILED

	;Очищаем все элементы
	LDD XH,Z+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Z+_DRV_FS_BUFFER_ADDR+0x01
	LDI TEMP,0x00
	LDI TEMP_H,high(DRV_FS_BLOCK_SIZE)
	LDI TEMP_L,low(DRV_FS_BLOCK_SIZE)
	MCALL RAM_FILL16
	;---
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
_DRV_FS_OP_FORMAT__LOOP:
	LDD TEMP,_DRV_FS_SD_DRV_ID
	LDI FLAGS,DRV_SD_OP_WRITE_BLOCK
	MCALL C5_EXEC
	LDI FLAGS,DRV_FS_RESULT_OK
	CPI ACCUM,0x00
	BREQ PC+0x03
	LDI FLAGS,DRV_FS_RESULT_OP_FAILED
	RJMP _DRV_FS_OP_FORMAT__END
	;Инкремент
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	LDI TEMP,0x00
	ADC TEMP_H,TEMP
	CPI TEMP_H,0x00
	BRNE _DRV_FS_OP_FORMAT__LOOP
	CPI TEMP_L,0x00
	BRNE _DRV_FS_OP_FORMAT__LOOP

_DRV_FS_OP_FORMAT__END:
	POP ACCUM
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
.endif
