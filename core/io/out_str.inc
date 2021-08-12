;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com        Начало
;01.08.2020	w5277c@gmail.com        Разбиение на файлы
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_STR
.else
.set DEF_C5_OUT_STR = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_char.inc"

;--------------------------------------------------------
C5_OUT_STR:
;--------------------------------------------------------
;Логирование строки, конец определяется по 0x00
;IN: Z-адрес на строку(15-ый бит false-RAM, true-ROM)
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z
	PUSH ACCUM

	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	MOV ACCUM,ZH
	ANDI ZH,0x7f
_C5_OUT_STR__LOG_STR_EL_CODEPOINT:
_C5_OUT_STR__BYTES_LOOP:
	;Считываем байт
	SBRS ACCUM,0x07
	LD TEMP,Z+
	SBRC ACCUM,0x07
	LPM TEMP,Z+
	CPI TEMP,0x00
	BREQ _C5_OUT_STR__END
	MCALL C5_OUT_CHAR
	RJMP _C5_OUT_STR__BYTES_LOOP

_C5_OUT_STR__END:
	POP ACCUM
	POP_Z
	POP TEMP
	RET
.endif
.endif
