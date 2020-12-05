;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_RAMDUMP
.else
.set DEF_C5_LOG_RAMDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"
.include	"./core/log/log_byte.inc"
.include	"./core/log/log_word.inc"
.include	"./core/log/log_romstr.inc"
.include	"./core/log/logstr_done.inc"
.include	"./core/log/logstr_new_line.inc"

_LOGSTR_RAMDUMP1:
	.db   0x0d,0x0a,"---RAMDUMP:0x",0x00
_LOGSTR_RAMDUMP2:
	.db   ",0x",0x00
_LOGSTR_RAMDUMP3:
	.db   ":",0x0d,0x0a,0x00

;--------------------------------------------------------
C5_LOG_RAMDUMP:
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
	C5_LOG_ROMSTR _LOGSTR_RAMDUMP1
	MCALL C5_LOG_WORD
	C5_LOG_ROMSTR _LOGSTR_RAMDUMP2
	MCALL C5_LOG_BYTE
	C5_LOG_ROMSTR _LOGSTR_RAMDUMP3
	MOV TEMP_H,TEMP
	MOV TEMP_L,TEMP

_C5_LOG_RAMDUMP__LOOP:
	PUSH_Z
	LDI LOOP_CNTR,0x08
_C5_LOG_RAMDUMP__HEX_LOOP:
	CPI TEMP_H,0x00
	BREQ _C5_LOG_RAMDUMP__HEX_NO_DATA
	LD TEMP,Z+
	MCALL C5_LOG_BYTE
	DEC TEMP_H
	RJMP _C5_LOG_RAMDUMP__HEX_NEXT
_C5_LOG_RAMDUMP__HEX_NO_DATA:
	LDI TEMP,'-'
	MCALL C5_LOG_CHAR
	MCALL C5_LOG_CHAR
_C5_LOG_RAMDUMP__HEX_NEXT:
	LDI TEMP,' '
	MCALL C5_LOG_CHAR
	DEC LOOP_CNTR
	BRNE _C5_LOG_RAMDUMP__HEX_LOOP
	POP_Z

	LDI LOOP_CNTR,0x08
_C5_LOG_RAMDUMP__ASCII_LOOP:
	CPI TEMP_L,0x00
	BREQ _C5_LOG_RAMDUMP__ASCII_NO_DATA
	DEC TEMP_L
	LD TEMP,Z+
	CPI TEMP,0x20
	BRCS _C5_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	CPI TEMP,0x7f
	BRCC _C5_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	MCALL C5_LOG_CHAR
	RJMP _C5_LOG_RAMDUMP__ASCII_NEXT
_C5_LOG_RAMDUMP__ASCII_NOT_PRINTABLE_DATA:
	LDI TEMP,'?'
	MCALL C5_LOG_CHAR
	RJMP _C5_LOG_RAMDUMP__ASCII_NEXT
_C5_LOG_RAMDUMP__ASCII_NO_DATA:
	LDI TEMP,'-'
	MCALL C5_LOG_CHAR
_C5_LOG_RAMDUMP__ASCII_NEXT:
	DEC LOOP_CNTR
	BRNE _C5_LOG_RAMDUMP__ASCII_LOOP

	CPI TEMP_L,0x00
	BREQ _C5_LOG_RAMDUMP__END
	MOV TEMP_H,TEMP_L
	C5_LOG_ROMSTR LOGSTR_NEW_LINE
	RJMP _C5_LOG_RAMDUMP__LOOP
_C5_LOG_RAMDUMP__END:
	C5_LOG_ROMSTR LOGSTR_DONE

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
.endif
