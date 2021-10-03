;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.10.2021	w5277c@gmail.com			Код вынесен из esp8266.inc
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_LOGGING
.else
.set DEF_DRIVER_ESP8266_LOGGING = 1

.ifdef LOGGING_PORT
.if LOGGING_LEVEL >= LOGGING_LVL_PNC

.include	"./core/io/out_char.inc"
.include	"./core/io/out_strn.inc"
.include	"./str/strn_trim.inc"
;.include "./str/str_replace.inc"
.include	"./core/io/out_cr.inc"

;--------------------------------------------------------
_DRV_ESP8266__LOG_REQV:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI TEMP,'>'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_EH
	MCALL STRN_TRIM
	MOVW ZL,XL
	MCALL C5_OUT_STRN
	MCALL C5_OUT_CR
	POP TEMP
	POP_Z
	POP_X
	RET
;--------------------------------------------------------
_DRV_ESP8266__LOG_REQV_BYTES:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	LDI TEMP,'>'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_EH
	MCALL C5_OUT_BYTES
	MCALL C5_OUT_CR
	POP TEMP
	POP_Z
	RET
;--------------------------------------------------------
_DRV_ESP8266__LOG_RESP:
;--------------------------------------------------------
;Внутренняя процедура логирования
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI TEMP,'<'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_L
	MCALL STRN_TRIM
	MOVW ZL,XL
	MCALL C5_OUT_STRN
	MCALL C5_OUT_CR
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
.endif
.endif