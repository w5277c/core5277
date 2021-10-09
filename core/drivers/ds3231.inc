;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;08.06.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
;TODO привести к виду _rtc.inc
;TODO C5_RAM_OFFSET Z->Y
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_DRIVER_DS3231
.else
.set DEF_DRIVER_DS3231 = 1

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./conv/bcd_byte_to_str.inc"

;---CONSTANTS--------------------------------------------
	.EQU	DRV_DS3231__I2C_ADDR					= 0x68			;Адрес устройства на шине I2C
	.EQU	DRV_DS3231__BUFFER_SIZE				= 0x13			;Размер буфера
	.EQU	DRV_DS3231__TIME_STR_SIZE			= 0x09			;Размер строки времени (ЧЧ:ММ:СС+0x00)
	.EQU	DRV_DS3231__DATE_STR_SIZE			= 0x09			;Размер строки даты (ДД.ММ.ГГ+0x00)
	.EQU	DRV_DS3231__DATETIME_STR_SIZE		= 0x12			;Размер строки даты и времени (ЧЧ:ММ:СС ДД.ММ.ГГ+0x00)
	;---VARS---
	.EQU	DRV_DS3231__VAR_HOUR					= 0x00
	.EQU	DRV_DS3231__VAR_MINUTE				= 0x01
	.EQU	DRV_DS3231__VAR_SECOND				= 0x02
	.EQU	DRV_DS3231__VAR_DATE					= 0x03
	.EQU	DRV_DS3231__VAR_MONTH				= 0x04
	.EQU	DRV_DS3231__VAR_YEAR					= 0x05
	.EQU	DRV_DS3231__VAR_I2C_DRV_ID			= 0x06
	;---OPERATIONS---
	.EQU	DRV_DS3231__OP_READ_DATA			= 0x00
	.EQU	DRV_DS3231__OP_WRITE_DATA			= 0x01
	.EQU	DRV_DS3231__OP_GET_TIME_STR		= 0x02
	.EQU	DRV_DS3231__OP_GET_DATE_STR		= 0x03
	.EQU	DRV_DS3231__OP_GET_DATETIME_STR	= 0x04
	.EQU	DRV_DS3231__OP_GET_ALARM			= 0x05
	.EQU	DRV_DS3231__OP_SET_ALARM			= 0x06
	.EQU	DRV_DS3231__OP_ENABLE_ALARM		= 0x07
	.EQU	DRV_DS3231__OP_DISABLE_ALARM		= 0x08
	.EQU	DRV_DS3231__OP_GET_STATUS_REG		= 0x09
	.EQU	DRV_DS3231__OP_GET_CONTROL_REG	= 0x0a
	.EQU	DRV_DS3231__OP_SET_CONTROL_REG	= 0x0b
	.EQU	DRV_DS3231__OP_GET_TEMP				= 0x0c

_DRV_DS3231_SET_0_OFFSET:
	.db   0x00

;--------------------------------------------------------
DRV_DS3231_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид драйвера i2c
;--------------------------------------------------------
	MOV TEMP,DRIVER_ID
	MOV TEMP_L,ACCUM
	LDI ACCUM,DRV_DS3231__VAR_I2C_DRV_ID
	MCALL C5_VAR_SET

	MOV TEMP,DRIVER_ID
	LDI ACCUM,DRV_DS3231__BUFFER_SIZE
	MCALL C5_RAM_REALLOC

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, работа с часами
;IN: ACCUM - тип операции
;OUT: флаг Z(true - успешно) + данные процедуры
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP

	MOV TEMP,DRIVER_ID
	MCALL C5_RAM_OFFSET

	CPI ACCUM,DRV_DS3231__OP_READ_DATA
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_READ_DATA
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_WRITE_DATA
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_WRITE_DATA
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_TIME_STR
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_TIME_STR
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_DATE_STR
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_DATE_STR
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_DATETIME_STR
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_DATETIME_STR
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_ALARM
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_ALARM
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_ALARM
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_SET_ALARM
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_ENABLE_ALARM
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_ENABLE_ALARM
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_DISABLE_ALARM
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_DISABLE_ALARM
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_STATUS_REG
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_STATUS_REG
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_CONTROL_REG
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_CONTROL_REG
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_SET_CONTROL_REG
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_SET_CONTROL_REG
	RJMP _DRV_DS3231_PROC_END
	CPI ACCUM,DRV_DS3231__OP_GET_TEMP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_DS3231__OP_GET_TEMP
	RJMP _DRV_DS3231_PROC_END
	CLZ

