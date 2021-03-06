;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CRC8_MAXIM
.else
.set DEF_CRC8_MAXIM = 1

.if REPORT_INCLUDES == 0x01
.message "included CRC8_MAXIM"
.endif

;--------------------------------------------------------
CRC8_MAXIM:
;--------------------------------------------------------
;Подсчет CRC
;IN: TEMP - байт, ACCUM - сумма
;OUT: ACCUM - сумма
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH LOOP_CNTR
	LDI TEMP_L,0x8C
	EOR ACCUM,TEMP
	LDI LOOP_CNTR,0x08
_CRC8_MAXIM__LOOP:
	LSR ACCUM
	BRCC PC+0x02
	EOR ACCUM,TEMP_L
	DEC LOOP_CNTR
	BRNE _CRC8_MAXIM__LOOP
	POP LOOP_CNTR
	POP TEMP_L
	RET
.endif
