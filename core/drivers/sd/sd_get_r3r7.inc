;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_R3R7
.else
.set DEF_DRIVER_SD_GET_R3R7 = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/log/log_str.inc"
		.include	"./core/log/log_byte.inc"
		.include	"./core/log/log_bytes.inc"
		.include	"./core/log/log_cr.inc"
_DRV_SD_LOG_STR_R3R7:
		.db "<R3/R7:",0x00
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_R3R7:
;--------------------------------------------------------
;Прием R7
;IN: X-адрес на буфер
;OUT: ACCUM
;--------------------------------------------------------
	PUSH TRY_CNTR
	PUSH TEMP

	LDI TRY_CNTR,0x10
_DRV_SD_GET_R7__LOOP:
	MCALL _DRV_SD_BYTE_RECV
	SBRS TEMP,0x07
	RJMP _DRV_SD_GET_R7__GOT_R1
	DEC TRY_CNTR
	BRNE _DRV_SD_GET_R7__LOOP
_DRV_SD_GET_R7__GOT_R1:
	MOV ACCUM,TEMP

	LDI TEMP,0x04
	MCALL _DRV_SD_BYTES_RECV
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Z
		LDI_Z _DRV_SD_LOG_STR_R3R7|0x8000
		MCALL C5_LOG_STR
		MOV TEMP,ACCUM
		MCALL C5_LOG_BYTE
		MOVW ZL,XL
		LDI TEMP,0x04
		MCALL C5_LOG_BYTES
		MCALL C5_LOG_CR
		POP_Z
	.endif
.endif
	POP TEMP
	POP TRY_CNTR
	RET
.endif
