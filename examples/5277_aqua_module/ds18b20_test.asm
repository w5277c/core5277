;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/wait_1s.inc"
.include	"./core/io/out_byte.inc"
.include	"./core/io/out_char.inc"
.include	"./core/io/out_sdnf.inc"

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
	MCALL C5_OUT_CHAR
	LDI TEMP,':'
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_H
	MCALL C5_OUT_BYTE
	MCALL C5_OUT_CR
	RJMP DS18B20_TEST_LOOP
DS18B20_TEST_NO_ERROR:
	MCALL C5_OUT_SDNF
	MCALL C5_OUT_CR
	RJMP DS18B20_TEST_LOOP

