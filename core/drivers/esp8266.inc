;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_ESP8266_UART_DRV_ID						= 0x00;Идентификатор драйвера UART
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
_DRV_ESP8266_AT_ECHO_OFF:			.db "ATE0",0x00;+END
_DRV_ESP8266_AT_SET_WIFI1:			.db "AT+CWJAP_DEF=",0x22,0x00;+APN+
_DRV_ESP8266_AT_SET_WIFI2:			.db 0x22,",",0x22,0x00;+PASSWORD+
_DRV_ESP8266_AT_SET_WIFI3:			.db 0x22,0x00;+END
_DRV_ESP8266_AT_SET_TCP_SERVER1:	.db "AT+CIPSTART=",0x22,"TCP",0x22,",",0x22,0x00;+IP+
_DRV_ESP8266_AT_SET_TCP_SERVER2:	.db 0x22,",",0x00;+PORT+END
_DRV_ESP8266_AT_SEND:				.db "AT+CIPSEND=",0x00;+XXX+END
_DRV_ESP8266_AT_END:					.db 0x0d,0x0a,0x00

	;---
	.EQU	_DRV_ESP8266_VARS_SIZE							= 0x01

;--------------------------------------------------------
DRV_ESP8266_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - размер буфера для AT комманд
;--------------------------------------------------------
	PUSH TEMP_L
	MCALL C5_RAM_REALLOC

	LDI TEMP_L,_DRV_ESP8266_VARS_SIZE
	ADD TEMP,TEMP_L
	STD Z+_DRV_ESP8266_UART_DRV_ID,TEMP

	POP TEMP_L
	MCALL C5_READY

;--------------------------------------------------------
;Основной код, коммуникация
;--------------------------------------------------------
	RET
