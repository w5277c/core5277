;-----------------------------------------------------------------------------------------------------------------------
;���� ���������������� ��� ��������� GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.10.2021	w5277c@gmail.com			��� ������� �� esp8266.inc
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_ESP8266_TCP_CLOSE
.else
.set DEF_DRIVER_ESP8266_TCP_CLOSE = 1

_DRV_ESP8266_AT_CLOSE_TCP_SERVER:		.db "AT+CIPSERVER=0",0x0d,0x0a,0x00,0x00

_DRV_ESP8266__OP_TCP_CLOSE:
	;TODO
	RET
.endif