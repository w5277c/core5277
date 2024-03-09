;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.02.2024	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Грубое вычисление разницы в минутах между двумя UPTIME, при максимальной дельте в 65535 минут(~45 дней),
;ошибка составит 106 мин.
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_UPTIME_MLDELTA
.else
.set DEF_UPTIME_MLDELTA = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_UPTIME_MLDELTA"
.endif

.include	"./core/uptime_delta.inc"
.include	"./math/div32x16.inc"

;--------------------------------------------------------
C5_UPTIME_MLDELTA:
;--------------------------------------------------------
;Вычисляем разницу, в минутах, между текущим UPTIME и
;UPTIME в RAM
;IN: Y-адрес значения UPTIME
;OUT: TEMP_H/L-UPTIME дельта в минутах(0xffff-
;переполнение)
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP_EH
	PUSH TEMP_EL

	MCALL C5_UPTIME_DELTA

	LDI_X 117															;Грубо, должно быть 117,1875
	MCALL DIV32X16
	CPI TEMP_EH,0x00
	BRNE _C5_UPTIME_MLDELTA__OVERFLOW
	CPI TEMP_EL,0x00
	BREQ _C5_UPTIME_MLDELTA__END
_C5_UPTIME_MLDELTA__OVERFLOW:
	LDI TEMP_H,0xff
	LDI TEMP_L,0xff

_C5_UPTIME_MLDELTA__END:
	POP TEMP_EL
	POP TEMP_EH
	POP_X
	RET
.endif
