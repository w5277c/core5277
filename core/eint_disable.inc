;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.01.2021  w5277c@gmail.com			Начало
;26.02.2021  w5277c@gmail.com			EIMSK!
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_EINT_DISABLE
.else
.set DEF_C5_EINT_DISABLE = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_EINT_DISABLE"
.endif

;--------------------------------------------------------
C5_EINT_DISABLE:
;--------------------------------------------------------
;Выключение внешних прерываний
;IN: TEMP_H-прерывание (C5_IR_INTn)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	MOV ACCUM,TEMP_H
	CLR TEMP_H
	CLR TEMP_L
	MCALL C5_IR_VECTOR_SET

	LDI_Z (INT_TABLE*2)-C5_IR_INT0
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ACCUM,Z
	COM ACCUM
	LDS TEMP_H,EIMSK
	AND TEMP_H,ACCUM
	STS EIMSK,TEMP_H

	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
.endif