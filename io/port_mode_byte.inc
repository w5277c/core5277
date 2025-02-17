;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;17.11.2024	konstantin@5277.ru		Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_MODE_BYTE
.else
.set DEF_PORT_MODE_BYTE = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_MODE_BYTE"
.endif

.include	"./io/_port_offset.inc"

;--------------------------------------------------------
PORT_MODE_BYTE:
;--------------------------------------------------------
;Устанвливаем направление пора(8 бит)
;IN: ACCUM-порт, TEMP-значение
;--------------------------------------------------------
	CPI ACCUM,0xff
	BRNE PC+0x02
	RET

	PUSH_Z
	PUSH ACCUM

	MCALL _PORT_OFFSET
	ADIW ZL,0x01
	ST Z,TEMP
.if VPORTS>0x00
	CPI ZH,high(VPORTS_ADDR)
	BRCS PC+0x04+_MCALL_SIZE
	BRNE PC+0x03
	CPI ZL,low(VPORTS_ADDR)
	BRCS PC+0x01+_MCALL_SIZE
	MCALL VPORT_SET
.endif

	POP ACCUM
	POP_Z
	RET
.endif
