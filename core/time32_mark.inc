;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;02.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_TIME32_MARK
.else
.set DEF_C5_TIME32_MARK = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_TIME32_MARK"
.endif

.include	"./core/uptime_copy.inc"

;--------------------------------------------------------
C5_TIME32_MARK:
;--------------------------------------------------------
;Записываем в выделенную память временную метку (4 байт)
;IN: PID,TEMP-смещение на выделенные 4 байт, для хранения
;временной метки
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MCALL C5_RAM_OFFSET
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	LDS TEMP,SREG
	PUSH TEMP
	CLI

	LDS TEMP,_C5_UPTIME+0x02
	STD Z+0x00,TEMP
	LDS TEMP,_C5_UPTIME+0x03
	STD Z+0x01,TEMP
	LDS TEMP,_C5_UPTIME+0x04
	STD Z+0x02,TEMP
	LDS TEMP,_C5_MAIN_TIMER_CNTR
	STD Z+0x03,TEMP

	POP TEMP
	STS SREG,TEMP

	POP TEMP
	POP_Z
	RET
.endif
