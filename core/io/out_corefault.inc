;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com        Начало
;02.08.2020	w5277c@gmail.com        Разбиение на файлы
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_COREFAULT
.else
.set DEF_C5_OUT_COREFAULT = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_byte.inc"
.include	"./core/io/out_word.inc"
.include	"./core/io/out_romstr.inc"

_OUTSTR_COREFAULT:
	.db 0x0d,0x0a,"---COREFAULT, CALLER:",0x00
_OUTSTR_COREFAULT_TID:
	.db ", TID:",0x00,0x00
_OUTSTR_COREFAULT_DID:
	.db ", DID:",0x00,0x00
_OUTSTR_CORE_TYPE_0x01:
	.db ", HUGE_STACK, SIZE:",0x00
_OUTSTR_CORE_TYPE_0x02:
	.db ", HUGE_TASK_STACK, OFFSET:",0x00,0x00
_OUTSTR_CORE_TYPE_0x03:
	.db ", HUGE_TASK_STACK_2, OFFSET:",0x00,0x00


;--------------------------------------------------------
C5_OUT_COREFAULT:
;--------------------------------------------------------
;Логирование сбоя
;IN PID-ид задачи, TEMP-код сбоя,
;TEMP_H/TEMP_L-размер стека либо его позиция
;--------------------------------------------------------
	CLI
	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L
	POP TEMP_H
	POP TEMP_L
	SUBI TEMP_L,0x02
	SBCI TEMP_H,0x00
	C5_OUT_ROMSTR _OUTSTR_COREFAULT
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_COREFAULT_TID
	PUSH TEMP
	MOV TEMP,_PID
	MCALL C5_OUT_BYTE
	C5_OUT_ROMSTR _OUTSTR_COREFAULT_DID
	MOV TEMP,PID
	MCALL C5_OUT_BYTE
	POP TEMP
	MOV TEMP_H,TEMP_EH
	MOV TEMP_L,TEMP_EL
	CPI TEMP,_C5_FAULT_HUGE_STACK
	BRNE _C5_OUT_COREFAULT__NO_TYPE_0x01
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x01
	MCALL C5_OUT_WORD
	RJMP _C5_OUT_COREFAULT__TYPE_LOGGED
_C5_OUT_COREFAULT__NO_TYPE_0x01:
	CPI TEMP,_C5_FAULT_HUGE_TASK_STACK
	BRNE _C5_OUT_COREFAULT__NO_TYPE_0x02
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x02
	MCALL C5_OUT_WORD
	RJMP _C5_OUT_COREFAULT__TYPE_LOGGED
_C5_OUT_COREFAULT__NO_TYPE_0x02:
	CPI TEMP,_C5_FAULT_HUGE_TASK_STACK2
	BRNE _C5_OUT_COREFAULT__NO_TYPE_0x03
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x03
	MCALL C5_OUT_WORD
	RJMP _C5_OUT_COREFAULT__TYPE_LOGGED
_C5_OUT_COREFAULT__NO_TYPE_0x03:

	MCALL C5_OUT_BYTE
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_WORD
_C5_OUT_COREFAULT__TYPE_LOGGED:
	MJMP MAIN
.endif
.endif
