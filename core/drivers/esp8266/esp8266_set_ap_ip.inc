;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2021	konstatin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_SET_AP_IP
.else
.set DEF_DRIVER_ESP8266_SET_AP_IP = 1

.include	"./str/str_to_ram.inc"

_DRV_ESP8266_AT_SET_AP_IP1:					.db "AT+CIPAP_CUR=",0x22,0x00,0x00;+IP+
_DRV_ESP8266_AT_SET_AP_IP2:					.db 0x22,0x0d,0x0a,0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_SET_AP_IP:
;--------------------------------------------------------
;Задаем IP для режима точки доступа
;IN; Y-адрес выделенной памяти
;TEMP_EH/EL/H/L-IP
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP

	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_AP_IP1|0x8000
	MCALL STR_TO_RAM
	LDI TEMP,'.'
	PUSH TEMP_L
	MOVW ZL,XL
	MOV TEMP_L,TEMP_EH
	MCALL NUM_TO_STR
	ST Z+,TEMP
	MOV TEMP_L,TEMP_EL
	MCALL NUM_TO_STR
	ST Z+,TEMP
	MOV TEMP_L,TEMP_H
	MCALL NUM_TO_STR
	ST Z+,TEMP
	POP TEMP_L
	MCALL NUM_TO_STR
	MOVW XL,ZL	
	LDI_Z _DRV_ESP8266_AT_SET_AP_IP2|0x8000
	MCALL STR_TO_RAM
	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	SBIW XL,_DRV_ESP8266_BUFFER
	SUB XL,YL
	MOV TEMP_H,XL
	MCALL _DRV_ESP8266__OP_RAW
	MCALL _DRV_ESP8266_CHECK_RESP_OK

	POP TEMP
	POP_Z
	POP_X
	RET
.endif
