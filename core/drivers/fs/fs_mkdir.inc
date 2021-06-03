;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.04.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_MKDIR
.else
.set DEF_DRIVER_FS_MKDIR = 1

.include	"./mem/ram_fill16.inc"
.include	"./mem/ram_copy8.inc"
.include	"./core/drivers/sd/sd_read_block.inc"
.include	"./core/drivers/sd/sd_write_block.inc"

;--------------------------------------------------------
_DRV_FS_OP_MKDIR:
;--------------------------------------------------------
;Создаем директорию
;IN: Z-адрес на переменные,Y-адрес на название
;директории, TEMP_H/L-родительский элемент
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

	;Проверка имени
	LDI FLAGS,DRV_FS_RESULT_INVALID_NAME
	MCALL STR_LEN
	CPI TEMP,0x00
	BREQ _DRV_FS_OP_MKDIR__END
	CPI TEMP,DRV_FS_NAME_SIZE+0x01
	BRCC _DRV_FS_OP_MKDIR__END
	MOV LOOP_CNTR,TEMP

	;Проверка родителя
	MCALL _DRV_FS_ITEM_INFO
	CPI FLAGS,DRV_FS_RESULT_OK
	BRNE _DRV_FS_OP_MKDIR__END
	LDI FLAGS,DRV_FS_RESULT_INVALID_ITEM
	LDD TEMP,Z+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	ANDI TEMP,0xf0
	CPI TEMP,DRV_FS_TYPE_DIR
	BRNE _DRV_FS_OP_MKDIR__END

	;Получаю номер последнего занятого элемента
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL _DRV_FS_ITEM_INFO
	LDI TEMP_H,Z+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_LEN+0x00
	LDI TEMP_L,Z+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_LEN+0x01
	CPI TEMP_H,0xff
	BRNE PC+0x03
	CPI TEMP_L,0xff
	BREQ _DRV_FS_OP_MKDIR__GET_REMOVED_ITEM
	;Есть свободный элемент
	INC TEMP_L
	CLR TEMP
	ADC TEMP_H,TEMP
	RJMP _DRV_FS_OP_MKDIR__GOT_ITEM_ID
_DRV_FS_OP_MKDIR__GET_REMOVED_ITEM:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	MCALL _DRV_FS_GET_REMOVED_ITEM
_DRV_FS_OP_MKDIR__GOT_ITEM_ID:



	MCALL _DRV_FS_ITEM_WRITE
	MCALL _DRV_FS_LAST_ITEM_ID_UPDATE







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
	ADIW YL,LOOP_CNTR
	LDI LOOP_CNTR,0x00
	ST Y,LOOP_CNTR

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

_DRV_FS_OP_MKDIR__END:
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
