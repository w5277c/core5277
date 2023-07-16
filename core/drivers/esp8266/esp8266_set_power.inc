;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.09.2021	konstatin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_SET_POWER
.else
.set DEF_DRIVER_ESP8266_SET_POWER = 1

.include	"./str/str_to_ram.inc"
.include	"./conv/num_to_str.inc"

_DRV_ESP8266_AT_SET_POWER1:					.db "AT+RFPOWER=",0x00;+POWER_VALUE+
_DRV_ESP8266_AT_SET_POWER2:					.db 0x0d,0x0a,0x00,0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_SET_POWER:
;--------------------------------------------------------
;Задаем параметры для активации точки доступа
;IN; Y-адрес выделенной памяти
;TEMP_L-TX power(0-82)
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z

	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_POWER1|0x8000
	MCALL STR_TO_RAM
	MOVW ZL,XL
	MCALL NUM_TO_STR
	MOVW XL,ZL
	LDI_Z _DRV_ESP8266_AT_SET_POWER2|0x8000
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
