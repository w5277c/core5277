;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_CHAR
.else
.set DEF_C5_LOG_CHAR = 1
.ifdef LOGGING_PORT

;--------------------------------------------------------
C5_LOG_CHAR:
;--------------------------------------------------------
;Логирование символа
;IN: TEMP - байт
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH ACCUM
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH FLAGS

	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,((LOGGING_PORT>>4)*0x03)
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH

	LD ACCUM,Z															;2t
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))														;1t
	ST Z,ACCUM															;2t
	LDI ACCUM,8
	DEC ACCUM
	BRNE PC-0x01

	LDS FLAGS,SREG
	CLI
	LD ACCUM,Z															;2t
	ANDI ACCUM,~(EXP2 (LOGGING_PORT & 0x0f))														;1t
	ST Z,ACCUM															;2t
	LDI ACCUM,8
	DEC ACCUM
	BRNE PC-0x01

	;DATA BITS
	LDI LOOP_CNTR,0x08
_C5_LOG_PROC__BITES_LOOP:

	LD ACCUM,Z															;2t
	SBRC TEMP,0x00
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))														;1t
	SBRS TEMP,0x00
	ANDI ACCUM,~(EXP2 (LOGGING_PORT & 0x0f))													;1t
	LSR TEMP
	ST Z,ACCUM															;2t

	NOP
	NOP
	LDI ACCUM,7
	DEC ACCUM
	BRNE PC-0x01

	DEC LOOP_CNTR
	BRNE _C5_LOG_PROC__BITES_LOOP
	;STOP
	LD ACCUM,Z															;2t
	ORI ACCUM,(EXP2 (LOGGING_PORT & 0x0f))														;1t
	ST Z,ACCUM															;2t

	STS SREG,FLAGS
	POP FLAGS
	POP TEMP
	POP LOOP_CNTR
	POP ACCUM
	POP ZL
	POP ZH
	RET
.endif
.endif
