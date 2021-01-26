;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_STR_EL
.else
.set DEF_C5_LOG_STR_EL = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_str.inc"

;--------------------------------------------------------
C5_LOG_STR_EL:
;--------------------------------------------------------
;Логирование элемента массива строк. Конец каждого
;элемента определяется по 0x00
;IN: Y - адрес на массив строк (15-ый бит false - RAM,
;true - ROM), TEMP-номер элемента
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z
	PUSH LOOP_CNTR

	MOV ZH,YH
	MOV ZL,YL
	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80

	CPI TEMP,0x00
	BREQ _C5_LOG_STR_EL__LOG_STR
	LDI LOOP_CNTR,TEMP
_C5_LOG_STR_EL__LOOP:
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000

	CPI TEMP,0x00
	BRNE _C5_LOG_STR_EL__LOOP
	DEC LOOP_CNTR
	BRNE _C5_LOG_STR_EL__LOOP

_C5_LOG_STR_EL__LOG_STR:
	POP LOOP_CNTR
	MJMP _C5_LOG_STR__LOG_STR_EL_CODEPOINT
.endif
.endif
