;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_BYTE_TO_STR
.else
.set DEF_C5_BYTE_TO_STR = 1

.include	"./inc/math/div16x8.inc"
.include	"./inc/math/mul24x8.inc"

;--------------------------------------------------------
C5_BYTE_TO_STR:
;--------------------------------------------------------
;Конвертирование числа в 3 символа
;IN: TEMP - число
;OUT: TEMP_H,TEMP_L,TEMP - 3 ASCII символа(0,0x30-0x39)
;-START--------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL

	MOV ACCUM,TEMP
	LDI TEMP_H,0x00
	MOV TEMP_L,ACCUM
	LDI TEMP,0x64
	MCALL C5_DIV16x8
	SUBI TEMP_L,(0x100-0x30)										;+0x30
	PUSH TEMP_L
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	SUBI TEMP_L,0x30
	LDI TEMP,0x64
	MCALL C5_MUL24x8
	SUB ACCUM,TEMP_L

	LDI TEMP_H,0x00
	MOV TEMP_L,ACCUM
	LDI TEMP,0x0a
	MCALL C5_DIV16x8
	BRTS PC+0x03
	SUBI TEMP_L,(0x100-0x30)										;+0x30
	PUSH TEMP_L
	LDI TEMP_EL,0x00
	LDI TEMP_H,0x00
	SUBI TEMP_L,0x30
	LDI TEMP,0x0a
	MCALL C5_MUL24x8
	SUB ACCUM,TEMP_L
	SUBI ACCUM,(0x100-0x30)										;+0x30

	MOV TEMP,ACCUM
	POP TEMP_L
	POP TEMP_H

	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	RET
.endif
