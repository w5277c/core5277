;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;09.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;22.01.2020  w5277c@gmail.com			Учет CORE_FREQ
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_WAIT_2MS
.else
.set DEF_C5_WAIT_2MS = 1

C5_WAIT_2MS:
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
C5_WAIT_2MS__LOOP1:
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,(59*CORE_FREQ)/10
C5_WAIT_2MS__LOOP2:
	LDI TEMP_L,0x40
C5_WAIT_2MS__LOOP3:
	NOP
	DEC TEMP_L
	BRNE C5_WAIT_2MS__LOOP3
	DEC TEMP_H
	BRNE C5_WAIT_2MS__LOOP2
	POP TEMP_L
	POP TEMP_H

	SUBI TEMP,0x01
	SBCI TEMP_L,0x00
	SBCI TEMP_H,0x00
	CPI TEMP,0x00
	BRNE C5_WAIT_2MS__LOOP1
	CPI TEMP_L,0x00
	BRNE C5_WAIT_2MS__LOOP1
	CPI TEMP_H,0x00
	BRNE C5_WAIT_2MS__LOOP1

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
