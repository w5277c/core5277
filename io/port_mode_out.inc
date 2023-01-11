;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;03.10.2020	w5277c@gmail.com			Атомарность
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;18.08.2021	w5277c@gmail.com			Ничего не делаем, если порт не задан(0xff)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_MODE_OUT
.else
.set DEF_PORT_MODE_OUT = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_MODE_OUT"
.endif

.include	"./io/port_offsets.inc"

;--------------------------------------------------------
PORT_MODE_OUT:
;--------------------------------------------------------
;Устанвливаем порт на выход
;IN: ACCUM-порт
;--------------------------------------------------------
	CPI ACCUM,0xff
	BRNE PC+0x02
	RET

	PUSH_Z
	PUSH_T16
	PUSH TEMP
	PUSH ACCUM

	MCALL PORT_OFFSETS
	MOV ZH,C0x00
	MOV ZL,TEMP_L

	LDS TEMP_L,SREG
	CLI
	LD TEMP,Z
	OR TEMP,ACCUM
	ST Z,TEMP
	STS SREG,TEMP_L

	POP ACCUM
	POP TEMP
	POP_T16
	POP_Z
	RET
.endif
