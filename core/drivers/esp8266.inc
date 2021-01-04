;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/ram/ram_realloc.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./str/str_to_ram.inc"
.include	"./str/str_find.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_ESP8266_UART_DRV_ID						= 0x00;Идентификатор драйвера UART
	.EQU	_DRV_ESP8266_BUFFER_SIZE						= 0x40;Размер буфера
	;---OFFSETS---
	.EQU	_DRV_ESP8266_BUFFER_OFFSET						= 0x01;Адрес начала буфера в выделенной памяти
	;---OPERATIONS---
	.EQU	DRV_ESP8266_OP_INIT								= 0x00;Инициализация работы
	.EQU	DRV_ESP8266_OP_RAW								= 0x01;Свободный формат данных
	.EQU	DRV_ESP8266_OP_SET_WIFI							= 0x02;Установить параметры Wi-Fi
	.EQU	DRV_ESP8266_OP_TCP_SERVER						= 0x03;Поднять TCP сервер
	.EQU	DRV_ESP8266_OP_WAIT_DATA						= 0x04;Ждать данные
	.EQU	DRV_ESP8266_OP_SEND_DATA						= 0x05;Передать данные
	.EQU	DRV_ESP8266_OP_TCP_CLOSE						= 0x06;Положить TCP соединение
	;---RESULTS---
	.EQU	DRV_ESP8266_RESULT_OK							= 0x00
	.EQU	DRV_ESP8266_RESULT_ERROR						= 0x01
	;---AT-CONSTANTS---
_DRV_ESP8266_AT_ECHO_OFF:			.db "ATE0",0x0d,0x0a,0x00
_DRV_ESP8266_AT_SET_WIFI1:			.db "AT+CWJAP_DEF=",0x22,0x00;+APN+
_DRV_ESP8266_AT_SET_WIFI2:			.db 0x22,",",0x22,0x00;+PASSWORD+
_DRV_ESP8266_AT_SET_WIFI3:			.db 0x22,0x00;+END
_DRV_ESP8266_AT_SET_TCP_SERVER1:	.db "AT+CIPSTART=",0x22,"TCP",0x22,",",0x22,0x00;+IP+
_DRV_ESP8266_AT_SET_TCP_SERVER2:	.db 0x22,",",0x00;+PORT+END
_DRV_ESP8266_AT_SEND:				.db "AT+CIPSEND=",0x00;+XXX+END
_DRV_ESP8266_AT_END:					.db 0x0d,0x0a,0x00
_DRV_ESP8266_AT_RESP_OK:			.db 0x0d,0x0a,"OK",0x0d,0x0a,0x00

	;---
	.EQU	_DRV_ESP8266_VARS_SIZE							= 0x01

;--------------------------------------------------------
DRV_ESP8266_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP - ид драйвера UART
;--------------------------------------------------------
	PUSH ACCUM

	LDI ACCUM,_DRV_ESP8266_VARS_SIZE+_DRV_ESP8266_BUFFER_SIZE
	MCALL C5_RAM_REALLOC

	STD Z+_DRV_ESP8266_UART_DRV_ID,TEMP

	POP ACCUM
	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;--------------------------------------------------------
;IN: ACCUM - тип операции
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI ACCUM,DRV_ESP8266_OP_INIT
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_INIT
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_RAW
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_RAW
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_SET_WIFI
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_SET_WIFI
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_TCP_SERVER
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_TCP_SERVER
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_WAIT_DATA
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_WAIT_DATA
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_SEND_DATA
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_SEND_DATA
	RJMP _DRV_ESP8266_PROC_END
	CPI ACCUM,DRV_ESP8266_OP_TCP_CLOSE
	BRNE PC+0x04
	MCALL _DRV_ESP8266__OP_TCP_CLOSE
	RJMP _DRV_ESP8266_PROC_END
_DRV_ESP8266_PROC_END:
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_ESP8266__OP_INIT:
;--------------------------------------------------------
;Инициализация ESP8266
;OUT:TEMP_H - RESULT
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_L

	LDI XH,0x00
	LDI XL,_DRV_ESP8266_BUFFER_OFFSET
	ADD XL,ZL
	ADC XH,ZH
	PUSH_X
	LDI YH,high(_DRV_ESP8266_AT_ECHO_OFF)|0x80
	LDI YL,low(_DRV_ESP8266_AT_ECHO_OFF)
	MCALL STR_TO_RAM
	MOV TEMP_EH,XL
	SUB TEMP_EH,ZL
	SUBI TEMP_EH,_DRV_ESP8266_BUFFER_OFFSET
	LDI TEMP_EL,(_DRV_ESP8266_BUFFER_SIZE-0x01)
	LDI TEMP_H,high(1000/2)
	LDI TEMP_L,low(1000/2)
	POP_X
	MOV YH,XH
	MOV YL,XL
	LDD TEMP,Z+_DRV_ESP8266_UART_DRV_ID
	MCALL C5_EXEC

	;Проверка ответа
	CPI TEMP_H,DRV_UART_ST_READY
	BRNE __DRV_ESP8266__OP_INIT__END
	ADD YL,TEMP_L
	LDI TEMP,0x00
	ADC YH,TEMP
	ST Y,TEMP
	LDI YH,high(_DRV_ESP8266_AT_RESP_OK)|0x80
	LDI YL,low(_DRV_ESP8266_AT_RESP_OK)
	MCALL STR_FIND
	LDI TEMP_H,DRV_UART_ST_FAIL
	CPI TEMP,0xff
	BREQ __DRV_ESP8266__OP_INIT__END
	LDI TEMP_H,DRV_UART_ST_READY

__DRV_ESP8266__OP_INIT__END:
	POP TEMP_L
	POP TEMP_EL
	POP TEMP_EH
	POP_Y
	POP_X
	RET

_DRV_ESP8266__OP_RAW:
_DRV_ESP8266__OP_SET_WIFI:
_DRV_ESP8266__OP_TCP_SERVER:
_DRV_ESP8266__OP_WAIT_DATA:
_DRV_ESP8266__OP_SEND_DATA:
_DRV_ESP8266__OP_TCP_CLOSE:
	RET
