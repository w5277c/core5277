;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021  w5277c@gmail.com			Начало
;16.01.2021  w5277c@gmail.com			Исправлена часть ошибок, добавлено логирование
;17.02.2021  w5277c@gmail.com			Частично реализована SEND_DATA и исправлены ошибки
;-----------------------------------------------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_ESP8266
.else
.set DEF__C5_DRV_ESP8266 = 1
.message "Included driver ESP8266 v0.2"

.include	"./core/ram/ram_realloc.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./str/str_to_ram.inc"
.include	"./str/str_find.inc"
.include	"./conv/num16_to_str.inc"
.include	"./core/time32_mark.inc"
.include	"./core/time32_delta.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_ESP8266_BUFFER_SIZE						= 0xc0;Размер буфера
	;---OFFSETS---
	.EQU	_DRV_ESP8266_UART_DRV_ID						= 0x00;1Б-Идентификатор драйвера UART
	.EQU	_DRV_ESP8266_TIME_START							= 0x01;4Б-Переменная INT
	.EQU	_DRV_ESP8266_TIME_DELTA							= 0x05;4Б-Переменная INT
	.EQU	_DRV_ESP8266_BUFFER								= 0x09;192Б-буфер для приема
	.EQU	_DRV_ESP8266_RAM_SIZE							= 0xc9

	;---OPERATIONS---
	.EQU	DRV_ESP8266_OP_INIT								= 0x00;Инициализация работы
	.EQU	DRV_ESP8266_OP_SET_UART							= 0x01;Инициализация UART
	.EQU	DRV_ESP8266_OP_RAW								= 0x02;Свободный формат данных
	.EQU	DRV_ESP8266_OP_SET_WIFI							= 0x03;Установить параметры Wi-Fi
	.EQU	DRV_ESP8266_OP_TCP_SERVER						= 0x04;Поднять TCP сервер
	.EQU	DRV_ESP8266_OP_TCP_CLIENT						= 0x05;Установить TCP подключение
	.EQU	DRV_ESP8266_OP_RECEIVE_DATA					= 0x06;Принять данные
	.EQU	DRV_ESP8266_OP_SEND_DATA						= 0x07;Передать данные
	.EQU	DRV_ESP8266_OP_TCP_CLOSE						= 0x08;Положить TCP соединение

	;---AT-CONSTANTS---
_DRV_ESP8266_AT_ECHO_OFF:					.db "ATE0",0x0d,0x0a,0x00,0x00
_DRV_ESP8266_AT_SET_MODE:					.db "AT+CWMODE_CUR=",0x00,0x00
;_DRV_ESP8266_AT_RES_MULTIPLE_CONNS:		.db "AT+CIPMUX=0",0x0d,0x0a,0x00
;_DRV_ESP8266_AT_SET_TCP_CLIENT1:			.db "AT+CIPSTART=0,",0x22,"TCP",0x22,",",0x22,0x00;+IP+
;_DRV_ESP8266_AT_SET_TCP_CLIENT2:			.db 0x22,",",0x00,0x00;+PORT+
;_DRV_ESP8266_AT_SET_TCP_CLIENT3:			.db ",5",0x0d,0x0a,0x00,0x00
;_DRV_ESP8266_AT_CLOSE_TCP:					.db "AT+CIPCLOSE=0",0x0d,0x0a,0x00

_DRV_ESP8266_AT_NEW_LINE:					.db 0x0d,0x0a,0x00,0x00
_DRV_ESP8266_AT_RESP_OK:					.db 0x0d,0x0a,"OK",0x0d,0x0a,0x00,0x00
_DRV_ESP8266_AT_RESP_ERROR:				.db 0x0d,0x0a,"ERROR",0x0d,0x0a,0x00
_DRV_ESP8266_AT_RESP_FAIL:					.db 0x0d,0x0a,"FAIL",0x0d,0x0a,0x00,0x00
_DRV_ESP8266_AT_CONST_COLON:				.db ":",0x00
_DRV_ESP8266_AT_RESP_READY:				.db 0x0d,0x0a,">",0x00
;;;;_DRV_ESP8266_AT_WIFI_CONNECTED:			.db 0x0d,0x0a,"WIFI CONNECTED",0x0d,0x0a,0x00

	;---

;--------------------------------------------------------
DRV_ESP8266_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP-ид драйвера UART
;--------------------------------------------------------
	LDI ACCUM,_DRV_ESP8266_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_ESP8266_UART_DRV_ID,TEMP

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;--------------------------------------------------------
;IN: ACCUM-тип операции
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI ACCUM,DRV_ESP8266_OP_INIT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_INIT
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_RAW
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_RAW
	RJMP _DRV_ESP8266_PROC_END
.ifdef _DRV_ESP8266__OP_SET_UART
	CPI ACCUM,DRV_ESP8266_OP_SET_UART
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_SET_UART
	RJMP _DRV_ESP8266_PROC_END
.endif
.ifdef _DRV_ESP8266__OP_SET_WIFI
	CPI ACCUM,DRV_ESP8266_OP_SET_WIFI
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_SET_WIFI
	RJMP _DRV_ESP8266_PROC_END
