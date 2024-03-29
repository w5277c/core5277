;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;04.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FLAG_RESET
.else
.set DEF_C5_FLAG_RESET = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_FLAG_RESET"
.endif

.include	"./core/_flag_offset.inc"
.include	"./conv/bitnum_to_num.inc"

;--------------------------------------------------------
C5_FLAG_RESET:
;--------------------------------------------------------
;Сброс флага
;IN:TEMP - номер флага(0-63)
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP

	MCALL _C5_FLAG_OFFSET
	ANDI TEMP,0x07
	MCALL BITNUM_TO_NUM
	LD ACCUM,Z
	COM TEMP
	AND ACCUM,TEMP
	ST Z,ACCUM

	POP TEMP
	POP ACCUM
	POP_Z
	RET
.endif
