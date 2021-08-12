;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;27.01.2021	w5277c@gmail.com			Начало
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_STR_EL
.else
.set DEF_C5_OUT_STR_EL = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_str.inc"

;--------------------------------------------------------
C5_OUT_STR_EL:
;--------------------------------------------------------
;Логирование элемента массива строк. Конец каждого
;элемента определяется по 0x00
;IN: Z-адрес на массив строк (15-ый бит false-RAM,
;true-ROM), TEMP-номер элемента
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z
	PUSH ACCUM
	PUSH LOOP_CNTR

	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	MOV ACCUM,ZH
	ANDI ZH,0x7f

	CPI TEMP,0x00
	BREQ _C5_OUT_STR_EL__LOG_STR
	MOV LOOP_CNTR,TEMP
_C5_OUT_STR_EL__LOOP:
	SBRS ACCUM,0x07
	LD TEMP,Z+
	SBRC ACCUM,0x07
	LPM TEMP,Z+
	CPI TEMP,0x00
	BRNE _C5_OUT_STR_EL__LOOP
	DEC LOOP_CNTR
	BRNE _C5_OUT_STR_EL__LOOP

_C5_OUT_STR_EL__LOG_STR:
	POP LOOP_CNTR
	MJMP _C5_OUT_STR__LOG_STR_EL_CODEPOINT
.endif
.endif
