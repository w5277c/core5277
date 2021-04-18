;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;18.04.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_PCINT_F
.else
.set DEF__C5_DRV_PCINT_F = 1
.message "Included driver hardware fast PCINT v0.1"

.include	"./conv/bitnum_to_num.inc"

	;---OPERATIONS---
	.EQU	DRV_PCINT_F_OP_SET								= 0x00;Устанавливаем обработчик
	.EQU	DRV_PCINT_F_OP_ENABLE							= 0x01;Включаем обработчик
	.EQU	DRV_PCINT_F_OP_DISABLE							= 0x02;Выключаем обработчик

;--------------------------------------------------------
DRV_PCINT_F_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
	;Завершаем инициализацию
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, устанавливаем обработчики
;IN: FLAGS-операция
;--------------------------------------------------------
	CPI FLAGS,DRV_PCINT_F_OP_SET
	BREQ _DRV_PCINT_F__SET
	CPI FLAGS,DRV_PCINT_F_OP_ENABLE
	BREQ _DRV_PCINT_F__ENABLE
	CPI FLAGS,DRV_PCINT_F_OP_DISABLE
	BREQ _DRV_PCINT_F__DISABLE
	RET

;--------------------------------------------------------
_DRV_PCINT_F__SET:
;--------------------------------------------------------
;Добавляем обработчик
;IN: ACCUM-номер порта(A=0,B=1,...),
;TEMP_H-PCMSK, TEMP_L-PID, Y-адрес обработчика
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM
	PUSH PID

	;Получаем адрес регистра PCINTx
	LDI ZH,high(PCINT_TABLE*2)
	LDI ZL,low(PCINT_TABLE*2)
;	MOV TEMP,ACCUM
	ADD ZL,ACCUM
	CLR TEMP
	ADC ZH,TEMP
	LPM TEMP,Z
	MOV ZL,TEMP
	LDI ZH,0x00
	;Применяем PCMSKx
	ST Z,TEMP_H
	;Устанавливаем адрес прерывания
	MOV PID,TEMP_L
	LDI TEMP,C5_IR_PCINT0-0x01										;Костыль, порт A может иметь PCINT?
	ADD ACCUM,TEMP
	MOV TEMP_H,YH
	MOV TEMP_L,YL
	MCALL C5_IR_VECTOR_SET

	POP PID
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_PCINT_F__ENABLE:
;--------------------------------------------------------
;Включаем обработчик порта
;IN: ACCUM-номер порта(0...)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	MOV TEMP,ACCUM
	MCALL BITNUM_TO_NUM
	LDS ACCUM,PCICR
	OR ACCUM,TEMP
	STS PCICR,ACCUM

	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_PCINT_F__DISABLE:
;--------------------------------------------------------
;Выключаем обработчик порта
;IN: ACCUM-номер порта(0...)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	MOV TEMP,ACCUM
	MCALL BITNUM_TO_NUM
	COM TEMP
	LDS ACCUM,PCICR
	AND ACCUM,TEMP
	STS PCICR,ACCUM

	POP ACCUM
	POP TEMP
	RET
