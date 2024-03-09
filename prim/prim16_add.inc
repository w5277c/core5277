;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.02.2024	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM16_ADD
.else
.set DEF_PRIM16_ADD = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM16_ADD"
.endif

.include	"./prim/_offsets16.inc"

;--------------------------------------------------------
PRIM16_ADD:
;--------------------------------------------------------
;Сложение примитивов
;IN: Y-адрес на память с переменными, X-смещение для
;слагаемого и результата, Z-смещение второго слага-
;емого, ACCUM-тип примитива
;OUT: флаги SREG после команды ADC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	MCALL _PRIM16_OFFSETS_R

	CLC
	LDS TEMP,SREG
	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
__PRIM16_ADD__LOOP:
	LD TEMP_H,-Z
	LD TEMP_L,-X
	STS SREG,TEMP
	ADC TEMP_H,TEMP_L
	LDS TEMP,SREG
	ST Z,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM16_ADD__LOOP
	STS SREG,TEMP

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
