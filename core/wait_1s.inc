;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;09.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_WAIT_1S
.else
.set DEF_C5_WAIT_1S = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_WAIT_1S"
.endif

.include "./core/wait_2ms.inc"
;--------------------------------------------------------
C5_WAIT_1S:
;--------------------------------------------------------
;Ждем
;IN TEMP - время в 1s
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	MOV LOOP_CNTR,TEMP
_C5_WAIT_SEC__LOOP:
	CLR TEMP_H
	LDI TEMP_L,0x01
	LDI TEMP,0xf4
	MCALL C5_WAIT_2MS
	DEC LOOP_CNTR
	BRNE _C5_WAIT_SEC__LOOP

_C5_WAIT_SEC__END:
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
