;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.01.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DT_TO_STR
.else
.set DEF_DT_TO_STR = 1

.if REPORT_INCLUDES == 0x01
.message "included DT_TO_STR"
.endif

.include	"./math/div16x8.inc"

;--------------------------------------------------------
DT_TO_STR:
;--------------------------------------------------------
;Конвертирование даты/времени в строку(hh.mm.ss DD.MM.YY)
;IN: TEMP_EH-год(0=2000), TEMP_EL-месяц(1-12),
;TEMP-день месяца(1-31), TEMP_H-часы(0-23),
;TEMP_L-минуты(0-59), XH-секунды(0-59),
;Z-dst адрес
;OUT: Z-новое смещение
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	PUSH TEMP
	PUSH TEMP_L
	MOV TEMP_L,TEMP_H
	CLR TEMP_H
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	LDI TEMP,':'
	ST Z+,TEMP
	POP TEMP_L
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	LDI TEMP,':'
	ST Z+,TEMP
	MOV TEMP_L,XH
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	LDI TEMP,' '
	ST Z+,TEMP

	POP TEMP_L
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	LDI TEMP,'.'
	ST Z+,TEMP
	MOV TEMP_L,TEMP_EL
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP
	LDI TEMP,'.'
	ST Z+,TEMP

	MOV TEMP_L,TEMP_EH
	LDI TEMP,0x0a
	MCALL DIV16X8
	SUBI TEMP_L,(0x100-0x30)
	ST Z+,TEMP_L
	SUBI TEMP,(0x100-0x30)
	ST Z+,TEMP

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
