;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020	w5277c@gmail.com			-с5
;06.07.2021	w5277c@gmail.com			Y->Z
;03.09.2021	w5277c@gmail.com			Значение 0xff для целой части корректно
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_SDNF_SUB
.else
.set DEF_SDNF_SUB = 1

.if REPORT_INCLUDES == 0x01
.message "included SDNF_SUB"
.endif

.include	"./math/sdnf_add.inc"

;--------------------------------------------------------
SDNF_SUB:																;Signed Digital aNd Fractional
;--------------------------------------------------------
;Разница знаковых целых с дробной частью (X - Z)
;IN: X, Z
;OUT: X
;--------------------------------------------------------
	;Проверка на корректность значений
	CPI XL,0xff
	BREQ _SDNF_SUB__ERROR
	CPI ZL,0xff
	BREQ _SDNF_SUB__ERROR

	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,0x01
	LSL ZL
	ROL TEMP_L
	EOR TEMP_H,TEMP_L
	LSR TEMP_H
	ROR ZL
	POP TEMP_L
	POP TEMP_H
	RJMP SDNF_ADD
_SDNF_SUB__ERROR:
	LDI XL,0xff
	RET
.endif
