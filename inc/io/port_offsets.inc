;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifndef DEF_C5_PORT_OFFSETS
.set DEF_C5_PORT_OFFSETS = 1

.include	"./inc/conv/bitnum_to_num.inc"

;--------------------------------------------------------
C5_PORT_OFFSETS:
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

	LDI ZH,high(_C5_PORTS_TABLE*2)
	LDI ZL,low(_C5_PORTS_TABLE*2)
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
	MCALL C5_BITNUM_TO_NUM
	MOV ACCUM,TEMP
	POP TEMP

	POP_Z
	RET
.endif
