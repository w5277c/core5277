;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.11.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_UPTIME_GET
.else
.set DEF_C5_UPTIME_GET = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_UPTIME_GET"
.endif

;--------------------------------------------------------
C5_UPTIME_GET:
;--------------------------------------------------------
;Помещаем в регистры 5 байт UPTIME
;OUT: TEMP_EH,TEMP_EL,TEMP_H,TEMP_L,TEMP-UPTIME
;--------------------------------------------------------
	PUSH ACCUM
	LDS ACCUM,SREG
	CLI
	LDS TEMP_EH,_C5_UPTIME+0x00
	LDS TEMP_EL,_C5_UPTIME+0x01
	LDS TEMP_H,_C5_UPTIME+0x02
	LDS TEMP_L,_C5_UPTIME+0x03
	LDS TEMP,_C5_UPTIME+0x04
	STS SREG,ACCUM
	POP ACCUM
	RET
.endif
