;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.01.2021	w5277c@gmail.com			Начало
;12.02.2021	w5277c@gmail.com			Исправлены явные косяки
;26.02.2021	w5277c@gmail.com			EIMSK,EIFR!
;20.06.2021	w5277c@gmail.com			Устранено ложное срабатывание при вклчючении
;15.04.2023	konstantin@5277.ru		Обнулялись правые биты EICRA
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_EINT_ENABLE
.else
.set DEF_C5_EINT_ENABLE = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_EINT_ENABLE"
.endif

;TODO запретить прерывания?
;TODO привести к виду EIMSK,EIFR

;--------------------------------------------------------
C5_EINT_ENABLE:
;--------------------------------------------------------
;Включение внешних прерываний
;IN: PID-ид задачи/драйвера,
;TEMP_H-прерывание (C5_IR_INTn)
;TEMP_L-тип реакции(C5_ISC_...)
;Y-адрес обработчика прерывания
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_Z

	SBRS TEMP_H,0x02
	LDS TEMP,EICRA
.ifdef EICRB
	SBRC TEMP_H,0x02
	LDS TEMP,EICRB
.endif

	PUSH TEMP_H
	ANDI TEMP_L,0x03

	SUBI TEMP_H,C5_IR_INT0
.if DEVICE_FAMILY == ATMEGA16
	;Не поддерживаются ISC
	CPI TEMP_H,C5_IR_INT2-C5_IR_INT0
	BREQ _C5_EINT_ENABLE__ISC_SET_SKIP
.endif
	ANDI TEMP_H,0x03
	BREQ _C5_EINT_ENABLE__LSL_SKIP

	MOV LOOP_CNTR,TEMP_H
	LDI TEMP_H,0x03
_C5_EINT_ENABLE__LOOP:
	LSL TEMP_H
	LSL TEMP_H
	LSL TEMP_L
	LSL TEMP_L
	DEC LOOP_CNTR
	BRNE _C5_EINT_ENABLE__LOOP
	COM TEMP_H

_C5_EINT_ENABLE__LSL_SKIP:
	AND TEMP,TEMP_H
	OR TEMP,TEMP_L
_C5_EINT_ENABLE__ISC_SET_SKIP:
	POP ACCUM
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	MCALL C5_IR_VECTOR_SET

	SBRS ACCUM,0x02
	STS EICRA,TEMP

.ifdef EICRB
	SBRC ACCUM,0x02
	STS EICRB,TEMP
.endif

	LDI_Z (INT_TABLE*2)-C5_IR_INT0
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ACCUM,Z
	LDS TEMP,EIFR
	OR TEMP,ACCUM
	STS EIFR,TEMP
	LDS TEMP,EIMSK
	OR TEMP,ACCUM
	STS EIMSK,TEMP

	POP_Z
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET
.endif