;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.04.2023	konstantin@5277.ru		Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_MEMINFO
.else
.set DEF_C5_OUT_MEMINFO = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_word.inc"
.include	"./core/io/out_romstr.inc"

_OUTSTR_MEMINFO:
	.db "MEM:T",0x00

;--------------------------------------------------------
C5_OUT_MEMINFO:
;--------------------------------------------------------
;Вывод информации о памяти:
;Txxxx,Dxxxx,Sxxxx,Fxxxx,где T-total,D-dyn,S-stack,F-free
;--------------------------------------------------------
	PUSH TEMP
	PUSH_T16
	PUSH_Z
	PUSH_X


	C5_OUT_ROMSTR _OUTSTR_MEMINFO
	LDI TEMP_H,high(_C5_STACK_END-_C5_FREE_RAM)
	LDI TEMP_L,low(_C5_STACK_END-_C5_FREE_RAM)
	MCALL C5_OUT_WORD
	LDI TEMP,','
	MCALL C5_OUT_CHAR

	LDS TEMP,SREG
	CLI
	LDS ZH,_C5_TOP_OF_FREE_RAM+0x00
	LDS ZL,_C5_TOP_OF_FREE_RAM+0x01
	LDS XH,_C5_TOP_OF_STACK+0x00
	LDS XL,_C5_TOP_OF_STACK+0x01
	STS SREG,TEMP

	LDI TEMP,'D'
	MCALL C5_OUT_CHAR
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	SUBI TEMP_L,low(_C5_FREE_RAM)
	SBCI TEMP_H,high(_C5_FREE_RAM)
	MCALL C5_OUT_WORD
	LDI TEMP,','
	MCALL C5_OUT_CHAR

	LDI TEMP,'S'
	MCALL C5_OUT_CHAR
	LDI TEMP_H,high(_C5_STACK_END)
	LDI TEMP_L,low(_C5_STACK_END)
	SUB TEMP_L,XL
	SBC TEMP_H,XH
	MCALL C5_OUT_WORD
	LDI TEMP,','
	MCALL C5_OUT_CHAR

	LDI TEMP,'F'
	MCALL C5_OUT_CHAR
	MOV TEMP_H,XH
	MOV TEMP_L,XL
	SUB TEMP_L,ZL
	SBC TEMP_H,ZH
	MCALL C5_OUT_WORD

	POP_X
	POP_Z
	POP_T16
	POP TEMP
	RET
.endif
.endif
