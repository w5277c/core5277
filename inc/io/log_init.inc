;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_LOG_INIT
.else
.set DEF_CORE5277_LOG_INIT = 1
.ifdef LOGGING_PORT

;--------------------------------------------------------
CORE5277_LOG_INIT:
;--------------------------------------------------------
;Инициализация порта логирования
;--------------------------------------------------------
	PUSH ZH
	PUSH ZL
	PUSH ACCUM

	LDI ZH,high(_CORE5277_PORTS_TABLE*2)
	LDI ZL,low(_CORE5277_PORTS_TABLE*2)
	LDI ACCUM,(LOGGING_PORT>>4)*0x03+0x01
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(LOGGING_PORT & 0x0f)
	ST Z,ACCUM

	LDI ZH,high(_CORE5277_PORTS_TABLE*2)
	LDI ZL,low(_CORE5277_PORTS_TABLE*2)
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
