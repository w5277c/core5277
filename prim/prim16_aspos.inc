;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.02.2024	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM16_ASPOS
.else
.set DEF_PRIM16_ASPOS = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM16_ASPOS"
.endif

;--------------------------------------------------------
PRIM16_ASPOS:
;--------------------------------------------------------
;Меняем знак .
;IN: Y-адрес на память с переменными, X-смещение на
;переменную, ACCUM-тип примитива
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP
	PUSH_T16
	PUSH ACCUM

	ADD XL,YL
	ADC XH,YH
	LD TEMP,X
	ANDI TEMP,0x7f
	ST X,TEMP
	ANDI ACCUM,0x0f
	ADD XL,ACCUM
	ADC XH,C0x00

	SEC
	LDS TEMP,SREG
_PRIM16_ASPOS_LOOP:
	LD TEMP_H,-X
	STS SREG,TEMP
	ADC TEMP_H,C0x00
	LDS TEMP,SREG
	ST X,TEMP_H
	DEC ACCUM
	BRNE _PRIM16_ASPOS_LOOP

	POP ACCUM
	POP_T16
	POP TEMP
	POP_X
	RET
