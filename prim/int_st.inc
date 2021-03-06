;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_INT_ST
.else
.set DEF_INT_ST = 1

.if REPORT_INCLUDES == 0x01
.message "included INT_ST"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
INT_ST:
;--------------------------------------------------------
;Чтение значения из INT
;IN: Z-адрес на память с переменными, TEMP-смещение для
;записи значения
;OUT: TEMP_EH/EL/H/L-значение
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z

	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	LDD TEMP_EH,Z+0x00
	LDD TEMP_EL,Z+0x01
	LDD TEMP_H,Z+0x02
	LDD TEMP_L,Z+0x03

	POP_Z
	POP TEMP
	RET
.endif
