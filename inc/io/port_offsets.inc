;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_PORT_OFFSETS
.else
.set DEF_CORE5277_PORT_OFFSETS = 1

.include	"./inc/conv/bitnum_to_num.inc"

;--------------------------------------------------------
CORE5277_PORT_OFFSETS:
;--------------------------------------------------------
;Возвращаем адреса для PORTx, DDRx и PINx,
;а также пин в виде числа(1,2,4,8,16,32,64,128)
;IN: ACCUM сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: TEMP_H - адрес регистра PORTx,
;TEMP_L - адрес регистра DDRx,
;TEMP - адрес регистра PINx
;ACCUM - пин в виде числа
;--------------------------------------------------------
	PUSH_Z

	LDI ZH,high(_CORE5277_PORTS_TABLE*2)
	LDI ZL,low(_CORE5277_PORTS_TABLE*2)
	MOV TEMP,ACCUM
	SWAP ACCUM
	ANDI ACCUM,0x0f
	MOV TEMP_H,ACCUM
	LSL ACCUM
	ADD ACCUM,TEMP_H
	ADD ZL,ACCUM
	CLR ACCUM
	ADC ZH,ACCUM
	LPM TEMP_H,Z+
	LPM TEMP_L,Z+
	LPM ACCUM,Z
	PUSH ACCUM
	ANDI TEMP,0x0f
	MCALL CORE5277_BITNUM_TO_NUM
	MOV ACCUM,TEMP
	POP TEMP

	POP_Z
	RET
.endif
