;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;03.10.2020	w5277c@gmail.com			Атомарность
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;18.08.2021	w5277c@gmail.com			Ничего не делаем, если порт не задан(0xff)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_SET_HI
.else
.set DEF_PORT_SET_HI = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_SET_HI"
.endif

.include	"./io/port_offsets.inc"

;--------------------------------------------------------
PORT_SET_HI:
;--------------------------------------------------------
;Устанвливаем порту высокий уровень
;IN: ACCUM-порт
;--------------------------------------------------------
	CPI ACCUM,0xff
	BRNE PC+0x02
	RET

	PUSH_Z
	PUSH_THL
	PUSH TEMP
	PUSH ACCUM

	MCALL PORT_OFFSETS
	MOV ZH,C0x00
	MOV ZL,TEMP_H

	LDS TEMP_L,SREG
	CLI
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	STS SREG,TEMP_L

	POP ACCUM
	POP TEMP
	POP_THL
	POP_Z
	RET
.endif
