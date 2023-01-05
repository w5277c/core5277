;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_DATARESP_TOKEN
.else
.set DEF_DRIVER_SD_GET_DATARESP_TOKEN = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/io/out_str.inc"
		.include	"./core/io/out_str_el.inc"
		.include	"./core/io/out_cr.inc"
_DRV_SD_LOG_STR_DATA_RESPONSE_TOKEN:
		.db	"Response:",0x00
_DRV_SD_LOG_STR_DATA_RESPONSE_TOKENS:
		.db	"Accepted",0x00,"CRC error",0x00,"Write error",0x00
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_DATARESP_TOKEN:
;--------------------------------------------------------
;Прием DATA RESPONSE токена
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI TRY_CNTR,0x00
_DRV_SD_GET_DATARESP_TOKEN1__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xff
	BRNE _DRV_SD_GET_DATARESP_TOKEN1__DONE
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_DATARESP_TOKEN1__LOOP
_DRV_SD_GET_DATARESP_TOKEN1__DONE:
	MOV ACCUM,TEMP
	ANDI ACCUM,0x1f

	LDI LOOP_CNTR,0x00
_DRV_SD_GET_DATARESP_TOKEN2__LOOP:
	LDI TRY_CNTR,0x00
_DRV_SD_GET_DATARESP_TOKEN2__LOOP2:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xff
	BREQ _DRV_SD_GET_DATARESP_TOKEN2__DONE
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_DATARESP_TOKEN2__LOOP2
	DEC LOOP_CNTR
	BRNE _DRV_SD_GET_DATARESP_TOKEN2__LOOP
	LDI ACCUM,0xff
_DRV_SD_GET_DATARESP_TOKEN2__DONE:

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Z
		LDI_Z _DRV_SD_LOG_STR_DATA_RESPONSE_TOKEN|0x8000
		MCALL C5_OUT_STR
		MOV TEMP,ACCUM
		LSR TEMP
		LSR TEMP
		DEC TEMP
		LDI_Z _DRV_SD_LOG_STR_DATA_RESPONSE_TOKENS|0x8000
		MCALL C5_OUT_STR_EL
		MCALL C5_OUT_CR
		POP_Z
	.endif
.endif

	POP LOOP_CNTR
	POP TEMP
	POP TRY_CNTR
	RET
.endif
