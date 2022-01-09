;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.06.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;09.01.2022	w5277c@gmail.com			Все по новой
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_DS3231
.else
.set DEF_DRIVER_DS3231 = 1

.include "./core/drivers/_rtc.inc"
.include	"./conv/bcd_to_byte.inc"
.include	"./conv/byte_to_bcd.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./dt/dt_to_timestamp.inc"
.include	"./dt/timestamp_to_dt.inc"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_DS3231_I2C_ADDR					= 0x68			;Адрес устройства на шине I2C
	;---I2C-DATA-OFFSETS----------------------------------
	.EQU	_DRV_DS3231_BUFFER_DT_OFFSET		= 0x00
	.EQU	_DRV_DS3231_BUFFER_A1_OFFSET		= 0x07
	.EQU	_DRV_DS3231_BUFFER_A2_OFFSET		= 0x0b
	.EQU	_DRV_DS3231_BUFFER_CNTR_OFFSET	= 0x0e
	.EQU	_DRV_DS3231_BUFFER_TEMP_OFFSET	= 0x10
	;---BUFFER-OFFSETS------------------------------------
	.EQU	_DRV_DS3231_I2C_DRV_ID				= 0x00			;1Б
	.EQU	_DRV_DS3231_BUFFER_OFFSET			= 0x01			;1Б
	.EQU	_DRV_DS3231_BUFFER_DATA				= 0x02
	;------CURRENT-DATE/TIME------------------------------
	.EQU		_DRV_DS3231_BUFFER_DT_SECONDS	= 0x02			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_MINUTES	= 0x03			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_HOUR		= 0x04			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_DAY		= 0x05			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_DATE		= 0x06			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_MONTH	= 0x07			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_YEAR		= 0x08			;1Б
	.EQU		_DRV_DS3231_BUFFER_DT_SIZE		= 0x09
	;------ALARM1-----------------------------------------
	.EQU		_DRV_DS3231_BUFFER_A1_SECONDS	= 0x02			;1Б
	.EQU		_DRV_DS3231_BUFFER_A1_MINUTES	= 0x03			;1Б
	.EQU		_DRV_DS3231_BUFFER_A1_HOUR		= 0x04			;1Б
	.EQU		_DRV_DS3231_BUFFER_A1_DAY		= 0x05			;1Б
	.EQU		_DRV_DS3231_BUFFER_A1_DATE		= 0x06			;1Б
	.EQU		_DRV_DS3231_BUFFER_A1_SIZE		= 0x07
	;------ALARM2-----------------------------------------
	.EQU		_DRV_DS3231_BUFFER_A2_MINUTES	= 0x02			;1Б
	.EQU		_DRV_DS3231_BUFFER_A2_HOUR		= 0x03			;1Б
	.EQU		_DRV_DS3231_BUFFER_A2_DATE		= 0x04			;1Б
	.EQU		_DRV_DS3231_BUFFER_A2_SIZE		= 0x05
	;------CONTROL/STATUS---------------------------------
	.EQU		_DRV_DS3231_BUFFER_CNTR_H		= 0x02			;2Б
	.EQU		_DRV_DS3231_BUFFER_CNTR_L		= 0x03			;2Б
	.EQU		_DRV_DS3231_BUFFER_CNTR_SIZE	= 0x04
	;------TEMPERATURE------------------------------------
	.EQU		_DRV_DS3231_BUFFER_TEMP_AGING	= 0x02			;1Б
	.EQU		_DRV_DS3231_BUFFER_TEMP_H		= 0x03			;1Б
	.EQU		_DRV_DS3231_BUFFER_TEMP_L		= 0x04			;1Б
	.EQU		_DRV_DS3231_BUFFER_TEMP_SIZE	= 0x05
	;---VARS----------------------------------------------
	
	;---OPERATIONS---
	.EQU	DRV_DS3231_OP_SET_DT					= 0x80
	.EQU	DRV_DS3231_OP_GET_DT					= 0x81
	.EQU	DRV_DS3231_OP_SET_ALARM				= 0x82
	.EQU	DRV_DS3231_OP_GET_ALARM				= 0x83
	.EQU	DRV_DS3231_OP_ENABLE_ALARM			= 0x84
	.EQU	DRV_DS3231_OP_DISABLE_ALARM		= 0x85
	.EQU	DRV_DS3231_OP_SET_CR					= 0x86
	.EQU	DRV_DS3231_OP_GET_SR					= 0x87
	.EQU	DRV_DS3231_OP_GET_TEMP				= 0x88

	.EQU	_DRV_DS3231_RAM_SIZE					= _DRV_DS3231_BUFFER_DT_SIZE

