;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.01.2023	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_FS_DELETE
.else
.set DEF__DRV_FS_DELETE = 1

;--------------------------------------------------------
_DRV_FS_DELETE:
;--------------------------------------------------------
;Создаем директорию
;IN: Y-адрес на переменные,TEMP_H/L-элемент
;OUT: TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL

	;MOV TEMP_EL,TEMP_H
	;OR TEMP_EL,TEMP_L
	;CPI TEMP_EL,0x00
	;BRNE PC+0x03
	;LDI TEMP,DRV_RESULT_ERROR


	CPI TEMP_H,0x00
	BRNE PC+0x05
	CPI TEMP_L,0x00
	BRNE PC+0x03
	LDI TEMP,DRV_RESULT_ERROR
	RJMP _DRV_FS_DELETE__END

	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_DELETE__END
	LDD TEMP_EH,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	MOV TEMP_EL,TEMP_H
	ANDI TEMP_EL,0x0f
	;Нет такого файла
	LDI TEMP,DRV_RESULT_ABSENT
	CPI TEMP_EL,DRV_FS_TYPE_FREE
	BREQ _DRV_FS_DELETE__END
	CPI TEMP_EL,DRV_FS_TYPE_RM_DIR
	BREQ _DRV_FS_DELETE__END
	CPI TEMP_EL,DRV_FS_TYPE_RM_FILE
	BREQ _DRV_FS_DELETE__END
	;READ ONLY
	LDI TEMP,DRV_RESULT_READ_ONLY
	SBRC TEMP_EH,DRV_FS_OPT_RO
	RJMP _DRV_FS_DELETE__END
	;DIR
	ORI TEMP_EH,DRV_FS_TYPE_RM_FILE
	CPI TEMP_EL,DRV_FS_TYPE_DIR
	BRNE _DRV_FS_DELETE__DIRCHECK_SKIP
	ORI TEMP_EH,DRV_FS_TYPE_RM_DIR
	LDI_T16 0x0001
_DRV_FS_DELETE__LOOP:
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_DELETE__END
	LDD TEMP,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	ANDI TEMP,0x0f
	CPI TEMP,DRV_FS_TYPE_RM_DIR
	BREQ _DRV_FS_DELETE__GOT_REMOVED
	CPI TEMP,DRV_FS_TYPE_RM_FILE
	BREQ _DRV_FS_DELETE__GOT_REMOVED
_DRV_FS_DELETE__NEXT_ITEM:
	LDD TEMP,Y+_DRV_FS_ITEMS+0x00
	CP TEMP,TEMP_H
	BRNE PC+0x06
	LDD TEMP,Y+_DRV_FS_ITEMS+0x01
	CP TEMP,TEMP_L
	BRNE PC+0x03
	LDI TEMP,DRV_RESULT_MAX_REACHED
	RJMP _DRV_FS_DELETE__END
	LDI TEMP,0x01
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	RJMP _DRV_FS_DELETE__LOOP
_DRV_FS_DELETE__GOT_REMOVED:
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_DELETE__END

_DRV_FS_DELETE__DIRCHECK_SKIP:
	STD Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT,TEMP_EH
	MCALL _DRV_FS_ITEM_WRITE

_DRV_FS_DELETE__END:

	POP TEMP_EL
	POP TEMP_EH
	RET
.endif
