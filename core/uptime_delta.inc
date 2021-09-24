;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;13.09.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_UPTIME_DELTA
.else
.set DEF_C5_UPTIME_DELTA = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_UPTIME_DELTA"
.endif

.include "./core/uptime_get.inc"

;--------------------------------------------------------
C5_UPTIME_DELTA:
;--------------------------------------------------------
;Вычисляем разницу между текущим UPTIME и UPTIME в
;переданных регистрах
;IN: Y-адрес значения UPTIME
;OUT: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L,TEMP - UPTIME дельта
;--------------------------------------------------------
	PUSH ACCUM

	MCALL C5_UPTIME_GET

	LDD ACCUM,Y+0x04
	SUB TEMP,ACCUM
	LDD ACCUM,Y+0x03
	SBC TEMP_L,ACCUM
	LDD ACCUM,Y+0x02
	SBC TEMP_H,ACCUM
	LDD ACCUM,Y+0x01
	SBC TEMP_EL,ACCUM
	LDD ACCUM,Y+0x00
	SBC TEMP_EH,ACCUM

	POP ACCUM
	RET
.endif
