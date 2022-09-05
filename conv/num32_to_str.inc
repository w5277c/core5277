;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало (не тестировано)
;18.10.2021	w5277c@gmail.com			Оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_NUMX32_TO_STR
.else
.set DEF_NUMX32_TO_STR = 1

.include	"./math/div32x16.inc"

;--------------------------------------------------------
NUM32_TO_STR:
;--------------------------------------------------------
;Запись 4Б числа в виде строки
;IN: TEMP_EH/EL/H/L-32b слово, Z-dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH_FT
	PUSH LOOP_CNTR
	PUSH_X

	LDI LOOP_CNTR,0x09
_NUM32_TO_STR__LOOP:
	LDI_X 0x0a
	MCALL DIV32x16
	SUBI XL,(0x100-0x30)
	PUSH XL
	DEC LOOP_CNTR
	BRNE _NUM32_TO_STR__LOOP
	SUBI TEMP_L,(0x100-0x30)
	PUSH TEMP_L

	LDI LOOP_CNTR,0x09
_NUM32_TO_STR__LOOP2:
	POP TEMP
	SBRC TEMP_L,0x07
	RJMP PC+0x03
	CPI TEMP,0x30
	BREQ PC+0x03
	ST Z+,TEMP
	ORI TEMP_L,(1<<0x07)
	DEC LOOP_CNTR
	BRNE _NUM32_TO_STR__LOOP2
	POP TEMP
	ST Z+,TEMP

	POP_X
	POP LOOP_CNTR
	POP_FT
	RET
.endif
