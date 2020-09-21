;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;09.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_WAIT_2MS
.else
.set DEF_CORE5277_WAIT_2MS = 1

CORE5277_WAIT_2MS:
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
CORE5277_WAIT_2MS__LOOP1:

	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,39
	LDI TEMP_L,178
CORE5277_WAIT_2MS__LOOP2:
	DEC TEMP_L
	BRNE CORE5277_WAIT_2MS__LOOP2
	DEC TEMP_H
	BRNE CORE5277_WAIT_2MS__LOOP2
	POP TEMP_L
	POP TEMP_H

	SUBI TEMP,0x01
	SBCI TEMP_L,0x00
	SBCI TEMP_H,0x00
	CPI TEMP,0x00
	BRNE CORE5277_WAIT_2MS__LOOP1
	CPI TEMP_L,0x00
	BRNE CORE5277_WAIT_2MS__LOOP1
	CPI TEMP_H,0x00
	BRNE CORE5277_WAIT_2MS__LOOP1

	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
