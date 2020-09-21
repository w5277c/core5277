;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_COREFAULT
.else
.set DEF_CORE5277_LOG_COREFAULT = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"

_LOGSTR_COREFAULT:
	.db 0x0d,0x0a,"---COREFAULT, CALLER:",0x00
_LOGSTR_COREFAULT_TID:
	.db ", TID:",0x00,0x00
_LOGSTR_COREFAULT_DID:
	.db ", DID:",0x00,0x00
_LOGSTR_CORE_TYPE_0x01:
	.db ", HUGE_STACK, SIZE:",0x00
_LOGSTR_CORE_TYPE_0x02:
	.db ", HUGE_TASK_STACK, OFFSET:",0x00,0x00


;--------------------------------------------------------
CORE5277_LOG_COREFAULT:
;--------------------------------------------------------
;Логирование сбоя
;IN PID - ид задачи, TEMP - код сбоя,
;TEMP_H/TEMP_L - размер стека либо его позиция
;--------------------------------------------------------
	CLI
	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L
	POP TEMP_H
	POP TEMP_L
	SUBI TEMP_L,0x02
	SBCI TEMP_H,0x00
	CORE5277_LOG_ROMSTR _LOGSTR_COREFAULT
	MCALL CORE5277_LOG_WORD
	CORE5277_LOG_ROMSTR _LOGSTR_COREFAULT_TID
	PUSH TEMP
	MOV TEMP,_PID
	MCALL CORE5277_LOG_BYTE
	CORE5277_LOG_ROMSTR _LOGSTR_COREFAULT_DID
	MOV TEMP,PID
	MCALL CORE5277_LOG_BYTE
	POP TEMP
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	CPI TEMP,_CORE5277_FAULT_HUGE_STACK
	BRNE _CORE5277_LOG_COREFAULT__NO_TYPE_0x01
	CORE5277_LOG_ROMSTR _LOGSTR_CORE_TYPE_0x01
	MCALL CORE5277_LOG_WORD
	RJMP _CORE5277_LOG_COREFAULT__TYPE_LOGGED
_CORE5277_LOG_COREFAULT__NO_TYPE_0x01:
	CPI TEMP,_CORE5277_FAULT_HUGE_TASK_STACK
	BRNE _CORE5277_LOG_COREFAULT__NO_TYPE_0x02
	CORE5277_LOG_ROMSTR _LOGSTR_CORE_TYPE_0x02
	MCALL CORE5277_LOG_WORD
	RJMP _CORE5277_LOG_COREFAULT__TYPE_LOGGED
_CORE5277_LOG_COREFAULT__NO_TYPE_0x02:
	MCALL CORE5277_LOG_BYTE
	LDI TEMP,':'
	MCALL CORE5277_LOG_CHAR
	MCALL CORE5277_LOG_WORD
_CORE5277_LOG_COREFAULT__TYPE_LOGGED:
	MJMP MAIN
.endif
.endif
