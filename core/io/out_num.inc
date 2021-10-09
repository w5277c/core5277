;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.10.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;26.01.2021	w5277c@gmail.com			Не выводим лишние нули
;03.07.2021	w5277c@gmail.com			Багфикс
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_NUM
.else
.set DEF_C5_OUT_NUM = 1
.ifdef LOGGING_PORT

.include	"./math/div16x8.inc"
.include	"./math/div10.inc"
.include	"./math/mul10.inc"
.include	"./math/mul100.inc"
.include	"./core/io/out_char.inc"

;--------------------------------------------------------
C5_OUT_NUM:
;--------------------------------------------------------
;Логирование десятиричного числа(1байт)
;IN: TEMP-байт
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_FT
	PUSH FLAGS

	LDI FLAGS,0x00
	MOV ACCUM,TEMP
	LDI TEMP_H,0x00
	MOV TEMP_L,ACCUM
	LDI TEMP,0x64
	MCALL DIV16x8
	OR FLAGS,TEMP_L
	BREQ PC+0x03+_MCALL_SIZE
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_OUT_CHAR

	MOV TEMP,TEMP_L
	MCALL MUL100
	SUB ACCUM,TEMP_L
	MOV TEMP,ACCUM
	MCALL DIV10
	OR FLAGS,TEMP_L
	BREQ PC+0x03+_MCALL_SIZE
	MOV TEMP,TEMP_L
	SUBI TEMP,(0x100-0x30)											;+0x30
	MCALL C5_OUT_CHAR

	MOV TEMP,TEMP_L
	MCALL MUL10
	SUB ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x30)										;+0x30
	MOV TEMP,ACCUM
	MCALL C5_OUT_CHAR

	POP FLAGS
	POP_FT
	POP ACCUM
	RET
.endif
.endif
