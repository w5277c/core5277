;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_STRN
.else
.set DEF_C5_LOG_STRN = 1
.ifdef LOGGING_PORT

.include	"./core/log/log_char.inc"

;--------------------------------------------------------
C5_LOG_STRN:
;--------------------------------------------------------
;Логирование строки с указанной длинной
;IN: Y - адрес на строку теста, TEMP-длина
;(15-ый бит false - RAM, true - ROM)
;--------------------------------------------------------
	PUSH LOOP_CNTR
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

	MOV LOOP_CNTR,TEMP
_C5_LOG_STRN__BYTES_LOOP:
	;Считываем байт
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	CPI TEMP,0x00
	BREQ _C5_LOG_STRN__END

	MCALL C5_LOG_CHAR
	DEC LOOP_CNTR
	BRNE _C5_LOG_STRN__BYTES_LOOP

_C5_LOG_STRN__END:
	POP ZL
	POP ZH
	POP TEMP
	POP LOOP_CNTR
	RET
.endif
.endif
