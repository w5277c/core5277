;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_SDNF_CP
.else
.set DEF_C5_SDNF_CP = 1

;--------------------------------------------------------
C5_SDNF_CP:														;Signed Digital aNd Fractional
;--------------------------------------------------------
;Сравнение двух знаковых целых с дробной частью (X < Y)
;IN: X, Y
;OUT: флаг С(A<B), флаг Z(A=B), ошибка - флаги включены
;-START--------------------------------------------------
	PUSH XL
	PUSH YL
	PUSH TEMP_H
	PUSH TEMP_L
	;Проверка на корректность значений
	CPI XH,0xff
	BRNE PC+0x03
	CPI XL,0xff
	BREQ _C5_SDNF_CP__ERROR
	CPI YH,0xff
	BRNE PC+0x03
	CPI YL,0xff
	BREQ _C5_SDNF_CP__ERROR
	;Выношу знаки в отдельные регистры
	MOV TEMP_H,XL
	ANDI TEMP_H,0x80
	MOV TEMP_L,YL
	ANDI TEMP_L,0x80
	ANDI XL,0x7f
	ANDI YL,0x7f
	CPI XL,100
	BRCC _C5_SDNF_CP__ERROR
	CPI YL,100
	BRCC _C5_SDNF_CP__ERROR

	;Проверяю целую часть
	CP XH,YH
	BREQ _C5_SDNF_CP__H_EQUAL
	BRCS PC+0x04
	SBRC TEMP_H,0x07													;Включаю флаг C, если |X|>|Y| и X<0
	SEC
	RJMP _C5_SDNF_CP__END
	SBRC TEMP_L,0x07													;Выключаю флаг C, если |X|<|Y| и Y<0
	CLC
	RJMP _C5_SDNF_CP__END
_C5_SDNF_CP__H_EQUAL:
	;проверяю дробную часть
	CP XL,YL
	BREQ _C5_SDNF_CP__L_EQUAL
	BRCS PC+0x04
	SBRC TEMP_H,0x07													;Включаю флаг C, если |X|>|Y| и X<0
	SEC
	RJMP _C5_SDNF_CP__END
	SBRC TEMP_L,0x07													;Выключаю флаг C, если |X|<|Y| и Y<0
	CLC
	RJMP _C5_SDNF_CP__END
_C5_SDNF_CP__L_EQUAL:
	EOR TEMP_L,TEMP_H
	BREQ _C5_SDNF_CP__EQUAL
	ROL TEMP_H															;Определяю флаг C, если |X|=|Y| и X<0
	CLZ
	RJMP _C5_SDNF_CP__END
_C5_SDNF_CP__ERROR:
	SEC
	SEZ
	RJMP _C5_SDNF_CP__END
_C5_SDNF_CP__EQUAL:
	CLC
	SEZ
	RJMP _C5_SDNF_CP__END
_C5_SDNF_CP__END:
	POP TEMP_L
	POP TEMP_H
	POP YL
	POP XL
	RET
.endif
