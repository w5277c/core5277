;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;19.12.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_TIMESTAMP_TO_DT
.else
.set DEF_TIMESTAMP_TO_DT = 1

.if REPORT_INCLUDES == 0x01
.message "included TIMESTAMP_TO_DT"
.endif

.include	"./math/div32x16.inc"

;--------------------------------------------------------
TIMESTAMP_TO_DT:
;--------------------------------------------------------
;Конвертация unix timestamp в дату и время
;IN: TEMP_EH/EL/H,L-unix timestamp
;OUT: TEMP_EH-год(0=2000), TEMP_EL-месяц(1-12),
;TEMP-день месяца(1-31), TEMP_H-часы(0-23),
;TEMP_L-минуты(0-59), XH-секунды(0-59),
;XL-день недели(1-7)
;--------------------------------------------------------
	PUSH TEMP

	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	LDI_X 60
	MCALL DIV32X16
	PUSH XL																;sec
	LDI_X 60
	MCALL DIV32X16
	PUSH XL																;min
	LDI_X 24
	MCALL DIV32X16
	PUSH XL																;hour

	MOV YH,TEMP_H
	MOV YL,TEMP_L
	LDI TEMP_H,high(1970)
	LDI TEMP_L,low(1970)
_TIMESTAMP_TO_DT__YEAR_COMPUTE:
	MOV ZH,TEMP_H
	MOV ZL,TEMP_L
	CPI YH,high(365)
	BRCS _TIMESTAMP_TO_DT__MONTH_COMPUTE
	BRNE PC+0x03
	CPI YL,low(365)
	BRCS _TIMESTAMP_TO_DT__MONTH_COMPUTE
	;DAYS	 >= 365
	LDI_X 400
	MCALL DIV32X16
	CPI XH,0x00
	BRNE PC+0x03
	CPI XL,0x00
	BREQ _TIMESTAMP_TO_DT__LEAP_YEAR
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	LDI_X 4
	MCALL DIV32X16
	CPI XL,0x00
	BRNE _TIMESTAMP_TO_DT__NORMAL_YEAR
	MOV TEMP_H,ZH
	MOV TEMP_L,ZL
	LDI_X 100
	MCALL DIV32X16
	CPI XL,0x00
	BREQ _TIMESTAMP_TO_DT__NORMAL_YEAR
_TIMESTAMP_TO_DT__LEAP_YEAR:
	SUBI TEMP_L,0x01
	SBCI TEMP_H,C0x00
_TIMESTAMP_TO_DT__NORMAL_YEAR:
	SUBI TEMP_L,low(365)
	SBCI TEMP_H,high(365)
	ADIW ZL,0x01
	RJMP _TIMESTAMP_TO_DT__YEAR_COMPUTE
	MOV TEMP,ZL
	SBIW TEMP,(2000-1970)
	PUSH TEMP														;year

_TIMESTAMP_TO_DT__MONTH_COMPUTE
	ADIW ZL,0x01


_WORD_TO_SDNF__END:
	POP TEMP
	RET
.endif
