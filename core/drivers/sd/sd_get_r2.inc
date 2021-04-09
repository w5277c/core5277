;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DRIVER_SD_GET_R2
.else
.set DEF_DRIVER_SD_GET_R2 = 1

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		.include	"./core/log/log_str.inc"
		.include	"./core/log/log_word.inc"
		.include	"./core/log/log_cr.inc"
_DRV_SD_LOG_STR_R2:
		.db "<R2:",0x00
	.endif
.endif

;--------------------------------------------------------
_DRV_SD_GET_R2:
;--------------------------------------------------------
;Прием R2
;IN: X-адрес на буфер
;OUT: TEMP_H,TEMP_L
;--------------------------------------------------------
	PUSH TEMP

	MCALL _DRV_SD_BYTE_RECV
	MOV TEMP_H,TEMP
	MCALL _DRV_SD_BYTE_RECV
	MOV TEMP_L,TEMP
.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_PNC
		PUSH_Y
		LDI_Y _DRV_SD_LOG_STR_R2|0x8000
		MCALL C5_LOG_STR
		MCALL C5_LOG_WORD
		MCALL C5_LOG_CR
		POP_Y
	.endif
.endif
	POP TEMP
	RET
.endif
