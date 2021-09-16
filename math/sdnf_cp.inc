;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020	w5277c@gmail.com			-с5
;06.07.2021	w5277c@gmail.com			Y->Z
;03.09.2021	w5277c@gmail.com			Значение 0xff для целой части корректно
;15.09.2021	w5277c@gmail.com			Equal(null==null), если оба значения сотых = 0xff
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_SDNF_CP
.else
.set DEF_SDNF_CP = 1

.if REPORT_INCLUDES == 0x01
.message "included SDNF_CP"
.endif

;--------------------------------------------------------
SDNF_CP:																	;Signed Digital aNd Fractional
;--------------------------------------------------------
;Сравнение двух знаковых целых с дробной частью (X < Z)
;IN: X, Z
;OUT: флаг С(X<Z), флаг Z(X=Z), ошибка - флаги включены
;-START--------------------------------------------------
	PUSH XL
	PUSH ZL
	PUSH TEMP_H
	PUSH TEMP_L
	;Проверка на корректность значений
	CPI XL,0xff
	BREQ PC+03
	CPI ZL,0xff
	BRNE PC+0x04
	CP XL,ZL
	BREQ _SDNF_CP__EQUAL
	RJMP _SDNF_CP__ERROR

	;Выношу знаки в отдельные регистры
	MOV TEMP_H,XL
	ANDI TEMP_H,0x80
	MOV TEMP_L,ZL
	ANDI TEMP_L,0x80
	ANDI XL,0x7f
	ANDI ZL,0x7f
	CPI XL,0x64
	BRCC _SDNF_CP__ERROR
	CPI ZL,0x64
	BRCC _SDNF_CP__ERROR

	;Проверяю целую часть
	CP XH,ZH
	BREQ _SDNF_CP__H_EQUAL
	BRCS PC+0x04
	SBRC TEMP_H,0x07													;Включаю флаг C, если |X|>|Z| и X<0
	SEC
	RJMP _SDNF_CP__END
	SBRC TEMP_L,0x07													;Выключаю флаг C, если |X|<|Z| и Z<0
	CLC
	RJMP _SDNF_CP__END
_SDNF_CP__H_EQUAL:
	;проверяю дробную часть
	CP XL,ZL
	BREQ _SDNF_CP__L_EQUAL
	BRCS PC+0x04
	SBRC TEMP_H,0x07													;Включаю флаг C, если |X|>|Z| и X<0
	SEC
	RJMP _SDNF_CP__END
	SBRC TEMP_L,0x07													;Выключаю флаг C, если |X|<|Z| и Z<0
	CLC
	RJMP _SDNF_CP__END
_SDNF_CP__L_EQUAL:
	EOR TEMP_L,TEMP_H
	BREQ _SDNF_CP__EQUAL
	ROL TEMP_H															;Определяю флаг C, если |X|=|Z| и X<0
	CLZ
	RJMP _SDNF_CP__END
_SDNF_CP__ERROR:
	SEC
	SEZ
	RJMP _SDNF_CP__END
_SDNF_CP__EQUAL:
	CLC
	SEZ
	RJMP _SDNF_CP__END
_SDNF_CP__END:
	POP TEMP_L
	POP TEMP_H
	POP ZL
	POP XL
	RET
.endif
