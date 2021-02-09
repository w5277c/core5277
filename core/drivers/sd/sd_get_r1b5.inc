;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_R1B5
.else
.set DEF_DRIVER_SD_GET_R1B5 = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/log/log_str.inc"
		.include	"./core/log/log_byte.inc"
		.include	"./core/log/log_cr.inc"
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_R1b5:
;--------------------------------------------------------
;Прием R1b5
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI TRY_CNTR,0x00
_DRV_SD_GET_R1b5__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R1b5__DONE1
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R1b5__LOOP
	RJMP _DRV_SD_GET_R1b5__FAIL
_DRV_SD_GET_R1b5__DONE1:
	MOV ACCUM,TEMP


	LDI LOOP_CNTR,0x00
_DRV_SD_GET_R1b5__LOOP2:
	LDI TRY_CNTR,0x00
_DRV_SD_GET_R1b5__LOOP3:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0xff
	BREQ _DRV_SD_GET_R1b5__DONE
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R1b5__LOOP3
	DEC LOOP_CNTR
	BRNE _DRV_SD_GET_R1b5__LOOP2
	LDI ACCUM,0xff
	RJMP _DRV_SD_GET_R1b5__DONE
_DRV_SD_GET_R1b5__FAIL:
	LDI ACCUM,0xff
_DRV_SD_GET_R1b5__DONE:
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Y
		LDI_Y _DRV_SD_LOG_STR_R1|0x8000
		MCALL C5_LOG_STR
		MOV TEMP,ACCUM
		MCALL C5_LOG_BYTE
		MCALL C5_LOG_CR
		POP_Y
	.endif
.endif

	POP LOOP_CNTR
	POP TEMP
	POP TRY_CNTR
	RET
.endif
