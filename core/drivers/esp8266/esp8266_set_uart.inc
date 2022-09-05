;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.09.2022	konstantin@5277.ru			Код вынесен из esp8266.inc
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_SET_UART
.else
.set DEF_DRIVER_ESP8266_SET_UART = 1

.include	"./conv/num32_to_str.inc"

	.EQU	DRV_ESP8266_BAUDRATE_4800			= 0x01
	.EQU	DRV_ESP8266_BAUDRATE_9600			= 0x02
	.EQU	DRV_ESP8266_BAUDRATE_19200			= 0x04

_DRV_ESP8266_AT_SET_UART1:					.db "AT+UART_CUR=",0x00,0x00
_DRV_ESP8266_AT_SET_UART2:					.db ",8,1,0,0",0x0d,0x0a,0x00,0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_SET_UART:
;--------------------------------------------------------
;Задаем BAUDRATE для UART
;IN; Y-адрес выделенной памяти
;TEMP_EL-скорость передачи данных
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_L
	PUSH TEMP

	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_UART1|0x8000
	MCALL STR_TO_RAM
	MOV TEMP,TEMP_EL
	LDI TEMP_H,high(4800)
	LDI TEMP_L,low(4800)
	MCALL MUL16X8
	CLR TEMP_EH
	MOVW ZL,XL
	MCALL NUM32_TO_STR
	MOVW XL,ZL
	LDI_Z _DRV_ESP8266_AT_SET_UART2|0x8000
	MCALL STR_TO_RAM

	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	SBIW XL,_DRV_ESP8266_BUFFER
	SUB XL,YL
	MOV TEMP_H,XL
	MCALL _DRV_ESP8266__OP_RAW
	MCALL _DRV_ESP8266_CHECK_RESP_OK
	
	POP TEMP
	POP TEMP_L
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_X
	RET
.endif
