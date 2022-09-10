;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.10.2021	w5277c@gmail.com			Код вынесен из esp8266.inc
;10.09.2022	konstantin@5277.ru		Актуализация
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_RECEIVE_DATA
.else
.set DEF_DRIVER_ESP8266_RECEIVE_DATA = 1

	.include	"./const/colon_str.inc"

_DRV_ESP8266_AT_RECV:						.db "AT+CIPRECVDATA=0,40",0x0d,0x0a,0x00
_DRV_ESP8266_AT_RESP_IPD:					.db 0x0d,0x0a,"+IPD,",0x00

;--------------------------------------------------------
_DRV_ESP8266__OP_RECEIVE_DATA:
;--------------------------------------------------------
;Принимаем данные
;IN: Y-адрес на выделенную память, TEMP_H/L-таймаут
;ожидания данных (2мс)
;OUT:TEMP_H-результат, TEMP_L-длина,Y-адрес на начало
;данных
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_EH
	PUSH TEMP_EL

	;Только читаем
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI TEMP_EH,0x00
	LDI TEMP_EL,(_DRV_ESP8266_BUFFER_SIZE-0x01)
	LDD TEMP,Z+_DRV_ESP8266_UART_DRV_ID
	MCALL C5_EXEC

	CPI TEMP_H,DRV_UART_ST_READY
	BREQ PC+0x02
	RJMP _DRV_ESP8266__OP_RECEIVE_DATA__NO_ANSWER
	CPI TEMP_L,0x00
	BRNE PC+0x02
	RJMP _DRV_ESP8266__OP_RECEIVE_DATA__NO_ANSWER
	
	PUSH_X
	ADD XL,TEMP_L
	ADC XH,C0x00
	ST X,TEMP
	POP_X
	LDI_Y _DRV_ESP8266_AT_RESP_IPD|0x8000
	MOV TEMP_EL,TEMP_L
	MCALL STR_FIND
	CPI TEMP_H,0xff
	BREQ _DRV_ESP8266__OP_RECEIVE_DATA__ERROR
	SUB TEMP_EL,TEMP_L
	ADD XL,TEMP_L
	ADC XH,C0x00

	LDI_Y CONST_COLON_STR|0x8000
	MCALL STR_FIND
	CPI TEMP_H,0xff
	BREQ _DRV_ESP8266__OP_RECEIVE_DATA__ERROR
	SUB TEMP_EL,TEMP_L
	ADD XL,TEMP_L
	CLR TEMP
	ADC XH,TEMP
	MOV TEMP_L,TEMP_EL

	MOV YH,XH
	MOV YL,XL
	LDI TEMP_H,DRV_RESULT_OK
	RJMP _DRV_ESP8266__OP_RECEIVE_DATA__END
_DRV_ESP8266__OP_RECEIVE_DATA__NO_ANSWER:
	LDI TEMP_H,DRV_RESULT_ABSENT
	RJMP _DRV_ESP8266__OP_RECEIVE_DATA__END
_DRV_ESP8266__OP_RECEIVE_DATA__ERROR:
	LDI TEMP_H,DRV_RESULT_ERROR
	RJMP _DRV_ESP8266__OP_RECEIVE_DATA__END
_DRV_ESP8266__OP_RECEIVE_DATA__END:
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET

.endif