;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.12.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_UART
.else
.set DEF__DRV_UART = 1
	;---OPTIONS---
	.EQU	DRV_UART_OPT_BREAK					= 0x00;			;Использовать посылку BREAK(1.5 октета с низким уровнем на шине)
	;---STATES_IDS---
	.EQU	DRV_UART_ST_READY						= 0x00;
	.EQU	DRV_UART_ST_SENDING					= 0x01;
	.EQU	DRV_UART_ST_BREAK						= 0x02;
	.EQU	DRV_UART_ST_WAIT_DATA				= 0x03;
	.EQU	DRV_UART_ST_RECEIVING				= 0x04;
	.EQU	DRV_UART_ST_FAIL						= 0x05;
	;---BAUDRATE_IDS---
	.EQU	DRV_UART_BAUDRATE_4800				= 0xcf
	.EQU	DRV_UART_BAUDRATE_9600				= 0x67
	.EQU	DRV_UART_BAUDRATE_14400				= 0x44
	.EQU	DRV_UART_BAUDRATE_19200				= 0x33
	.EQU	DRV_UART_BAUDRATE_28800				= 0x22
	.EQU	DRV_UART_BAUDRATE_38400				= 0x19
	.EQU	DRV_UART_BAUDRATE_57600				= 0x10
	.EQU	DRV_UART_BAUDRATE_76800				= 0x0c
	.EQU	DRV_UART_BAUDRATE_115200			= 0x08
.endif
