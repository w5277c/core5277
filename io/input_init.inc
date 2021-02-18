;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_INPUT_INIT
.else
.set DEF_INPUT_INIT = 1
.ifdef INPUT_PORT

.if REPORT_INCLUDES == 0x01
.message "included INPUT_INIT"
.endif

;--------------------------------------------------------
INPUT_INIT:
;--------------------------------------------------------
;Инициализация порта ввода (soft UART RX)
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH ACCUM

	LDI ZH,high(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03+0x01))
	LDI ZL,low(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03+0x01))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ANDI ACCUM,~(EXP2(INPUT_PORT & 0x0f))
	ST Z,ACCUM

	LDI ZH,high(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03))
	LDI ZL,low(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(INPUT_PORT & 0x0f)
	ST Z,ACCUM

	POP ACCUM
	POP ZL
	POP ZH
	RET
.endif
.endif
