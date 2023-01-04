;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало (не тестировано)
;04.01.2023	konstantin@5277.ru			Актуализация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_NUM32
.else
.set DEF_C5_OUT_NUM32 = 1
.ifdef LOGGING_PORT

.include	"./math/div32x16.inc"
.include	"./core/io/out_char.inc"

;--------------------------------------------------------
C5_OUT_NUM32:
;--------------------------------------------------------
;Логирование десятиричного числа(4 байта)
;IN: TEMP_EH/EL/H/L-двойное слово
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH_X

	LDI LOOP_CNTR,0x09
_C5_OUT_NUM32__LOOP1:
	LDI_X 0x000a
	MCALL DIV32X16
	PUSH XL
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM32__LOOP1
	PUSH TEMP_L

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x0a
_C5_OUT_NUM32__LOOP2:
	POP TEMP
	CPI LOOP_CNTR,0x01
	BREQ PC+0x03
	OR TEMP_L,TEMP
	BREQ PC+0x02+_MCALL_SIZE
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM32__LOOP2

	POP_X
	POP LOOP_CNTR
	POP_FT
	RET
.endif
.endif
