;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_PRIM_HEX
.else
.set DEF_C5_LOG_PRIM_HEX = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"
.include	"./core/log/log_byte.inc"

;--------------------------------------------------------
C5_LOG_PRIM_HEX:
;--------------------------------------------------------
;Логирование примитива
;IN: Z-адрес на память с переменными, TEMP-смещение для
;переменной, ACCUM-тип примитива
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Z

	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f

	LDI TEMP,'0'
	MCALL C5_LOG_CHAR
	LDI TEMP,'x'
	MCALL C5_LOG_CHAR
_C5_LOG_PRIM_HEX__LOOP:
	LD TEMP,Z+
	MCALL C5_LOG_BYTE
	DEC LOOP_CNTR
	BRNE _C5_LOG_PRIM_HEX__LOOP

	POP_Z
	POP LOOP_CNTR
	POP TEMP
	RET
.endif
.endif
