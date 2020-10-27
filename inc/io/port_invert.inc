;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;03.10.2020  w5277c@gmail.com        Оптимизация, атомарность
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_PORT_INVERT
.else
.set DEF_C5_PORT_INVERT = 1

.include	"./inc/io/port_offsets.inc"

;--------------------------------------------------------
C5_PORT_INVERT:
;--------------------------------------------------------
;Инвертирование состояния порта
;IN: ACCUM адрес регистра PORTx
;--------------------------------------------------------;
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_PORT_OFFSETS

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
