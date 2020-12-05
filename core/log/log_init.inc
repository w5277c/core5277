;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_INIT
.else
.set DEF_C5_LOG_INIT = 1
.ifdef LOGGING_PORT

;--------------------------------------------------------
C5_LOG_INIT:
;--------------------------------------------------------
;Инициализация порта логирования
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH ACCUM

	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,(LOGGING_PORT>>4)*0x03+0x01
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(LOGGING_PORT & 0x0f)
	ST Z,ACCUM

	LDI ZH,high(PORTS_TABLE*2)
	LDI ZL,low(PORTS_TABLE*2)
	LDI ACCUM,(LOGGING_PORT>>4)*0x03
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(LOGGING_PORT & 0x0f)
	ST Z,ACCUM

	POP ACCUM
	POP ZL
	POP ZH
	RET
.endif
.endif
