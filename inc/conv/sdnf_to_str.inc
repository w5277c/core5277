;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SDNF_TO_STR
.else
.set DEF_C5_SDNF_TO_STR = 1

.include	"./inc/conv/num_to_str.inc"

;--------------------------------------------------------
C5_SDNF_TO_STR:												;Signed Digital aNd Fractional
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
	MCALL C5_NUM_TO_STR
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
	BREQ _C5_SDNF_TO_STR__ZERO_F

	LDI TEMP,'.'
	ST Z+,TEMP

	MOV TEMP,TEMP_L
	MCALL C5_NUM_TO_STR
	ST Z+,TEMP_L
	ST Z+,TEMP

_C5_SDNF_TO_STR__ZERO_F:
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	RET
.endif
