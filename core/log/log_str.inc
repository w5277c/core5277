;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_STR
.else
.set DEF_C5_LOG_STR = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_STR:
;--------------------------------------------------------
;Логирование строки, конец определяется по 0x00
;IN: Y - адрес на строку теста
;(15-ый бит false - RAM, true - ROM)
;--------------------------------------------------------
	PUSH TEMP
	PUSH ZH
	PUSH ZL

	MOV ZH,YH
	MOV ZL,YL
	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80

_C5_LOG_STR__LOG_STR_EL_CODEPOINT:
_C5_LOG_STR__BYTES_LOOP:
	;Считываем байт
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	CPI TEMP,0x00
	BREQ _C5_LOG_STR__END

	MCALL C5_LOG_CHAR
	RJMP _C5_LOG_STR__BYTES_LOOP

_C5_LOG_STR__END:
	POP ZL
	POP ZH
	POP TEMP
	RET
.endif
.endif
