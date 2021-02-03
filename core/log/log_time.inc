;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало (не проверено)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_TIME
.else
.set DEF_C5_LOG_TIME = 1
.ifdef LOGGING_PORT

.include	"./math/mul16x8.inc"
.include	"./math/mul32x16.inc"
.include	"./core/log/log_cr.inc"
.include	"./core/log/log_str.inc"
.include	"./core/log/log_numx16.inc"
.include	"./core/log/log_numx32.inc"

_LOG_TIME_STR_TIME:
	.db   "TIME:",0x00
_LOG_TIME_STR_OVERFLOW:
	.db   ">100days",0x00
_LOG_TIME_STR_US:
	.db   "us",0x00
_LOG_TIME_STR_MS:
	.db   "ms",0x00
;--------------------------------------------------------
C5_LOG_TIME:
;--------------------------------------------------------
;Логирование временной метки
;IN: PID,TEMP-смещение на выделенные 6 байт, хранящие
;временную метку
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP


	LDI_Y _LOG_TIME_STR_TIME|0x8000
	MCALL C5_LOG_STR

	LDI ACCUM,0x00
	LDD TEMP,Z+0x00
	CPI TEMP,0x00
	BRNE _C5_LOG_TIME__OVERFLOW
	OR ACCUM,TEMP
	LDD TEMP_EH,Z+0x01
	OR ACCUM,TEMP_EH
	LDD TEMP_EL,Z+0x02
	OR ACCUM,TEMP_EL
	LDD TEMP_H,Z+0x03
	OR ACCUM,TEMP_H
	LDD TEMP_L,Z+0x04
	CPI ACCUM,0x00
	BRNE _C5_LOG_TIME__MS
	CPI TEMP_L,0x06
	BRCC _C5_LOG_TIME__MS
_C5_LOG_TIME__US:
	LDI TEMP_H,0x00
	LDD TEMP_L,Z+0x05
	LDI TEMP,0x32
	MCALL MUL16X8
	MCALL C5_LOG_NUMX16
	LDI_Y _LOG_TIME_STR_US|0x8000
	MCALL C5_LOG_STR
	RJMP _C5_LOG_TIME__END
_C5_LOG_TIME__OVERFLOW:
	LDI_Y _LOG_TIME_STR_OVERFLOW|0x8000
	MCALL C5_LOG_STR
	RJMP _C5_LOG_TIME__END
_C5_LOG_TIME__MS:
	LSL TEMP_L
	ROL TEMP_H
	ROL TEMP_EL
	ROL TEMP_EH
	MCALL C5_LOG_NUMX32
	LDI_Y _LOG_TIME_STR_MS|0x8000
	MCALL C5_LOG_STR
_C5_LOG_TIME__END:
	MCALL C5_LOG_CR

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	POP_X
	RET
.endif
.endif
