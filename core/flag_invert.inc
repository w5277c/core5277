;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;04.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;11.06.2021	w5277c@gmail.com			Атомарность
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FLAG_INVERT
.else
.set DEF_C5_FLAG_INVERT = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_FLAG_INVERT"
.endif

.include	"./inc/core/flag_get.inc"
.include	"./inc/core/flag_set.inc"
.include	"./inc/core/flag_reset.inc"

;--------------------------------------------------------
C5_FLAG_INVERT:
;--------------------------------------------------------
;Инвертирование флага
;IN:TEMP - номер флага(0-63)
;--------------------------------------------------------
	PUSH ACCUM
	LDS ACCUM,SREG
	CLI

	MCALL C5_FLAG_GET
	BRNE PC+0x04
	MCALL C5_FLAG_SET
	RJMP PC+0x03
	MCALL C5_FLAG_RESET

	STS SREG,ACCUM
	POP ACCUM
	RET
.endif
