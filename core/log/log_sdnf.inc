;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_SDNF
.else
.set DEF_C5_LOG_SDNF = 1
.ifdef LOGGING_PORT

.include	"./conv/byte_to_str.inc"
.include	"./core/log/log_char.inc"
.include	"./core/log/log_romstr.inc"
.include	"./core/log/logstr_null.inc"

;--------------------------------------------------------
C5_LOG_SDNF:
;--------------------------------------------------------
;Логирование SDNF числа
;IN: TEMP_H/TEMP_L-SDNF число
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	CPI TEMP_L,0xff
	BREQ _C5_LOG_SDNF__NULL_VALUE

	SBRS TEMP_L,0x07
	RJMP _C5_LOG_SDNF__POSITIVE_VALUE
	LDI TEMP,'-'
	MCALL C5_LOG_CHAR
	ANDI TEMP_L,0x7f
_C5_LOG_SDNF__POSITIVE_VALUE:

	PUSH TEMP_L
	MOV TEMP,TEMP_H
	MCALL BYTE_TO_STR
	PUSH TEMP
	CPI TEMP_H,'0'
	BREQ PC+0x04
	MOV TEMP,TEMP_H
	MCALL C5_LOG_CHAR
	CPI TEMP_L,'0'
	BREQ PC+0x04
	MOV TEMP,TEMP_L
	MCALL C5_LOG_CHAR
	POP TEMP
	MCALL C5_LOG_CHAR
	POP TEMP_L
	CPI TEMP_L,0x00
	BREQ _C5_LOG_SDNF__NO_FRACTIONAL

	LDI TEMP,'.'
	MCALL C5_LOG_CHAR

	MOV TEMP,TEMP_L
	MCALL BYTE_TO_STR
	PUSH TEMP
	MOV TEMP,TEMP_L
	MCALL C5_LOG_CHAR
	POP TEMP
	MCALL C5_LOG_CHAR
	RJMP _C5_LOG_SDNF__END
_C5_LOG_SDNF__NULL_VALUE:
	C5_LOG_ROMSTR LOGSTR_NULL
	RJMP _C5_LOG_SDNF__END
_C5_LOG_SDNF__NO_FRACTIONAL:
_C5_LOG_SDNF__END:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
.endif
