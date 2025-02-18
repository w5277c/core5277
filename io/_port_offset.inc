;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.11.2024	konstantin@5277.ru		Начало
;21.11.2024	konstantin@5277.ru		Сократил PORTS_TABLE, оставив только PINx(всегда DDRx=PINx+1, PORTx=PINx+2)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_OFFSET
.else
.set DEF_PORT_OFFSET = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_OFFSET"
.endif

.include	"./conv/_bitnum_to_num_table.inc"

;-----------------------------------------------------------
_PORT_OFFSET:
;-----------------------------------------------------------
;Возвращаем смещение на первый адрес регистров порта
;IN: ACCUM-сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: Z-адрес регистра PINx, ACCUM-пин в виде числа
;-----------------------------------------------------------
	PUSH TEMP
	MOV TEMP,ACCUM
	ANDI ACCUM,0x0f

	LDI_Z _BITNUM_TO_NUM_TABLE*2
	ADD ZL,ACCUM
	ADC ZH,C0x00
	LPM ACCUM,Z

	SWAP TEMP
	ANDI TEMP,0x0f

.if VPORTS>0x00
	CPI TEMP,PORTS_QUANTITY
	BRCS _PORT_OFFSET__REALPORTS
	LDI_Z VPORTS_ADDR
	SUBI TEMP,PORTS_QUANTITY
	ADD ZL,TEMP
	ADC ZH,C0x00
	LSL TEMP
	ADD ZL,TEMP
	ADC ZH,C0x00
	POP TEMP
	RET
.endif
_PORT_OFFSET__REALPORTS:
	LDI_Z PORTS_TABLE*2
	ADD ZL,TEMP
	ADC ZH,C0x00
	LPM ZL,Z
	LDI ZH,0x00
	POP TEMP
	RET
.endif
