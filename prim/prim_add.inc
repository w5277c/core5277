;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.02.2021  w5277c@gmail.com			Начало
;06.02.2021  w5277c@gmail.com			Изменены аргументы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_ADD
.else
.set DEF_PRIM_ADD = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM_ADD"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_ADD:
;--------------------------------------------------------
;Сложение примитивов
;IN: Z-адрес на память с переменными, TEMP_H-смещение для
;слагаемого и результата, TEMP_L-смещение второго слага-
;емого, ACCUM-тип примитива
;OUT: флаги SREG после команды ADC
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	MCALL _PRIM_OFFSETS_R

	CLC
	LDS TEMP,SREG
	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
__PRIM_ADD__LOOP:
	LD TEMP_H,-Z
	LD TEMP_L,-X
	STS SREG,TEMP
	ADC TEMP_H,TEMP_L
	LDS TEMP,SREG
	ST Z,TEMP_H
	DEC LOOP_CNTR
	BRNE __PRIM_ADD__LOOP
	STS SREG,TEMP

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
