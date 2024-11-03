;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;03.11.2024	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_OFFSET
.else
.set DEF_PORT_OFFSET = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_OFFSET"
.endif

.include	"./conv/_bitnum_to_num_table.inc"

;--------------------------------------------------------
_PORT_OFFSET:
;--------------------------------------------------------
;Возвращаем смещение на первый адрес регистров порта
;IN: ACCUM-сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: Z-адрес регистра PORTx, ACCUM-пин в виде числа
;MODIFY: TEMP
;--------------------------------------------------------
.if VPORTS>0x00
	CPI ACCUM,(PORTS_QUANTITY<<0x04)-0x01
	BRCS PC+0x01+_MCALL_SIZE
	MJMP VPORT_OFFSETS
.endif

	MOV TEMP,ACCUM
	ANDI ACCUM,0x0f

	LDI_Z _BITNUM_TO_NUM_TABLE*2
	ADD ZL,ACCUM
	ADC ZH,C0x00
	LPM ACCUM,Z

	SWAP TEMP
	ANDI TEMP,0x0f
	LDI_Z (PORTS_TABLE*2)
	ADD ZL,TEMP
	ADD ZL,TEMP
	ADD ZL,TEMP

	RET
.endif
