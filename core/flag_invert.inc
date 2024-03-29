;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;04.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;11.06.2021	w5277c@gmail.com			Атомарность
;07.08.2023	konstantin@5277.ru		Замена флага Z на C
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
	BRCS PC+0x01+_MCALL_SIZE+0x01
	MCALL C5_FLAG_SET
	RJMP PC+0x01+_MCALL_SIZE
	MCALL C5_FLAG_RESET

	STS SREG,ACCUM
	POP ACCUM
	RET
.endif
