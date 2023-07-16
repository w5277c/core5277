;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.04.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_FS_NAME_CHECK
.else
.set DEF_DRIVER_FS_NAME_CHECK = 1

.include	"./mem/ram_find.inc"
.include	"./mem/ram_find_range.inc"

;--------------------------------------------------------
_DRV_FS_NAME_CHECK:
;--------------------------------------------------------
;Проверка корректности имени
;Запрещены знаки <0x21,0x22,0x2f,0x7f, имена '.','..','/'
;IN: X-RAM адрес на имя, LOOP_CNTR-длина строки
;OUT: TEMP-результат DRV_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	CPI LOOP_CNTR,0x00
	BREQ _DRV_FS_NAME_CHECK__INVALID
	CPI LOOP_CNTR,DRV_FS_NAME_SIZE+0x01
	BRCC _DRV_FS_NAME_CHECK__INVALID

	LDI TEMP,0x22
	MCALL RAM_FIND
	BREQ _DRV_FS_NAME_CHECK__INVALID
	LDI TEMP,0x2f
	MCALL RAM_FIND
	BREQ _DRV_FS_NAME_CHECK__INVALID
	LDI TEMP,0x7f
	MCALL RAM_FIND
	BREQ _DRV_FS_NAME_CHECK__INVALID

	LDI TEMP_L,0x00
	LDI TEMP_H,0x20
	MCALL RAM_FIND_RANGE
	BREQ _DRV_FS_NAME_CHECK__INVALID

	CPI LOOP_CNTR,0x01
	BRNE _DRV_FS_NAME_CHECK__NOT1BYTESIZE
	LD TEMP,X
	CPI TEMP,'.'
	BREQ _DRV_FS_NAME_CHECK__INVALID
	CPI TEMP,'/'
	BREQ _DRV_FS_NAME_CHECK__INVALID
	RJMP _DRV_FS_NAME_CHECK__VALID
_DRV_FS_NAME_CHECK__NOT1BYTESIZE:
	CPI LOOP_CNTR,0x02
	BRNE _DRV_FS_NAME_CHECK__VALID
	LD TEMP,X
	CPI TEMP,'.'
	BRNE _DRV_FS_NAME_CHECK__VALID
	ADIW XL,0x01
	LD TEMP,X
	SBIW XL,0x01
	CPI TEMP,'.'
	BREQ _DRV_FS_NAME_CHECK__INVALID
_DRV_FS_NAME_CHECK__VALID:
	LDI TEMP,DRV_RESULT_OK
	RJMP _DRV_FS_NAME_CHECK__END
_DRV_FS_NAME_CHECK__INVALID:
	LDI TEMP,DRV_RESULT_INVALID_NAME
_DRV_FS_NAME_CHECK__END:

	POP TEMP_L
	POP TEMP_H
	RET
.endif