;--------------------------------------------------------
DRV_DS3231_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM-ид драйвера I2C
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_DS3231_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Y+_DRV_DS3231_I2C_DRV_ID,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, работа с часами
;IN: FLAGS-код операции
;OUT: TEMP-результат(DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_DS3231_I2C_DRV_ID

	CPI FLAGS,DRV_RTC_SET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_SET_TIMESTAMP
	RJMP _DRV_DS3231_PROC_END
	CPI FLAGS,DRV_RTC_GET_TIMESTAMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_TIMESTAMP
	RJMP _DRV_DS3231_PROC_END
	

	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_DS3231_PROC_END:
	POP_Y
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_SET_TIMESTAMP:
;--------------------------------------------------------
;Записываем DATE/TIME из TIMESTAMP
;IN: Y-адрес на переменные, TEMP-I2C drv id
;TEMP_EH/EL/H/L-TIMESTAMP
;OUT: TEMP-результат(DRV_RESULT_...)
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH_X
	PUSH_Z

	MCALL TIMESTAMP_TO_DT
	;TODO RCALL _DRV_DS3231__OP_SET_DT
	;TODO RET


	MCALL BYTE_TO_BCD
	STD Y+_DRV_DS3231_BUFFER_DT_DATE,TEMP	
	MOV TEMP,XH
	MCALL BYTE_TO_BCD
	STD Y+_DRV_DS3231_BUFFER_DT_SECONDS,TEMP
	MOV TEMP,TEMP_L
	MCALL BYTE_TO_BCD
	STD Y+_DRV_DS3231_BUFFER_DT_MINUTES,TEMP
	MOV TEMP,TEMP_H
	MCALL _DRV_DS3231_BYTE_TO_HOURS
	STD Y+_DRV_DS3231_BUFFER_DT_HOUR,TEMP
	STD Y+_DRV_DS3231_BUFFER_DT_DAY,C0x00
	MOV TEMP,TEMP_EL
	MCALL BYTE_TO_BCD
	ORI TEMP,0x80
	STD Y+_DRV_DS3231_BUFFER_DT_MONTH,TEMP
	MOV TEMP,TEMP_EH
	MCALL BYTE_TO_BCD
	STD Y+_DRV_DS3231_BUFFER_DT_YEAR,TEMP

	LDI ACCUM,_DRV_DS3231_BUFFER_DT_OFFSET
	STD Y+_DRV_DS3231_BUFFER_OFFSET,ACCUM
	MOVW ZL,YL
	ADIW ZL,_DRV_DS3231_BUFFER_OFFSET
	MOVW XL,YL
	ADIW XL,_DRV_DS3231_BUFFER_DATA
	LDI TEMP_H,_DRV_DS3231_BUFFER_DT_SIZE
	LDI TEMP_L,0x01;???
	LDI ACCUM,DRV_DS3231_I2C_ADDR
	MCALL C5_EXEC

	POP_Z
	POP_X
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	RET


;--------------------------------------------------------
_DRV_DS3231__OP_GET_TIMESTAMP:
;--------------------------------------------------------
;Считываем DATE/TIME данные и конвертируем в TIMESTAMP
;IN: Y-адрес на переменные, TEMP-I2C drv id
;OUT: TEMP_EH/EL/H/L-TIMESTAMP,
;TEMP-результат(DRV_RESULT_...)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_X
	PUSH_Z

	;TODO RCALL _DRV_DS3231__OP_GET_DT

	LDI ACCUM,_DRV_DS3231_BUFFER_DT_OFFSET
	STD Y+_DRV_DS3231_BUFFER_OFFSET,ACCUM
	MOVW ZL,YL
	ADIW ZL,_DRV_DS3231_BUFFER_OFFSET
	MOVW XL,YL
	ADIW XL,_DRV_DS3231_BUFFER_DATA
	LDI TEMP_H,0x01
	LDI TEMP_L,_DRV_DS3231_BUFFER_DT_SIZE
	LDI ACCUM,DRV_DS3231_I2C_ADDR
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_DS3231__OP_GET_TIMESTAMP__END
	
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_YEAR
	MCALL BCD_TO_BYTE
	MOV TEMP_EH,TEMP
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_MONTH
	MOV TEMP_EL,TEMP
	ANDI TEMP_EL,0x0f
	SBRC TEMP,0x04
	SUBI TEMP_EL,(0x100-0x0a)
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_HOUR
	MCALL _DRV_DS3231_HOURS_TO_BYTE
	MOV TEMP_H,TEMP
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_MINUTES
	MCALL BCD_TO_BYTE
	MOV TEMP_L,TEMP
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_SECONDS
	MCALL BCD_TO_BYTE
	MOV XH,TEMP
	LDD TEMP,Y+_DRV_DS3231_BUFFER_DT_DATE
	MCALL BCD_TO_BYTE

	MCALL DT_TO_TIMESTAMP
	LDI TEMP,DRV_RESULT_OK

_DRV_DS3231__OP_GET_TIMESTAMP__END:
	POP_Z
	POP_X
	RET

;--------------------------------------------------------
_DRV_DS3231_BYTE_TO_HOURS:
;--------------------------------------------------------
;Преобразуем байт в часы
;IN: TEMP-байт
;OUT: TEMP-значение от RTC
;--------------------------------------------------------
	PUSH ACCUM

	MOV ACCUM,TEMP
	ANDI TEMP,0x0f
	CPI ACCUM,0x14
	BRCS PC+0x03
	ORI TEMP,0x20
	RJMP PC+0x04
	CPI ACCUM,0x0a
	BRCS PC+0x02
	ORI TEMP,0x10

	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_DS3231_HOURS_TO_BYTE:
;--------------------------------------------------------
;Преобразуем часы в байт
;IN: TEMP-значение от RTC
;OUT: TEMP-байт
;--------------------------------------------------------
	PUSH ACCUM

	MOV ACCUM,TEMP
	ANDI TEMP,0x0f
	SBRC ACCUM,0x04
	SUBI TEMP,(0x100-0x0a)
	SBRS ACCUM,0x06
	RJMP _DRV_DS3231_FORMAT24__24
_DRV_DS3231_FORMAT24__12:
	SBRC ACCUM,0x05
	SUBI TEMP,(0x100-0x0c)
	RJMP _DRV_DS3231_FORMAT24__END
_DRV_DS3231_FORMAT24__24:
	SBRC ACCUM,0x05
	SUBI TEMP,(0x100-0x14)

_DRV_DS3231_FORMAT24__END:
	POP ACCUM
	RET
.endif