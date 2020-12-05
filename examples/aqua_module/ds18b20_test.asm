;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/core/wait_1s.inc"
.include	"./inc/io/log_byte.inc"
.include	"./inc/io/log_char.inc"
.include	"./inc/io/log_sdnf.inc"

DS18B20_TEST_INIT:
	MCALL C5_READY
;--------------------------------------------------------
DS18B20_TEST_LOOP:
	;MCALL C5_SUSPEND
	LDI TEMP,0x01														;Пауза в 1 сеунду
	MCALL C5_WAIT_1S

	LDI TEMP,PID_DS18B20_DRV
	MCALL C5_EXEC
	CPI TEMP_L,0xFF
	BRNE DS18B20_TEST_NO_ERROR
	LDI TEMP,'E'
	MCALL C5_LOG_CHAR
	LDI TEMP,':'
	MCALL C5_LOG_CHAR
	MOV TEMP,TEMP_H
	MCALL C5_LOG_BYTE
	C5_LOG_ROMSTR LOGSTR_NEW_LINE
	RJMP DS18B20_TEST_LOOP
DS18B20_TEST_NO_ERROR:
	MCALL C5_LOG_SDNF
	C5_LOG_ROMSTR LOGSTR_NEW_LINE
	RJMP DS18B20_TEST_LOOP
