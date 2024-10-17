;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.03.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;05.12.2020	w5277c@gmail.com			-с5
;12.10.2024	konstantin@5277.ru		Реализация виртуальных портов
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PORT_OFFSETS
.else
.set DEF_PORT_OFFSETS = 1

.if REPORT_INCLUDES == 0x01
.message "included PORT_OFFSETS"
.endif

.include	"./conv/bitnum_to_num.inc"

;--------------------------------------------------------
PORT_OFFSETS:
;--------------------------------------------------------
;Возвращаем адреса для PORTx, DDRx и PINx,
;а также пин в виде числа(1,2,4,8,16,32,64,128)
;IN: ACCUM-сдвоенный порт и пин (PA0, PC7 и т.п.)
;OUT: TEMP_H-адрес регистра PORTx,
;TEMP_L-адрес регистра DDRx,
;TEMP-адрес регистра PINx
;ACCUM-пин в виде числа
;--------------------------------------------------------
	CPI ACCUM,0xff
	BRNE PC+0x02
	RET
.if VPORTS>0x00
	CPI ACCUM,(PORTS_QUANTITY<<0x04)-0x01
	BRCS PC+0x01+_MCALL_SIZE
	MJMP VPORT_OFFSETS
.endif

	PUSH_Z

	LDI_Z (PORTS_TABLE*2)
	MOV TEMP,ACCUM
	SWAP ACCUM
	ANDI ACCUM,0x0f
	MOV TEMP_H,ACCUM
	LSL ACCUM
	ADD ACCUM,TEMP_H
	ADD ZL,ACCUM
	ADC ZH,C0x00
	LPM TEMP_H,Z+
	LPM TEMP_L,Z+
	LPM ACCUM,Z
	PUSH ACCUM
	ANDI TEMP,0x0f
	MCALL BITNUM_TO_NUM
	MOV ACCUM,TEMP
	POP TEMP

	POP_Z
	RET
.endif
