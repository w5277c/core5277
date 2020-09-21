;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_RAMDUMP
.else
.set DEF_CORE5277_LOG_RAMDUMP = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_word.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_done.inc"
.include	"./inc/io/logstr_new_line.inc"

_LOGSTR_RAMDUMP1:
	.db   0x0d,0x0a,"---RAMDUMP:0x",0x00
_LOGSTR_RAMDUMP2:
	.db   ",0x",0x00
_LOGSTR_RAMDUMP3:
	.db   ":",0x0d,0x0a,0x00

;--------------------------------------------------------
CORE5277_LOG_RAMDUMP:
;--------------------------------------------------------
;Логируем блок памяти
;IN: Z - адрес начала блока, TEMP - длина блока
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	CORE5277_LOG_ROMSTR _LOGSTR_RAMDUMP1
	MCALL CORE5277_LOG_WORD
	CORE5277_LOG_ROMSTR _LOGSTR_RAMDUMP2
	MCALL CORE5277_LOG_BYTE
	CORE5277_LOG_ROMSTR _LOGSTR_RAMDUMP3
	MOV TEMP_H,TEMP
	MOV TEMP_L,TEMP

_CORE5277_LOG_RAMDUMP__LOOP:
	PUSH_Z
	LDI LOOP_CNTR,0x08
_CORE5277_LOG_RAMDUMP__HEX_LOOP:
	CPI TEMP_H,0x00
	BREQ _CORE5277_LOG_RAMDUMP__HEX_NO_DATA
	LD TEMP,Z+
	MCALL CORE5277_LOG_BYTE
	DEC TEMP_H
	RJMP _CORE5277_LOG_RAMDUMP__HEX_NEXT
_CORE5277_LOG_RAMDUMP__HEX_NO_DATA:
	LDI TEMP,'-'
	MCALL CORE5277_LOG_CHAR
	MCALL CORE5277_LOG_CHAR
_CORE5277_LOG_RAMDUMP__HEX_NEXT:
	LDI TEMP,' '
	MCALL CORE5277_LOG_CHAR
	DEC LOOP_CNTR
	BRNE _CORE5277_LOG_RAMDUMP__HEX_LOOP
	POP_Z

	LDI LOOP_CNTR,0x08
_CORE5277_LOG_RAMDUMP__ASCII_LOOP:
	CPI TEMP_L,0x00
	BREQ _CORE5277_LOG_RAMDUMP__ASCII_NO_DATA
	DEC TEMP_L
	LD TEMP,Z+
	CPI TEMP,0x20
	BRCS _CORE5277_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	CPI TEMP,0x7f
	BRCC _CORE5277_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	MCALL CORE5277_LOG_CHAR
	RJMP _CORE5277_LOG_RAMDUMP__ASCII_NEXT
_CORE5277_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA:
	LDI TEMP,'?'
	MCALL CORE5277_LOG_CHAR
	RJMP _CORE5277_LOG_RAMDUMP__ASCII_NEXT
_CORE5277_LOG_RAMDUMP__ASCII_NO_DATA:
	LDI TEMP,'-'
	MCALL CORE5277_LOG_CHAR
_CORE5277_LOG_RAMDUMP__ASCII_NEXT:
	DEC LOOP_CNTR
	BRNE _CORE5277_LOG_RAMDUMP__ASCII_LOOP

	CPI TEMP_L,0x00
	BREQ _CORE5277_LOG_RAMDUMP__END
	MOV TEMP_H,TEMP_L
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE
	RJMP _CORE5277_LOG_RAMDUMP__LOOP
_CORE5277_LOG_RAMDUMP__END:
	CORE5277_LOG_ROMSTR _LOGSTR_DONE

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
.endif
