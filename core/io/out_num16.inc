;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.01.2021	w5277c@gmail.com			Начало (не тестировано)
;18.10.2021	w5277c@gmail.com			Оптимизация
;04.01.2023	konstantin@5277.ru			Актуализация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_NUM16
.else
.set DEF_C5_OUT_NUM16 = 1
.ifdef LOGGING_PORT

.include	"./math/div16x8.inc"
.include	"./core/io/out_char.inc"

;--------------------------------------------------------
C5_OUT_NUM16:
;--------------------------------------------------------
;Логирование десятиричного 16b числа
;IN: TEMP_H/L-16b число
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,0x04
_C5_OUT_NUM16__LOOP:
	LDI TEMP,0x0a
	MCALL DIV16x8
	PUSH TEMP
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM16__LOOP
	PUSH TEMP_L

	LDI TEMP_L,0x00
	LDI LOOP_CNTR,0x05
_C5_OUT_NUM16__LOOP2:
	POP TEMP
	CPI LOOP_CNTR,0x01
	BREQ PC+0x03
	OR TEMP_L,TEMP
	BREQ PC+0x02+_MCALL_SIZE
	SUBI TEMP,(0x100-0x30)
	MCALL C5_OUT_CHAR
	DEC LOOP_CNTR
	BRNE _C5_OUT_NUM16__LOOP2

	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
.endif
