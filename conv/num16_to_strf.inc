;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021	w5277c@gmail.com			Начало
;18.10.2021	w5277c@gmail.com			Оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_NUM16_TO_STRF
.else
.set DEF_NUM16_TO_STRF = 1

.if REPORT_INCLUDES == 0x01
.message "included NUM16_TO_STRF"
.endif

.include	"./math/div16x8.inc"

;--------------------------------------------------------
NUM16_TO_STRF:
;--------------------------------------------------------
;Конвертирование знакового целого с дробной в текст
;Конец строки не указывается
;IN: TEMP_H/L-число (00000-65535), Z-dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	ADIW ZL,0x05

	LDI LOOP_CNTR,0x04
_NUM16_TO_STRF__LOOP:
	LDI TEMP,0x0a
	MCALL DIV16x8
	SUBI TEMP,(0x100-0x30)
	ST -Z,TEMP
	DEC LOOP_CNTR
	BRNE _NUM16_TO_STRF__LOOP
	SUBI TEMP_L,(0x100-0x30)
	ST -Z,TEMP_L

	ADIW ZL,0x05

	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP LOOP_CNTR
	RET
.endif
