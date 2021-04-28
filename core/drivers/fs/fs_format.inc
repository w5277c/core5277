;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.04.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_FORMAT
.else
.set DEF_DRIVER_FS_FORMAT = 1

.include	"./mem/ram_fill16.inc"
.include	"./mem/ram_copy8.inc"
.include	"./core/drivers/sd/sd_write_block.inc"

;--------------------------------------------------------
_DRV_FS_OP_FORMAT:
;--------------------------------------------------------
;Создаем новую FS (большинство полей содержит 0x00)
;IN: Z-адрес на переменные,Y-адрес на название диска
;TEMP-длина названия диска
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

	CPI TEMP,DRV_FS_NAME_SIZE+0x01
	BRCS PC+0x03
	LDI FLAGS,DRV_FS_RESULT_INVALID_NAME
	RJMP _DRV_FS_OP_FORMAT__END

	PUSH_Y
	PUSH TEMP
	LDD XH,Z+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Z+_DRV_FS_BUFFER_ADDR+0x01
	LDI TEMP,0x00
	LDI TEMP_H,high(DRV_FS_BLOCK_SIZE)
	LDI TEMP_L,low(DRV_FS_BLOCK_SIZE)
	MCALL RAM_FILL16
	MOV YH,XH
	MOV YL,XL

	LDI TEMP,_DRV_FS_RTC_DRV_ID
	LDI ACCUM,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	STD Y+DRV_FS_ITEM_CTIME+0x00,TEMP_EH
	STD Y+DRV_FS_ITEM_CTIME+0x01,TEMP_EL
	STD Y+DRV_FS_ITEM_CTIME+0x02,TEMP_H
	STD Y+DRV_FS_ITEM_CTIME+0x03,TEMP_L

	LDI TEMP,(1<<DRV_FS_OPT_HIDDEN)
	STD Y+DRV_FS_ITEM_OPT,TEMP

	POP LOOP_CNTR
	POP_X
	MOV ZH,YH
	MOV ZL,YL
	ADIW ZL,DRV_FS_ITEM_NAME
	MCALL RAM_COPY8
	CPI LOOP_CNTR,DRV_FS_NAME_SIZE
	BREQ PC+0x04	;TODO надо проверить
	ADIW ZL,LOOP_CNTR
	LDI LOOP_CNTR,0x00
	ST Z,LOOP_CNTR

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
