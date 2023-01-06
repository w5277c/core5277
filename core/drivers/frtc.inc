;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;16.09.2021	w5277c@gmail.com			Z->Y
;06.01.2022	w5277c@gmail.com			Акутализация кода
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FRTC
.else
.set DEF_C5_FRTC = 1
.message "Included driver Fake RTC v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/drivers/_rtc.inc"
.include	"./mem/ram_fill.inc"
.include	"./core/uptime_copy.inc"
.include	"./math/divnx16.inc"

	;---OFFSETS---
	.EQU	_DRV_FRTC_UPTIME											= 0x00;5B Значение UPTIME для вычислений
	.EQU	_DRV_FRTC_TIMESTAMP										= 0x05;4B Значение TIMESTAMP при UPTIME=0
	;---
	.EQU	_DRV_FRTC_RAM_SIZE										= 0x09;

;--------------------------------------------------------
DRV_FRTC_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_FRTC_RAM_SIZE
	MCALL C5_RAM_REALLOC

	LDI LOOP_CNTR,_DRV_FRTC_RAM_SIZE
	LDI TEMP,0x00
	MOVW XL,YL
	MCALL RAM_FILL

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: FLAGS-тип операции
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_Y

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_RTC_SET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FRTC_SET_TIMESTAMP
	RJMP _DRV_FRTC__END
	CPI FLAGS,DRV_RTC_GET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FRTC_GET_TIMESTAMP
	RJMP _DRV_FRTC__END
	;...

	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_FRTC__END:

	POP_Y
	RET

;--------------------------------------------------------
_DRV_FRTC_SET_TIMESTAMP:
;--------------------------------------------------------
;Устанвливаем TIMESTAMP с учетом UPTIME
;IN: TEMP_EH/EL/H/L-временная метка в сек.
;OUT: TEMP-DRV_RTC_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH ACCUM
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L

	MCALL C5_DISPATCHER_LOCK
	MCALL C5_UPTIME_COPY
	LDI_X 500
	LDI ACCUM,0x05
	MCALL DIVNX16

	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x04
	SUB TEMP_L,TEMP
	STD Y+_DRV_FRTC_TIMESTAMP+0x03,TEMP_L
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x03
	SBC TEMP_H,TEMP
	STD Y+_DRV_FRTC_TIMESTAMP+0x02,TEMP_H
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x02
	SBC TEMP_EL,TEMP
	STD Y+_DRV_FRTC_TIMESTAMP+0x01,TEMP_EL
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x01
	SBC TEMP_EH,TEMP
	STD Y+_DRV_FRTC_TIMESTAMP+0x00,TEMP_EH

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP,DRV_RESULT_OK

	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP ACCUM
	POP_X
	RET

;--------------------------------------------------------
_DRV_FRTC_GET_TIMESTAMP:
;--------------------------------------------------------
;Возвращаем текущий TIMESTAMP
;OUT TEMP_EH/EL/H/L-временная метка в сек.,
;TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH ACCUM

	MCALL C5_DISPATCHER_LOCK
	MCALL C5_UPTIME_COPY
	LDI_X 500
	LDI ACCUM,0x05
	MCALL DIVNX16

	MCALL C5_DISPATCHER_LOCK
	LDD TEMP_L,Y+_DRV_FRTC_TIMESTAMP+0x03
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x04
	ADD TEMP_L,TEMP
	LDD TEMP_H,Y+_DRV_FRTC_TIMESTAMP+0x02
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x03
	ADC TEMP_H,TEMP
	LDD TEMP_EL,Y+_DRV_FRTC_TIMESTAMP+0x01
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x02
	ADC TEMP_EL,TEMP
	LDD TEMP_EH,Y+_DRV_FRTC_TIMESTAMP+0x00
	LDD TEMP,Y+_DRV_FRTC_UPTIME+0x01
	ADC TEMP_EH,TEMP

	MCALL C5_DISPATCHER_UNLOCK
	LDI TEMP,DRV_RESULT_OK

	POP ACCUM
	POP_X
	RET
.endif
