;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_SDNF
.else
.set DEF_CORE5277_LOG_SDNF = 1
.ifdef LOGGING_PORT

.include	"./inc/conv/byte_to_str.inc"
.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_null.inc"

;--------------------------------------------------------
CORE5277_LOG_SDNF:
;--------------------------------------------------------
;Логирование SDNF числа
;IN: TEMP_H/TEMP_L - SDNF число
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	CPI TEMP_L,0xff
	BREQ _CORE5277_LOG_SDNF__NULL_VALUE

	SBRS TEMP_L,0x07
	RJMP _CORE5277_LOG_SDNF__POSITIVE_VALUE
	LDI TEMP,'-'
	MCALL CORE5277_LOG_CHAR
	ANDI TEMP_L,0x7f
_CORE5277_LOG_SDNF__POSITIVE_VALUE:

	PUSH TEMP_L
	MOV TEMP,TEMP_H
	MCALL CORE5277_BYTE_TO_STR
	PUSH TEMP
	CPI TEMP_H,'0'
	BREQ PC+0x04
	MOV TEMP,TEMP_H
	MCALL CORE5277_LOG_CHAR
	CPI TEMP_L,'0'
	BREQ PC+0x04
	MOV TEMP,TEMP_L
	MCALL CORE5277_LOG_CHAR
	POP TEMP
	MCALL CORE5277_LOG_CHAR
	POP TEMP_L
	CPI TEMP_L,0x00
	BREQ _CORE5277_LOG_SDNF__NO_FRACTIONAL

	LDI TEMP,'.'
	MCALL CORE5277_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL CORE5277_BYTE_TO_STR
	PUSH TEMP
	MOV TEMP,TEMP_L
	MCALL CORE5277_LOG_CHAR
	POP TEMP
	MCALL CORE5277_LOG_CHAR
	RJMP _CORE5277_LOG_SDNF__END
_CORE5277_LOG_SDNF__NULL_VALUE:
	CORE5277_LOG_ROMSTR LOGSTR_NULL
	RJMP _CORE5277_LOG_SDNF__END
_CORE5277_LOG_SDNF__NO_FRACTIONAL:
_CORE5277_LOG_SDNF__END:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
.endif
