;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;03.11.2024	konstantin@5277.ru		Оптимизация(flash,cpu)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_GET_BYTE
.else
.set DEF_PORT_GET_BYTE = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_GET_BYTE"
.endif

.include	"./io/_port_offset.inc"

;--------------------------------------------------------
PORT_GET_BYTE:
;--------------------------------------------------------
;Считывание состояния порта(8 бит)
;IN: ACCUM - порт
;OUT: ACCUM - значение
;--------------------------------------------------------
	CPI ACCUM,0xff
	BRNE PC+0x02
	RET

	PUSH_Z
	PUSH TEMP
	MCALL _PORT_OFFSET
	ADIW ZL,0x02
	LPM TEMP,Z
.if VPORTS>0x00
	CPI ZL,VPORTS_ADDR
	BRCS PC+0x01+_MCALL_SIZE
	MCALL VPORT_GET
.endif
	LD ACCUM,Z
	POP TEMP
	POP_Z
	RET
.endif
