;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;03.10.2020  w5277c@gmail.com        Оптимизация, атомарность
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_PORT_INVERT
.else
.set DEF_CORE5277_PORT_INVERT = 1

.include	"./inc/io/port_offsets.inc"

;--------------------------------------------------------
CORE5277_PORT_INVERT:
;--------------------------------------------------------
;Инвертирование состояния порта
;IN: ACCUM адрес регистра PORTx
;--------------------------------------------------------;
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL CORE5277_PORT_OFFSETS

	LDS TEMP_L,SREG
	CLI
	CLR ZH
	MOV ZL,TEMP
	LD TEMP,Z
	MOV ZL,TEMP_H
	LD TEMP_H,Z
	AND TEMP,ACCUM
	BREQ PC+0x04
	COM ACCUM
	AND TEMP_H,ACCUM
	RJMP PC+0x02
	OR TEMP_H,ACCUM
	ST Z,TEMP_H
	STS SREG,TEMP_L

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
