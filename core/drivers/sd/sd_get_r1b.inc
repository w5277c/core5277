;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;09.04.2021  w5277c@gmail.com			Что это вообще?
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_R1B
.else
.set DEF_DRIVER_SD_GET_R1B = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/log/log_str.inc"
		.include	"./core/log/log_byte.inc"
		.include	"./core/log/log_cr.inc"
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_R1b:
;--------------------------------------------------------
;Прием R1b
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Y

	LDI TRY_CNTR,0x00
_DRV_SD_GET_R1b__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R1b__DONE1
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R1b__LOOP
	RJMP _DRV_SD_GET_R1b__FAIL
_DRV_SD_GET_R1b__DONE1:

	LDI_Y 0xffff
_DRV_SD_GET_R1b__LOOP2:
	MCALL _DRV_SD_BYTE_RECV
	CPI TEMP,0x00
	BREQ PC+0x03
	LDI ACCUM,0x00
	RJMP _DRV_SD_GET_R1b__DONE
	SBIW YL,0x01
	BRNE _DRV_SD_GET_R1b__LOOP2
_DRV_SD_GET_R1b__FAIL:
	LDI ACCUM,0xff
_DRV_SD_GET_R1b__DONE:
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

	POP_Y
	POP TEMP
	RET
.endif
