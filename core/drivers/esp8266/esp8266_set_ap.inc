;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2021	konstatin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_SET_AP
.else
.set DEF_DRIVER_ESP8266_SET_AP = 1

.include	"./str/str_to_ram.inc"

_DRV_ESP8266_AT_SET_AP1:					.db "AT+CWSAP_CUR=",0x22,0x00,0x00;+APN+
_DRV_ESP8266_AT_SET_AP2:					.db 0x22,",",0x22,0x00;+PASSWORD+
_DRV_ESP8266_AT_SET_AP3:					.db 0x22,",5,4",0x0d,0x0a,0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_SET_AP:
;--------------------------------------------------------
;Задаем параметры для активации точки доступа
;IN; Y-адрес выделенной памяти
;X-адрес на имя AP, Z-адрес на пароль
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z

	PUSH_Z
	PUSH_X
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_AP1|0x8000
	MCALL STR_TO_RAM
	POP_Z
	MCALL STR_TO_RAM
	LDI_Z _DRV_ESP8266_AT_SET_AP2|0x8000
	MCALL STR_TO_RAM
	POP_Z
	MCALL STR_TO_RAM
	LDI_Z _DRV_ESP8266_AT_SET_AP3|0x8000
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
