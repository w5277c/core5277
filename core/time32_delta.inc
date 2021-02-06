;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIME32_DELTA
.else
.set DEF_C5_TIME32_DELTA = 1

.include	"./core/time32_mark.inc"

;--------------------------------------------------------
C5_TIME32_DELTA:
;--------------------------------------------------------
;Считаем дельту между текущей временной меткой и
;записанной в выделенной паямти (4 байта)
;IN: PID,TEMP-смещение на выделенные 4 байта, хранящие
;временную метку
;OUT: Дельта будет записана поверх временной метки в
;выделенную память
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	MCALL C5_RAM_OFFSET
	ADD ZL,TEMP
	CLR TEMP_EH
	ADC ZH,TEMP_EH

	LDD TEMP_EH,Z+0x00
	LDD TEMP_EL,Z+0x01
	LDD TEMP_H,Z+0x02
	LDD TEMP_L,Z+0x03

	;Используем TEMP!
	MCALL C5_TIME32_MARK

	LDD TEMP,Z+0x03
	SUB TEMP,TEMP_L
	STD Z+0x03,TEMP
	LDD TEMP,Z+0x02
	SUB TEMP,TEMP_H
	STD Z+0x02,TEMP
	LDD TEMP,Z+0x01
	SUB TEMP,TEMP_EL
	STD Z+0x01,TEMP
	LDD TEMP,Z+0x00
	SUB TEMP,TEMP_EH
	STD Z+0x00,TEMP

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	RET
.endif
