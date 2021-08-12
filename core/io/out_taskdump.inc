;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020	w5277c@gmail.com        Начало
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_TASKDUMP
.else
.set DEF_C5_OUT_TASKDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_byte.inc"
.include	"./core/io/out_word.inc"
.include	"./core/io/out_romstr.inc"
.include	"./core/io/out_cr.inc"
.include	"./core/io/out_stackdump.inc"

_OUTSTR_TID:
	.db   "TID:",0x00,0x00
_OUTSTR_STATE:
	.db   "STA:",0x00,0x00
_OUTSTR_RAM:
	.db   "RAM:",0x00,0x00
_OUTSTR_TIME:
	.db   "TIM:",0x00,0x00
_OUTSTR_STACK:
	.db   "STK:",0x00,0x00

;--------------------------------------------------------
C5_OUT_TASKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: PID
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	MCALL _C5_PROC_HEADER_GET
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_ABSENT
	BRNE PC+0x02
	RJMP C5_OUT_TASKDUMP__END

	MCALL C5_OUT_CR

	C5_OUT_ROMSTR _OUTSTR_TID
	MOV TEMP,PID
	MCALL C5_OUT_BYTE
	LDI TEMP,' '
	MCALL C5_OUT_CHAR

	LDD TEMP,Z+_C5_PROC_STATE
	C5_OUT_ROMSTR _OUTSTR_STATE
	MCALL C5_OUT_BYTE

	LDI TEMP,' '
	MCALL C5_OUT_CHAR

	LDD TEMP_H,Z+_C5_PROC_RAM_OFFSET+0x00
	LDD TEMP_L,Z+_C5_PROC_RAM_OFFSET+0x01
	LDD TEMP,Z+_C5_PROC_RAM_SIZE
	C5_OUT_ROMSTR _OUTSTR_RAM
	MCALL C5_OUT_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL C5_OUT_CHAR
	POP TEMP
	MCALL C5_OUT_BYTE
	LDI TEMP,' '
	MCALL C5_OUT_CHAR

	LDD TEMP_H,Z+_C5_TASK_EXECTIME+0x00
	LDD TEMP_L,Z+_C5_TASK_EXECTIME+0x01
	LDD TEMP,Z+_C5_TASK_EXECTIME+0x02
	C5_OUT_ROMSTR _OUTSTR_TIME
	MCALL C5_OUT_WORD
	MCALL C5_OUT_BYTE
	LDI TEMP,' '
	MCALL C5_OUT_CHAR

	LDD TEMP_H,Z+_C5_TASK_STACK_OFFSET+0x00
	LDD TEMP_L,Z+_C5_TASK_STACK_OFFSET+0x01
	LDD TEMP,Z+_C5_TASK_STACK_SIZE
	C5_OUT_ROMSTR _OUTSTR_STACK
	MCALL C5_OUT_WORD
	PUSH TEMP
	LDI TEMP,'.'
	MCALL C5_OUT_CHAR
	POP TEMP
	MCALL C5_OUT_BYTE

	MCALL C5_OUT_STACKDUMP

C5_OUT_TASKDUMP__END:
	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
.endif
