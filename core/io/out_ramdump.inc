;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com			Начало
;02.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;01.02.2021	w5277c@gmail.com			Вывод по 16 байт, 16 бит длина
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_RAMDUMP
.else
.set DEF_C5_OUT_RAMDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"
.include	"./core/io/out_byte.inc"
.include	"./core/io/out_word.inc"
.include	"./core/io/out_romstr.inc"
.include	"./core/io/outstr_done.inc"
.include	"./core/io/out_cr.inc"

_OUTSTR_RAMDUMP1:
	.db   0x0d,0x0a,"---RAMDUMP:0x",0x00
_OUTSTR_RAMDUMP2:
	.db   ",0x",0x00
_OUTSTR_RAMDUMP3:
	.db   ":",0x0d,0x0a,0x00

;--------------------------------------------------------
C5_OUT_RAMDUMP:
;--------------------------------------------------------
;Логируем блок памяти
;IN: Z-адрес начала блока, TEMP_H/L-длина блока
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	PUSH TEMP_H
	PUSH TEMP_L
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	C5_OUT_ROMSTR _OUTSTR_RAMDUMP1
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_RAMDUMP2
	POP TEMP_L
	POP TEMP_H
	MCALL C5_OUT_WORD
	C5_OUT_ROMSTR _OUTSTR_RAMDUMP3
	MOV TEMP_EH,TEMP_H
	MOV TEMP_EL,TEMP_L

_C5_OUT_RAMDUMP__LOOP:
	PUSH_Z
	LDI LOOP_CNTR,0x10
_C5_OUT_RAMDUMP__HEX_LOOP:
	CPI TEMP_H,0x00
	BRNE PC+0x03
	CPI TEMP_L,0x00
	BREQ _C5_OUT_RAMDUMP__HEX_NO_DATA
	LD TEMP,Z+
	MCALL C5_OUT_BYTE
	SUBI TEMP_L,0x01
	SBCI TEMP_H,0x00
	RJMP _C5_OUT_RAMDUMP__HEX_NEXT
_C5_OUT_RAMDUMP__HEX_NO_DATA:
	LDI TEMP,'-'
	MCALL C5_OUT_CHAR
	MCALL C5_OUT_CHAR
_C5_OUT_RAMDUMP__HEX_NEXT:
	LDI TEMP,' '
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE _C5_OUT_RAMDUMP__HEX_LOOP
	POP_Z

	LDI LOOP_CNTR,0x10
_C5_OUT_RAMDUMP__ASCII_LOOP:
	CPI TEMP_EH,0x00
	BRNE PC+0x03
	CPI TEMP_EL,0x00
	BREQ _C5_OUT_RAMDUMP__ASCII_NO_DATA
	SUBI TEMP_EL,0x01
	SBCI TEMP_EH,0x00
	LD TEMP,Z+
	CPI TEMP,0x20
	BRCS _C5_OUT_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	CPI TEMP,0x7f
	BRCC _C5_OUT_RAMDUMP__ASCII_NOT_PRINTABLE_DATA
	MCALL C5_OUT_CHAR
	RJMP _C5_OUT_RAMDUMP__ASCII_NEXT
_C5_OUT_RAMDUMP__ASCII_NOT_PRINTABLE_DATA:
	LDI TEMP,'?'
	MCALL C5_OUT_CHAR
	RJMP _C5_OUT_RAMDUMP__ASCII_NEXT
_C5_OUT_RAMDUMP__ASCII_NO_DATA:
	LDI TEMP,'-'
	MCALL C5_OUT_CHAR
_C5_OUT_RAMDUMP__ASCII_NEXT:
	DEC LOOP_CNTR
	BRNE _C5_OUT_RAMDUMP__ASCII_LOOP

	CPI TEMP_H,0x00
	BRNE PC+0x03
	CPI TEMP_L,0x00
	BREQ _C5_OUT_RAMDUMP__END
	MCALL C5_OUT_CR
	RJMP _C5_OUT_RAMDUMP__LOOP
_C5_OUT_RAMDUMP__END:
	C5_OUT_ROMSTR OUTSTR_DONE

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
.endif
