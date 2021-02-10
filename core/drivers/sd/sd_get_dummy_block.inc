;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_DUMMY_BLOCK
.else
.set DEF_DRIVER_SD_GET_DUMMY_BLOCK = 1

.include	"./core/time32_mark.inc"
.include	"./core/time32_delta.inc"
.endif

;--------------------------------------------------------
_DRV_SD_GET_DUMMY_BLOCK:
;--------------------------------------------------------
;Прием блока данных
;OUT: ACCUM-0x00 успешное выполнение
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH_Y

	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_MARK

	LDI YH,high(512)
	LDI YL,low(512)

	LDI TRY_CNTR,0x00
_DRV_SD_GET_DUMMY_BLOCK__BEGIN_LOOP:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xfe
	BREQ _DRV_SD_GET_DUMMY_BLOCK__GOT_ANSWER
_DRV_SD_GET_DUMMY_BLOCK__NO_ERROR:
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_DUMMY_BLOCK__BEGIN_LOOP
	RJMP _DRV_SD_GET_DUMMY_BLOCK__FAIL

_DRV_SD_GET_DUMMY_BLOCK__GOT_ANSWER:
	LDI TEMP,_DRV_SD_VAR_TIME
	MCALL C5_TIME32_DELTA

_DRV_SD_GET_DUMMY_BLOCK__DATA:
	MCALL _DRV_SD_BYTE_RECV
	SBIW YL,0x01
	BRNE _DRV_SD_GET_DUMMY_BLOCK__DATA
	;Получаем CRC
	MCALL _DRV_SD_BYTE_RECV
	MCALL _DRV_SD_BYTE_RECV

	LDI ACCUM,0x00
	RJMP _DRV_SD_GET_DUMMY_BLOCK__END
_DRV_SD_GET_DUMMY_BLOCK__FAIL:
	LDI ACCUM,0xff
_DRV_SD_GET_DUMMY_BLOCK__END:

	POP_Y
	POP TEMP
	POP TRY_CNTR
	RET
.endif