_DRV_DS3231_PROC_END:
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_READ_DATA:
;--------------------------------------------------------
;Считываем данные
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_X
	PUSH_Y

	MOV TEMP,DRIVER_ID
	LDI ACCUM,DRV_DS3231__VAR_I2C_DRV_ID
	MCALL C5_VAR_GET
	MOV TEMP,TEMP_L
	LDI YH,high(_DRV_DS3231_SET_0_OFFSET)|0x80
	LDI YL,low(_DRV_DS3231_SET_0_OFFSET)
	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP_EH,0x01
	LDI TEMP_EL,DRV_DS3231__BUFFER_SIZE
	LDI ACCUM,DRV_DS3231__I2C_ADDR
	MCALL C5_DRIVER_EXEC
	CPI TEMP,DRV_I2C_STATE_OK
	BRNE _DRV_DS3231__OP_READ_DATA_ERROR
	;Копирую данные о времени и дате в переменные
	MOV TEMP,DRIVER_ID
	MCALL C5_VARS_GET_OFFSET

	LD TEMP,X+
	STD Z+DRV_DS3231__VAR_SECOND,TEMP
	LD TEMP,X+
	STD Z+DRV_DS3231__VAR_MINUTE,TEMP
	LD TEMP,X+
	ANDI TEMP,0x3f
	STD Z+DRV_DS3231__VAR_HOUR,TEMP
	LD TEMP,X+
	LD TEMP,X+
	STD Z+DRV_DS3231__VAR_DATE,TEMP
	LD TEMP,X+
	STD Z+DRV_DS3231__VAR_MONTH,TEMP
	LD TEMP,X+
	STD Z+DRV_DS3231__VAR_YEAR,TEMP

	SEZ
	RJMP _DRV_DS3231__OP_READ_DATA_END
_DRV_DS3231__OP_READ_DATA_ERROR:
	CLZ
_DRV_DS3231__OP_READ_DATA_END:
	POP_Y
	POP_X
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_WRITE_DATA:
;--------------------------------------------------------
;Записываем данные
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_X
	PUSH_Y


	MOV TEMP,DRIVER_ID
	LDI ACCUM,0x11
	MCALL C5_RAM_EXTEND

	;Формирую данные о новом времени и дате из переменных
	MOV TEMP,DRIVER_ID
	MCALL C5_VARS_GET_OFFSET
	PUSH_X
	LDI TEMP,0x00														;Адрес первой ячейки для записи DS3231
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_SECOND
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_MINUTE
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_HOUR
	ST X+,TEMP
	LDI TEMP,0x00
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_DATE
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_MONTH
	ST X+,TEMP
	LDD TEMP,Z+DRV_DS3231__VAR_YEAR
	ST X+,TEMP

	LDI TEMP,0x07
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LDI LOOP_CNTR,0x09
_DRV_DS3231__OP_WRITE_DATA_LOOP:
	LD TEMP,Z+
	ST X+,TEMP
	DEC LOOP_CNTR
	BRNE _DRV_DS3231__OP_WRITE_DATA_LOOP
	POP_X

	MOV TEMP,DRIVER_ID
	LDI ACCUM,DRV_DS3231__VAR_I2C_DRV_ID
	MCALL C5_VAR_GET
	MOV TEMP,TEMP_L
	MOV YH,XH
	MOV YL,XL
	LDI TEMP_EH,0x11
	LDI TEMP_EL,0x00
	LDI ACCUM,DRV_DS3231__I2C_ADDR
	MCALL C5_DRIVER_EXEC
	CPI TEMP,DRV_I2C_STATE_OK

	MOV TEMP,DRIVER_ID
	LDI ACCUM,0x11
	MCALL C5_RAM_REDUCE

	POP_Y
	POP_X
	POP TEMP_EL
	POP TEMP_EH
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_TIME_STR:
;--------------------------------------------------------
;Формируем строку времени
;IN: Y - адрес памяти, для записи строки
;OUT: TEMP - длина строкис учетом 0x00,
;флаг Z(true - успешно)
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L

	LDD TEMP,Z+0x02
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,':'
	ST Y+,TEMP
	LDD TEMP,Z+0x01
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,':'
	ST Y+,TEMP
	LDD TEMP,Z+0x00
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,0x00
	ST Y+,TEMP

	SEZ
	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_DATE_STR:
