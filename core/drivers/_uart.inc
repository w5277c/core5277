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
	.EQU	DRV_UART_OPT_MAGIC					= 0x01;			;Анализировать при чтении магические первые два байта(вызывать процедуру)
	;---STATES_IDS---
	.EQU	DRV_UART_ST_READY						= 0x00;
	.EQU	DRV_UART_ST_SENDING					= 0x01;
	.EQU	DRV_UART_ST_BREAK						= 0x02;
	.EQU	DRV_UART_ST_WAIT_DATA				= 0x03;
	.EQU	DRV_UART_ST_RECEIVING				= 0x04;
	.EQU	DRV_UART_ST_FAIL						= 0x05;
	.EQU	DRV_UART_ST_OVERFLOW					= 0x06;
	;---UNIVERSAL-OFFSETS---
	.EQU	DRV_UART_RX_PORT						= 0x00			;1B - Порт для приема данных
	.EQU	DRV_UART_TX_PORT						= 0x01			;1B - Порт для передачи данных
	.EQU	DRV_UART_DR_PORT						= 0x02			;1B - Порт для задания направления прием/передача
.endif
