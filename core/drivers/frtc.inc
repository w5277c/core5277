;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;16.09.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FRTC
.else
.set DEF_C5_FRTC = 1
.message "Included driver Fake RTC v0.1"

	;---OFFSETS---
	.EQU	_DRV_FRTC_TIMESTAMP										= 0x00;4B Значение времени при UPTIME=0
	;---
	.EQU	_DRV_FRTC_VARS_SIZE										= 0x04;

;--------------------------------------------------------
DRV_FRTC_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_FRTC_VARS_SIZE
	MCALL C5_RAM_REALLOC

	LDI ACCUM,0x00
	STD Y+_DRV_FRTC_TIMESTAMP+0x00,ACCUM
	STD Y+_DRV_FRTC_TIMESTAMP+0x01,ACCUM
	STD Y+_DRV_FRTC_TIMESTAMP+0x02,ACCUM
	STD Y+_DRV_FRTC_TIMESTAMP+0x03,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: ACCUM-тип операции
;OUT: FLAGS-результат DRV_RTC_RESULT_...
;--------------------------------------------------------
	PUSH_Y

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	CPI ACCUM,DRV_RTC_SET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FRTC_SET_TIMESTAMP
	RJMP _DRV_FRTC__END
	CPI ACCUM,DRV_RTC_GET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FRTC_GET_TIMESTAMP
	RJMP _DRV_FRTC__END
	;...

	LDI FLAGS,DRV_RTC_RESULT_UNSUPPORTED
_DRV_FRTC__END:

	POP_Y
	RET

;--------------------------------------------------------
_DRV_FRTC_SET_TIMESTAMP:
;--------------------------------------------------------
;Устанвливаем TIMESTAMP с учетом UPTIME=0
;IN: TEMP_EH/EL/H,L-временная метка в сек.
;OUT: FLAGS-результат DRV_RTC_RESULT_...
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	MCALL C5_UPTIME_GET
	LSR TEMP_EH
	ROR TEMP_EL
	ROR TEMP_H
	ROR TEMP_L

	MCALL C5_DISPATCHER_LOCK
	POP TEMP
	SUB TEMP,TEMP_L
	STD Y+_DRV_FRTC_TIMESTAMP+0x03,TEMP
	POP TEMP
	SBC TEMP,TEMP_H
	STD Y+_DRV_FRTC_TIMESTAMP+0x02,TEMP
	POP TEMP
	SBC TEMP,TEMP_EL
	STD Y+_DRV_FRTC_TIMESTAMP+0x01,TEMP
	POP TEMP
	SBC TEMP,TEMP_EH
	STD Y+_DRV_FRTC_TIMESTAMP+0x00,TEMP
	MCALL C5_DISPATCHER_UNLOCK

	LDI FLAGS,DRV_RTC_RESULT_OK

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	RET

;--------------------------------------------------------
_DRV_FRTC_GET_TIMESTAMP:
;--------------------------------------------------------
;Возвращаем текущий TIMESTAMP
;OUT TEMP_EH/EL/H,L-временная метка в сек.,
;FLAGS-результат DRV_RTC_RESULT_...
;--------------------------------------------------------
	PUSH TEMP

	MCALL C5_UPTIME_GET
	LSR TEMP_EH
	ROR TEMP_EL
	ROR TEMP_H
	ROR TEMP_L

	LDI FLAGS,0x00
	MCALL C5_DISPATCHER_LOCK
	LDD TEMP,Y+_DRV_FRTC_TIMESTAMP+0x03
	OR FLAGS,TEMP
	ADD TEMP_L,TEMP
	LDD TEMP,Y+_DRV_FRTC_TIMESTAMP+0x02
	OR FLAGS,TEMP
	ADC TEMP_H,TEMP
	LDD TEMP,Y+_DRV_FRTC_TIMESTAMP+0x01
	OR FLAGS,TEMP
	ADC TEMP_EL,TEMP
	LDD TEMP,Y+_DRV_FRTC_TIMESTAMP+0x00
	OR FLAGS,TEMP
	ADC TEMP_EH,TEMP
	MCALL C5_DISPATCHER_UNLOCK

	CPI FLAGS,0x00
	BRNE _DRV_FRTC_GET_TIMESTAMP__END
	CLR TEMP_EH
	CLR TEMP_EL
	CLR TEMP_H
	CLR TEMP_L

_DRV_FRTC_GET_TIMESTAMP__END:
	LDI FLAGS,DRV_RTC_RESULT_OK

	POP TEMP
	RET

.endif
