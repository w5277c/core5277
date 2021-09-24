;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.11.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_UPTIME_COPY
.else
.set DEF_C5_UPTIME_COPY = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_UPTIME_COPY"
.endif

;--------------------------------------------------------
C5_UPTIME_COPY:
;--------------------------------------------------------
;Записываем в память 5 байт UPTIME
;IN: Y-адрес для записи
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	PUSH TEMP
	CLI
	LDS TEMP,_C5_UPTIME+0x00
	STD Y+0x00,TEMP
	LDS TEMP,_C5_UPTIME+0x01
	STD Y+0x01,TEMP
	LDS TEMP,_C5_UPTIME+0x02
	STD Y+0x02,TEMP
	LDS TEMP,_C5_UPTIME+0x03
	STD Y+0x03,TEMP
	LDS TEMP,_C5_UPTIME+0x04
	STD Y+0x04,TEMP
	POP TEMP
	STS SREG,TEMP
	POP TEMP
	RET
.endif
