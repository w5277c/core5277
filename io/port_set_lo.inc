;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;03.10.2020  w5277c@gmail.com			Атомарность
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_SET_LO
.else
.set DEF_PORT_SET_LO = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_SET_LO"
.endif

.include	"./io/port_offsets.inc"

;--------------------------------------------------------
PORT_SET_LO:
;--------------------------------------------------------
;Устанвливаем порту низкий уровень
;IN: ACCUM - порт
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM

	MCALL PORT_OFFSETS
	CLR ZH
	MOV ZL,TEMP_H

	LDS TEMP_L,SREG
	CLI
	LD TEMP,Z
	COM ACCUM
	AND TEMP,ACCUM
	ST Z,TEMP
	STS SREG,TEMP_L

	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
