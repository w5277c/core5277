;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIME_DELTA
.else
.set DEF_C5_TIME_DELTA = 1

.include	"./core/uptime_get.inc"

;--------------------------------------------------------
C5_TIME_DELTA:
;--------------------------------------------------------
;Считаем дельту между текущей временной меткой и
;записанной в выделенной паямти (7 байт)
;IN: PID,TEMP-смещение на выделенные 7 байт, хранящие
;временную метку
;OUT: Дельта будет записана поверх временной метки в
;выделенную память
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_L
	PUSH TEMP_H
	PUSH TEMP_EL
	PUSH TEMP_EH

	MCALL C5_RAM_OFFSET
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP

	LDS TEMP,SREG
	PUSH TEMP
	CLI

	MCALL C5_UPTIME_GET

	PUSH TEMP
	LDD ACCUM,Z+0x05
	LDS TEMP,_C5_MAIN_TIMER_CNTR
	SUB TEMP,ACCUM
	STD Z+0x05,TEMP

	LDD ACCUM,Z+0x06
	_C5_TIMER_TCNT TEMP
	SUB TEMP,ACCUM
	STD Z+0x06,TEMP
	POP TEMP

	LDD ACCUM,Z+0x04
	SUB TEMP,ACCUM
	STD Z+0x04,TEMP
	LDD ACCUM,Z+0x03
	SBC TEMP_L,ACCUM
	STD Z+0x03,TEMP_L
	LDD ACCUM,Z+0x02
	SBC TEMP_H,ACCUM
	STD Z+0x02,TEMP_H
	LDD ACCUM,Z+0x01
	SBC TEMP_EL,ACCUM
	STD Z+0x01,TEMP_EL
	LDD ACCUM,Z+0x00
	SBC TEMP_EH,ACCUM
	STD Z+0x00,TEMP_EH

	POP TEMP
	STS SREG,TEMP

	POP TEMP_EH
	POP TEMP_EL
	POP TEMP_H
	POP TEMP_L
	POP TEMP
	POP ACCUM
	POP_Z
	RET