;--------------------------------------------------------
;Формируем строку даты
;IN: Y - адрес памяти, для записи строки
;OUT: TEMP - длина строкис учетом 0x00,
;флаг Z(true - успешно)
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L

	LDD TEMP,Z+0x04
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,'.'
	ST Y+,TEMP
	LDD TEMP,Z+0x05
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,'.'
	ST Y+,TEMP
	LDD TEMP,Z+0x06
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,0x00
	ST Y+,TEMP

	SEZ
	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_DATETIME_STR:
;--------------------------------------------------------
;Формируем строку даты и времени
;IN: Y - адрес памяти, для записи строки
;OUT: TEMP - длина строкис учетом 0x00,
;флаг Z(true - успешно)
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L

	LDD TEMP,Z+0x02
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,':'
	ST Y+,TEMP
	LDD TEMP,Z+0x01
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,':'
	ST Y+,TEMP
	LDD TEMP,Z+0x00
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,' '
	ST Y+,TEMP
	LDD TEMP,Z+0x04
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,'.'
	ST Y+,TEMP
	LDD TEMP,Z+0x05
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,'.'
	ST Y+,TEMP
	LDD TEMP,Z+0x06
	MCALL BCD_BYTE_TO_STR
	ST Y+,TEMP_H
	ST Y+,TEMP_L
	LDI TEMP,0x00
	ST Y+,TEMP

	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_ALARM:
;--------------------------------------------------------
;Считываем данные будильника
;IN: TEMP_L - номер будильника(1,2)
;OUT: XH - hours(0-23), XL - minutes(0-59),
;YH - seconds(0-59),
;YL - date(1-31)/day(0x81-0x87)/(0x00-disabled),
;флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_SET_ALARM:
;--------------------------------------------------------
;Записываем данные будильника
;IN: TEMP_L - номер будильника(1,2)
;XH - hours(0-23), XL - minutes(0-59),
;YH - seconds(0-59),
;YL - date(1-31)/day(0x81-0x87)
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_DISABLE_ALARM:
;--------------------------------------------------------
;Отключаем будильник
;IN: TEMP_L - номер будильника(1,2)
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_ENABLE_ALARM:
;--------------------------------------------------------
;Включаем будильник
;IN: TEMP_L - номер будильника(1,2)
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_STATUS_REG:
;--------------------------------------------------------
;Считыааем статус регистр
;OUT: TEMP - значение регистра,
;флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_CONTROL_REG:
;--------------------------------------------------------
;Считываем регистр упроавления
;OUT: TEMP - значение регистра,
;флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET


;--------------------------------------------------------
_DRV_DS3231__OP_SET_CONTROL_REG:
;--------------------------------------------------------
;Записываем регистр управления
;IN: TEMP - значение регистра
;OUT: флаг Z(true - успешно)
;--------------------------------------------------------
	;TODO
	RET

;--------------------------------------------------------
_DRV_DS3231__OP_GET_TEMP:
;--------------------------------------------------------
;Считываем температуру
;OUT: TEMP_H/TEMP_L - температура в формате SDNF
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L

	CLR TEMP_L
	LDD TEMP,Z+0x12
	SWAP TEMP
	LSR TEMP
	LSR TEMP
	CPI TEMP,0x00
	BREQ _DRV_DS3231__OP_GET_TEMP__LOOP_SKIP
_DRV_DS3231__OP_GET_TEMP__LOOP:
	SUBI TEMP_L,(0x100-0x19)										;+25
	DEC TEMP
	BRNE _DRV_DS3231__OP_GET_TEMP__LOOP
_DRV_DS3231__OP_GET_TEMP__LOOP_SKIP:
	LDD TEMP_H,Z+0x11
	SBRC TEMP_H,0x07
	ORI TEMP_L,0x07
	ANDI TEMP_H,0x7f

	POP TEMP_L
	POP TEMP_H
	RET
.endif




