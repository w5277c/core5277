;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.04.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_RTC
.else
.set DEF__DRV_RTC = 1

	;---OPERATIONS---
	.EQU	DRV_RTC_SET_TIMESTAMP				= 0x01			;Установить время
	.EQU	DRV_RTC_GET_TIMESTAMP				= 0x02			;Считать время

.endif
