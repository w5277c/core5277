;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.10.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_MF
.else
.set DEF_DRIVER_FS_MF = 1

.include "./str/str_len.inc"
.include	"./core/drivers/fs/_fs_name_check.inc"
.include	"./core/drivers/fs/_fs_get_removed_item.inc"
.include	"./mem/ram_fill.inc"
.include	"./mem/ram_copy.inc"

;--------------------------------------------------------
_DRV_FS_OP_MF:
;--------------------------------------------------------
;Создаем файл
;IN: Y-адрес на переменные,Z-RAM адрес на название
;файла, TEMP_H/L-директория, FLAGS-параметры
;DRV_FS_OPT_...(H-NIBBLE)
;OUT: FLAGS-результат DRV_RESULT_...
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
	MOVW XL,ZL
	MCALL STR_LEN
	MOV LOOP_CNTR,TEMP
	MCALL _DRV_FS_NAME_CHECK
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MF__END

	;Проверка родителя
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MF__END
	LDD TEMP,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	ANDI TEMP,0x0f
	CPI TEMP,DRV_FS_TYPE_DIR
	BRNE _DRV_FS_OP_MF__INVALID

	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L
	;Получаю номер последнего занятого элемента
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MF__END
	LDI TEMP_H,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_LEN+0x00
	LDI TEMP_L,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_LEN+0x01
	CPI TEMP_H,0xff
	BRNE PC+0x03
	CPI TEMP_L,0xff
	BREQ _DRV_FS_OP_MF__GET_REMOVED_ITEM
	;Есть свободный элемент
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	LDI ACCUM,0x00
	RJMP _DRV_FS_OP_MF__GOT_ITEM_ID
_DRV_FS_OP_MF__GET_REMOVED_ITEM:
	LDI TEMP_H,0x00
	LDI TEMP_L,0x01
	MCALL _DRV_FS_GET_REMOVED_ITEM
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MD__END
	LDI ACCUM,0x01
_DRV_FS_OP_MF__GOT_ITEM_ID:

	PUSH LOOP_CNTR
	LDD XH,Y+_DRV_FS_ITEM_BUFFER+0x00
	LDD XL,Y+_DRV_FS_ITEM_BUFFER+0x01
	LDI TEMP,0x00
	LDI LOOP_CNTR,DRV_FS_ITEM_SIZE
	MCALL RAM_FILL
	POP LOOP_CNTR
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_PARENT_ID+0x00,TEMP_EH
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_PARENT_ID+0x01,TEMP_EL

	PUSH ACCUM
	LDI TEMP,_DRV_FS_RTC_DRV_ID
	LDI ACCUM,DRV_RTC_GET_TIMESTAMP
	MCALL C5_EXEC
	POP ACCUM
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x05
	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_CTIME+0x00,TEMP_EH
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_CTIME+0x01,TEMP_EL
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_CTIME+0x02,TEMP_H
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_CTIME+0x03,TEMP_L

	LDI TEMP,DRV_FS_TYPE_DIR
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT,TEMP

	MOVW XL,ZL
	MOVW ZL,YL
	ADIW ZL,_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_NAME
	MCALL RAM_COPY

	MCALL _DRV_FS_ITEM_WRITE
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MD__END

	;Не обновляем последний номер элемента если была замена на удаленный
	CPI ACCUM,0x00
	BRNE PC+0x01+_MCALL_SIZE
	MCALL _DRV_FS_LAST_ITEM_ID_UPDATE
	RJMP _DRV_FS_OP_MD__END

_DRV_FS_OP_MD__INVALID:
	LDI TEMP,DRV_RESULT_INVALID_ITEM
_DRV_FS_OP_MD__END:
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
