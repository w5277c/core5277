;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;09.01.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_BYTE_TO_BCD
.else
.set DEF_BYTE_TO_BCD = 1

.if REPORT_INCLUDES == 0x01
.message "included BYTE_TO_BCD"
.endif

.include "./math/mul10.inc"
.include "./math/div10.inc"

;--------------------------------------------------------
BYTE_TO_BCD:
;--------------------------------------------------------
;Конвертация байта в BCD число
;IN: TEMP-байт(диапазон значений 0-99)
;OUT: TEMP-BCD число
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	MCALL DIV10
	MOV TEMP_H,TEMP_L
	PUSH TEMP
	MOV TEMP,TEMP_H
	SWAP TEMP_H
	PUSH TEMP_H
	MCALL MUL10
	POP TEMP_H
	POP TEMP
	SUB TEMP,TEMP_L
	OR TEMP,TEMP_H
	
	POP TEMP_L
	POP TEMP_H
	RET
.endif
