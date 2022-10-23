;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.10.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_GET_REMOVED_ITEM
.else
.set DEF_DRIVER_FS_GET_REMOVED_ITEM = 1

;--------------------------------------------------------
_DRV_FS_GET_REMOVED_ITEM:
;--------------------------------------------------------
;Поиск удаленного элемента(сначала файл, затем
;директория)
;IN: Y-адрес на переменные, TEMP_H/L-номер начального
;элемента.
;OUT: TEMP_H/L-номер удаленного элемента,
;TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP_EH,0x00
	LDI TEMP_EL,0x00
_DRV_FS_GET_REMOVED_ITEM__LOOP:
	;Проверка родителя
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_MKDIR__END
	LDD TEMP,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	ANDI TEMP,0x0f
	CPI TEMP,DRV_FS_TYPE_RM_FILE
	BREQ _DRV_FS_GET_REMOVED_ITEM__SUCCESS
	CPI TEMP,DRV_FS_TYPE_RM_DIR
	BRNE _DRV_FS_GET_REMOVED_ITEM___NEXT
	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L
_DRV_FS_GET_REMOVED_ITEM___NEXT:
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	BRCC _DRV_FS_GET_REMOVED_ITEM__LOOP
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	OR TEMP_EH,TEMP_EL
	BRNE _DRV_FS_GET_REMOVED_ITEM__SUCCESS
	LDI TEMP,DRV_RESULT_ABSENT
	RJMP _DRV_FS_GET_REMOVED_ITEM__END
_DRV_FS_GET_REMOVED_ITEM__SUCCESS:
	LDI TEMP,DRV_RESULT_OK
_DRV_FS_GET_REMOVED_ITEM__END:

	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	RET
.endif
