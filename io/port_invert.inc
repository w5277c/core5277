;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;03.10.2020	w5277c@gmail.com			Оптимизация, атомарность
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_INVERT
.else
.set DEF_PORT_INVERT = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_INVERT"
.endif

.include	"./io/port_offsets.inc"

;--------------------------------------------------------
PORT_INVERT:
;--------------------------------------------------------
;Инвертирование состояния порта
;IN: ACCUM адрес регистра PORTx
;--------------------------------------------------------;
	PUSH_Z
	PUSH_T16
	PUSH TEMP
	PUSH ACCUM

	MCALL PORT_OFFSETS

	LDS TEMP_L,SREG
	CLI
	MOV ZH,C0x00
	MOV ZL,TEMP_H
	LD TEMP,Z
	EOR TEMP,ACCUM
	ST Z,TEMP
	STS SREG,TEMP_L

	POP ACCUM
	POP TEMP
	POP_T16
	POP_Z
	RET
.endif
