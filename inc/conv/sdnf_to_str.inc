;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_SDNF_TO_STR
.else
.set DEF_CORE5277_SDNF_TO_STR = 1

.include	"./inc/conv/num_to_str.inc"

;--------------------------------------------------------
CORE5277_SDNF_TO_STR:												;Signed Digital aNd Fractional
;--------------------------------------------------------
;Конвертирование знакового целого с дробной в текст
;Конец строки не указывается
;IN: TEMP_H - целая, TEMP_L - дробная со знаком(7-й бит),
;Z - dst адрес
;OUT: Z - новое смещение
;-START--------------------------------------------------
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L

	LDI TEMP,'-'
	SBRC TEMP_L,0x07
	ST Z+,TEMP
	ANDI TEMP_L,0x7f

	PUSH TEMP_L
	MOV TEMP,TEMP_H
	MCALL CORE5277_NUM_TO_STR
	CPI TEMP_H,'0'
	BREQ PC+0x02
	ST Z+,TEMP_H
	CPI TEMP_H,'0'
	BRNE PC+03
	CPI TEMP_L,'0'
	BREQ PC+0x02
	ST Z+,TEMP_L
	ST Z+,TEMP
	POP TEMP_L

	TST TEMP_L
	BREQ _CORE5277_SDNF_TO_STR__ZERO_F

	LDI TEMP,'.'
	ST Z+,TEMP

	MOV TEMP,TEMP_L
	MCALL CORE5277_NUM_TO_STR
	ST Z+,TEMP_L
	ST Z+,TEMP

_CORE5277_SDNF_TO_STR__ZERO_F:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
