;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.01.2021	w5277c@gmail.com			Начало, первый успешный тест
;-----------------------------------------------------------------------------------------------------------------------
;На основе https://howardhinnant.github.io/date_algorithms.html
;-----------------------------------------------------------------------------------------------------------------------
;TODO: Добавить проверки на корректность диапазона, проверсти тщательное тестирование и оптимизацию
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DT_TO_TIMESTAMP
.else
.set DEF_DT_TO_TIMESTAMP = 1

.if REPORT_INCLUDES == 0x01
.message "included DT_TO_TIMESTAMP"
.endif

.include	"./math/mul32x16.inc"
.include	"./math/mul16x8.inc"
.include	"./math/div16x8.inc"

;--------------------------------------------------------
DT_TO_TIMESTAMP:
;--------------------------------------------------------
;Конвертация unix timestamp в дату и время(рабочий
;диапазон 01.03.2000 00:00:00-31.12.2099 23:59:59)
;IN: TEMP_EH-год(0=2000), TEMP_EL-месяц(1-12),
;TEMP-день месяца(1-31), TEMP_H-часы(0-23),
;TEMP_L-минуты(0-59), XH-секунды(0-59),
;OUT: TEMP_EH/EL/H/L-unix timestamp(0x00000000-ошибка)
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH FLAGS
	PUSH TEMP

	MOV ACCUM,TEMP_EH
	MOV FLAGS,TEMP_EL
	PUSH TEMP
	PUSH XH
	PUSH TEMP_L
	PUSH TEMP_H

	;Вычисляем время
	CLR TEMP_EH
	CLR TEMP_EL
	CLR TEMP_H
	POP TEMP_L
	LDI_X 60
	MCALL MUL32X16
	POP TEMP
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	LDI_X 60
	MCALL MUL32X16
	POP TEMP
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	ADC TEMP_EL,C0x00
	MOV YH,TEMP_EH
	MOV YL,TEMP_EL
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L

	;Вычисляем день в году(0-365)
	CPI FLAGS,0x03
	BRCC PC+0x03
	DEC ACCUM																;m<=2
	SUBI FLAGS,(0x100-0x0c)
	SUBI FLAGS,0x03
	CLR TEMP_H
	MOV TEMP_L,FLAGS
	LDI TEMP,153
	MCALL MUL16X8
	LDI TEMP,0x02
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	LDI TEMP,0x05
	MCALL DIV16X8
	POP TEMP
	DEC TEMP
	ADD TEMP_L,TEMP
	ADC TEMP_H,C0x00
	PUSH TEMP_H
	PUSH TEMP_L

	CLR TEMP_EH
	CLR TEMP_EL
	CLR TEMP_H
	MOV TEMP_L,ACCUM
	LDI_X 365
	MCALL MUL32X16
	POP TEMP
	ADD TEMP_L,TEMP
	POP TEMP
	ADC TEMP_H,TEMP
	ADC TEMP_EL,C0x00
	ADC TEMP_EH,C0x00

	PUSH TEMP_H
	PUSH TEMP_L
	CLR TEMP_H
	MOV TEMP_L,ACCUM
	LDI TEMP,0x04
	MCALL DIV16X8
	POP TEMP
	ADD TEMP_L,TEMP
	POP TEMP
	ADC TEMP_H,TEMP
	ADC TEMP_EL,C0x00
	ADC TEMP_EH,C0x00

	LDI_X (86400/2)
	MCALL MUL32X16
	LSL TEMP_L
	ROL TEMP_H
	ROL TEMP_EL
	ROL TEMP_EH
	ADD ZL,TEMP_L
	ADC ZH,TEMP_H
	ADC YL,TEMP_EL
	ADC YH,TEMP_EH

	LDI TEMP_L,(_TIMESTAMP_TO_DT_01032000>>0x00 & 0xff)
	ADD TEMP_L,ZL
	LDI TEMP_H,(_TIMESTAMP_TO_DT_01032000>>0x08 & 0xff)
	ADC TEMP_H,ZH
	LDI TEMP_EL,(_TIMESTAMP_TO_DT_01032000>>0x10 & 0xff)
	ADC TEMP_EL,YL
	LDI TEMP_EH,(_TIMESTAMP_TO_DT_01032000>>0x18 & 0xff)
	ADC TEMP_EH,YH

	POP TEMP
	POP FLAGS
	POP ACCUM
	POP_Z
	POP_Y
	POP_X
	RET
.endif