.endif
.ifdef _DRV_ESP8266__OP_TCP_SERVER
	CPI ACCUM,DRV_ESP8266_OP_TCP_SERVER
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_TCP_SERVER
	RJMP _DRV_ESP8266_PROC_END
.endif
.ifdef _DRV_ESP8266__OP_RECEIVE_DATA
	CPI ACCUM,DRV_ESP8266_OP_RECEIVE_DATA
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_RECEIVE_DATA
	RJMP _DRV_ESP8266_PROC_END
.endif
.ifdef _DRV_ESP8266__OP_SEND_DATA
	CPI ACCUM,DRV_ESP8266_OP_SEND_DATA
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_SEND_DATA
	RJMP _DRV_ESP8266_PROC_END
.endif
.ifdef _DRV_ESP8266__OP_TCP_CLOSE
	CPI ACCUM,DRV_ESP8266_OP_TCP_CLOSE
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_ESP8266__OP_TCP_CLOSE
	RJMP _DRV_ESP8266_PROC_END
.endif
_DRV_ESP8266_PROC_END:

	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_ESP8266__OP_RAW:
;--------------------------------------------------------
;Выполнение AT команды
;IN: Z-адрес содержащий команду, TEMP_H-длина команды
;Y-адрес на выделенную память
;OUT:TEMP_H-результат, TEMP_L-длина
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH_X

	LDD TEMP,Y+_DRV_ESP8266_UART_DRV_ID
	MOV TEMP_EH,TEMP_H
	LDI TEMP_EL,(_DRV_ESP8266_BUFFER_SIZE-0x01)
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
.ifdef _DRV_ESP8266__LOG_REQV
	MCALL _DRV_ESP8266__LOG_REQV
.endif
	LDI TEMP_H,high(1000/2)
	LDI TEMP_L,low(1000/2)
	MCALL C5_EXEC
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE _DRV_ESP8266__OP_RAW__ERROR
	CPI TEMP_L,0x00
	BREQ _DRV_ESP8266__OP_RAW__ERROR
.ifdef _DRV_ESP8266__LOG_RESP
	MCALL _DRV_ESP8266__LOG_RESP
.endif
	LDI TEMP_H,DRV_RESULT_OK
	RJMP _DRV_ESP8266__OP_RAW__END
_DRV_ESP8266__OP_RAW__ERROR:
	LDI TEMP_H,DRV_RESULT_ERROR
_DRV_ESP8266__OP_RAW__END:

	POP_X
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	RET

;--------------------------------------------------------
_DRV_ESP8266_CHECK_RESP_OK:
;--------------------------------------------------------
;Проверяем на ответ содержащий 'OK'
;IN: Y-адрес на выделенную память
;TEMP_H-код результата выполнения команды
;TEMP_L-длина данных
;OUT:TEMP_H-результат
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_L

	CPI TEMP_H,DRV_RESULT_OK
	BRNE __DRV_ESP8266_CHECK_RESP_OK__FAIL

	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	ADD XL,TEMP_L
	ADC XH,C0x00
	ST X,C0x00

	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_RESP_OK|0x8000
	MCALL STR_FIND
	CPI TEMP_H,0xff
	BREQ __DRV_ESP8266_CHECK_RESP_OK__FAIL

	LDI TEMP_H,DRV_RESULT_OK
	RJMP __DRV_ESP8266_CHECK_RESP_OK__END
__DRV_ESP8266_CHECK_RESP_OK__FAIL:

	LDI TEMP_H,DRV_RESULT_ERROR
__DRV_ESP8266_CHECK_RESP_OK__END:

	POP TEMP_L
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_ESP8266__OP_INIT:
;--------------------------------------------------------
;Инициализация ESP8266
;IN: Y-адрес на выделенную память, TEMP_L-CWMODE параметр
;OUT:TEMP_H-RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_L

	;ECHO OFF
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_ECHO_OFF|0x8000
	MCALL STR_TO_RAM
	SBIW XL,_DRV_ESP8266_BUFFER
	SUB XL,YL
	MOV TEMP_H,XL
	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	MCALL _DRV_ESP8266__OP_RAW
	MCALL _DRV_ESP8266_CHECK_RESP_OK
	CPI TEMP_H,DRV_RESULT_OK
	BRNE __DRV_ESP8266__OP_INIT__END

	;Station mode
	MOVW XL,YL
	ADIW XL,_DRV_ESP8266_BUFFER
	LDI_Z _DRV_ESP8266_AT_SET_MODE|0x8000
	MCALL STR_TO_RAM
	POP TEMP_L
	PUSH TEMP_L
	SUBI TEMP_L,(0x100-0x30)
	ST X+,TEMP_L
	LDI_Z _DRV_ESP8266_AT_NEW_LINE|0x8000
	MCALL STR_TO_RAM
	SBIW XL,_DRV_ESP8266_BUFFER
	SUB XL,YL
	MOV TEMP_H,XL
	MOVW ZL,YL
	ADIW ZL,_DRV_ESP8266_BUFFER
	MCALL _DRV_ESP8266__OP_RAW
	MCALL _DRV_ESP8266_CHECK_RESP_OK

__DRV_ESP8266__OP_INIT__END:
	POP TEMP_L
	POP_Z
	POP_X
	RET

.endif
