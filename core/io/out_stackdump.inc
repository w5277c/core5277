;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.09.2020	w5277c@gmail.com        Начало
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_STACKDUMP
.else
.set DEF_C5_OUT_STACKDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_byte.inc"
.include	"./core/io/out_word.inc"
.include	"./core/io/out_romstr.inc"

_OUTSTR_INVALID:
	.db   "INVALID",0x00

;--------------------------------------------------------
C5_OUT_STACKDUMP:
;--------------------------------------------------------
;Логирование информации о задаче
;IN: Z-адрес на заголовок задачи
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH ZH
	PUSH ZL

	LDD LOOP_CNTR,Z+_C5_TASK_STACK_SIZE
	LDD TEMP,Z+_C5_TASK_STACK_OFFSET+0x01
	LDD ZH,Z+_C5_TASK_STACK_OFFSET+0x00
	MOV ZL,TEMP

	CPI LOOP_CNTR,0x00
	BREQ _C5_OUT_STACKDUMP__END

	CPI ZH,high(_C5_STACK_END)
	BRCS PC+0x05
	BRNE _C5_OUT_STACKDUMP__INVALID
	CPI ZL,low(_C5_STACK_END)
	BRCS PC+0x02
	BRNE _C5_OUT_STACKDUMP__INVALID
	CPI ZH,high(_C5_FREE_RAM)
	BRCS _C5_OUT_STACKDUMP__INVALID
	BRNE PC+0x03
	CPI ZL,low(_C5_FREE_RAM)
	BRCS _C5_OUT_STACKDUMP__INVALID

	LDI TEMP,'['
	MCALL C5_OUT_CHAR
	LD TEMP,Z+
_C5_OUT_STACKDUMP__LOOP:
	LD TEMP,-Z
	MCALL C5_OUT_BYTE
	DEC LOOP_CNTR
	BRNE _C5_OUT_STACKDUMP__LOOP
	RJMP _C5_OUT_STACKDUMP__DONE

_C5_OUT_STACKDUMP__INVALID:
	LDI TEMP,'['
	MCALL C5_OUT_CHAR
	C5_OUT_ROMSTR _OUTSTR_INVALID

_C5_OUT_STACKDUMP__DONE:
	LDI TEMP,']'
	MCALL C5_OUT_CHAR
_C5_OUT_STACKDUMP__END:
	POP ZL
	POP ZH
	POP LOOP_CNTR
	POP TEMP
	RET
.endif
.endif
