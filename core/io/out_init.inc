;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com        Начало
;02.08.2020	w5277c@gmail.com        Разбиение на файлы
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;21.11.2024	konstantin@5277.ru		Сократил PORTS_TABLE, оставив только PINx(всегда DDRx=PINx+1, PORTx=PINx+2)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_INIT
.else
.set DEF_C5_OUT_INIT = 1
.ifdef LOGGING_PORT

;--------------------------------------------------------
C5_OUT_INIT:
;--------------------------------------------------------
;Инициализация порта логирования
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM

	LDI_Z PORTS_TABLE*2+(LOGGING_PORT>>4)						;PINx
	ADIW ZL,0x02														;PORTx
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(LOGGING_PORT & 0x0f)
	ST Z,ACCUM

	LDI_Z PORTS_TABLE*2+(LOGGING_PORT>>4)						;PINx
	ADIW ZL,0x01														;DDRx
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(LOGGING_PORT & 0x0f)
	ST Z,ACCUM

	POP ACCUM
	POP_Z
	RET
.endif
.endif
