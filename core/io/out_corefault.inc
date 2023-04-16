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
.include	"./core/io/out_meminfo.inc"
.include	"./core/io/out_ramdump.inc"
.include "./sys/wd_reset.inc"

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
_OUTSTR_CORE_TYPE_0x04:
	.db ", INSUFFICIENT RAM",0x00,0x00
_OUTSTR_CORE_TYPE_0x05:
	.db 0x0d,0x0a,"SP:",0x00
_OUTSTR_CORE_TYPE_0x06:
	.db ",TOP_SP:",0x00,0x00
_OUTSTR_CORE_TYPE_0x07:
	.db ",FREE_RAM:",0x00,0x00
_OUTSTR_CORE_TYPE_0x08:
	.db ",TOP_FREE_RAM:",0x00,0x00



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
	CPI TEMP,_C5_FAULT_INSUFFICIENT_RAM
	BRNE _C5_OUT_COREFAULT__NO_TYPE_0x04
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x04
	RJMP _C5_OUT_COREFAULT__TYPE_LOGGED
_C5_OUT_COREFAULT__NO_TYPE_0x04:

	MCALL C5_OUT_BYTE
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_WORD
_C5_OUT_COREFAULT__TYPE_LOGGED:
	MCALL C5_OUT_CR
	MCALL C5_OUT_MEMINFO
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x05
	LDS TEMP_H,SPH
	LDS TEMP_L,SPL
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x06
	LDS TEMP_H,_C5_TOP_OF_STACK+0x00
	LDS TEMP_L,_C5_TOP_OF_STACK+0x01
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x07
	LDI TEMP_H,high(_C5_FREE_RAM)
	LDI TEMP_L,low(_C5_FREE_RAM)
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_CORE_TYPE_0x08
	LDS TEMP_H,_C5_TOP_OF_FREE_RAM+0x00
	LDS TEMP_L,_C5_TOP_OF_FREE_RAM+0x01
	MCALL C5_OUT_WORD
	LDI_Z _C5_FREE_RAM
	ANDI ZL,0xf0
	LDI TEMP_H,high(RAMEND+0x01)
	LDI TEMP_L,low(RAMEND+0x01)
	SUB TEMP_L,ZL
	SBC TEMP_H,ZH
	MCALL C5_OUT_RAMDUMP
	MJMP WD_RESET
.endif
.endif
