;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_R1
.else
.set DEF_DRIVER_SD_GET_R1 = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/io/out_str.inc"
		.include	"./core/io/out_byte.inc"
		.include	"./core/io/out_cr.inc"
_DRV_SD_LOG_STR_R1:
		.db "<R1:",0x00
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_R1:
;--------------------------------------------------------
;Прием R1
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI TRY_CNTR,0x00
_DRV_SD_GET_R1__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R1__DONE1
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R1__LOOP
	RJMP _DRV_SD_GET_R1__FAIL
_DRV_SD_GET_R1__DONE1:
	MOV ACCUM,TEMP
	RJMP _DRV_SD_GET_R1__DONE
_DRV_SD_GET_R1__FAIL:
	LDI ACCUM,0xff
_DRV_SD_GET_R1__DONE:
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Z
		LDI_Z _DRV_SD_LOG_STR_R1|0x8000
		MCALL C5_OUT_STR
		MOV TEMP,ACCUM
		MCALL C5_OUT_BYTE
		MCALL C5_OUT_CR
		POP_Z
	.endif
.endif
	POP LOOP_CNTR
	POP TEMP
	POP TRY_CNTR
	RET
.endif
