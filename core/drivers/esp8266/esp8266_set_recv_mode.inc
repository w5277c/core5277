;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2021	konstatin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_SET_RECV_MODE
.else
.set DEF_DRIVER_ESP8266_RECV_MODE = 1

.include	"./const/new_line_str.inc"
.include	"./str/str_to_ram.inc"

_DRV_ESP8266_AT_SET_RECV_MODE:					.db "AT+CIPRECVMODE=",0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_SET_RECV_MODE:
;--------------------------------------------------------
;Задаем режим приема
;IN; Y-адрес выделенной памяти,
;TEMP_L-0(active)/1(passive)
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z

	PUSH_Z
	PUSH_X
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_RECV_MODE|0x8000
	MCALL STR_TO_RAM
	MOVW ZL,XL
	MCALL NUM_TO_STR
	MOVW XL,ZL
	LDI_Z CONST_NEW_LINE|0x8000
	MCALL STR_TO_RAM
	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	SBIW XL,_DRV_ESP8266_BUFFER
	SUB XL,YL
	MOV TEMP_H,XL
	MCALL _DRV_ESP8266__OP_RAW
	MCALL _DRV_ESP8266_CHECK_RESP_OK

	POP_Z
	POP_X
	RET
.endif
