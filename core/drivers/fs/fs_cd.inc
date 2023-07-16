;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;22.10.2022	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_CD
.else
.set DEF_DRIVER_FS_CD = 1

.include	"./core/drivers/fs/_fs_name_check.inc"

;--------------------------------------------------------
_DRV_FS_OP_CD:
;--------------------------------------------------------
;Смена директории
;IN: Y-адрес на переменные,TEMP_H/L-ид элемента
;OUT: TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH ACCUM

	;Проверка элемента
	MCALL _DRV_FS_ITEM_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_OP_CD__END
	LDI TEMP,DRV_RESULT_INVALID_ITEM
	LDD ACCUM,Y+_DRV_FS_ITEM_BUFFER+DRV_FS_ITEM_OPT
	ANDI ACCUM,0x0f
	CPI ACCUM,DRV_FS_TYPE_DIR
	BRNE _DRV_FS_OP_CD__END

	STD Y+_DRV_FS_CUR_DIR+0x00,TEMP_H
	STD Y+_DRV_FS_CUR_DIR+0x01,TEMP_L

	LDI TEMP,DRV_RESULT_OK
_DRV_FS_OP_CD__END:
	POP ACCUM
	RET
.endif
