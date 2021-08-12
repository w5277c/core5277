;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_CHAR
.else
.set DEF_C5_OUT_CHAR = 1
.ifdef LOGGING_PORT

;--------------------------------------------------------
C5_OUT_CHAR:
;--------------------------------------------------------
;Логирование символа
;IN: TEMP-байт
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH FLAGS

	LDI_Z (PORTS_TABLE*2+((LOGGING_PORT>>4)*0x03))
	LPM ZL,Z
	CLR ZH

	LD ACCUM,Z
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))
	ST Z,ACCUM
	LDI ACCUM,0x14;0x08
	DEC ACCUM
	BRNE PC-0x01

	LDS FLAGS,SREG
	CLI
	LD ACCUM,Z
	ANDI ACCUM,~(EXP2 (LOGGING_PORT & 0x0f))
	ST Z,ACCUM
	LDI ACCUM,0x14;0x08
	DEC ACCUM
	BRNE PC-0x01

	;DATA BITS
	LDI LOOP_CNTR,0x08
_C5_OUT_PROC__BITES_LOOP:

	LD ACCUM,Z
	SBRC TEMP,0x00
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))
	SBRS TEMP,0x00
	ANDI ACCUM,~(EXP2 (LOGGING_PORT & 0x0f))
	LSR TEMP
	ST Z,ACCUM

	NOP
	LDI ACCUM,0x13;0x07
	DEC ACCUM
	BRNE PC-0x01
	DEC LOOP_CNTR
	BRNE _C5_OUT_PROC__BITES_LOOP
	;STOP
	LD ACCUM,Z
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))
	ST Z,ACCUM

	STS SREG,FLAGS
	POP FLAGS
	POP TEMP
	POP LOOP_CNTR
	POP ACCUM
	POP_Z
	RET
.endif
.endif
