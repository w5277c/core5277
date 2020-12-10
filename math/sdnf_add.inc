;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;01.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;10.12.2020  w5277c@gmail.com			-с5
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_SDNF_ADD
.else
.set DEF_SDNF_ADD = 1

;--------------------------------------------------------
SDNF_ADD:																;Signed Digital aNd Fractional
;--------------------------------------------------------
;Сумма знаковых целых с дробной частью (X + Y)
;IN: X, Y
;OUT: X, flag C - переполнение
;-START--------------------------------------------------
	PUSH_Y
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	;Проверка на корректность значений
	CPI XH,0xff
	BRNE PC+0x03
	CPI XL,0xff
	BREQ _SDNF_ADD__ERROR
	CPI YH,0xff
	BRNE PC+0x03
	CPI YL,0xff
	BREQ _SDNF_ADD__ERROR
	;Выношу знаки в отдельные регистры
	MOV TEMP_H,XL
	ANDI TEMP_H,0x80
	MOV TEMP_L,YL
	ANDI TEMP_L,0x80
	ANDI XL,0x7f
	ANDI YL,0x7f
	CPI XL,100
	BRCC _SDNF_ADD__ERROR
	CPI YL,100
	BRCC _SDNF_ADD__ERROR

	;Проверяю знаки, выполняю сложение, если знаки одинаковые
	MOV TEMP,TEMP_H
	EOR TEMP,TEMP_L
	BREQ _SDNF_ADD__ADD
	;Иначе вычитание, но сначала нужно перенести большее по модулю в X
	CP XH,YH
	BRCC _SDNF_ADD_L1
	MOV TEMP,XH
	MOV XH,YH
	MOV YH,TEMP
	MOV TEMP,XL
	MOV XL,YL
	MOV YL,TEMP
	MOV TEMP,TEMP_H
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,TEMP
	RJMP _SDNF_ADD__SUB
_SDNF_ADD_L1:
	CP XL,YL
	BRCC PC+0x0a
	MOV TEMP,XH
	MOV XH,YH
	MOV YH,TEMP
	MOV TEMP,XL
	MOV XL,YL
	MOV YL,TEMP
	MOV TEMP,TEMP_H
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,TEMP
_SDNF_ADD__SUB:
	;Вычитаю
	SUB XL,YL
	BRCC PC+0x04
	LDI TEMP,(256-100)
	SUB XL,TEMP
	SEC
	SBC XH,YH
	OR XL,TEMP_H														;Восстанавливаю знак
	RJMP _SDNF_ADD__END
_SDNF_ADD__ADD:
	LDI TEMP,99
	ADD XL,YL
	CP TEMP,XL
	BRCC PC+0x03
	SUBI XL,100
	SEC
	ADC XH,YH
	OR XL,TEMP_H														;Восстанавливаю знак
	RJMP _SDNF_ADD__END
_SDNF_ADD__ERROR:
	LDI XH,0xff
	LDI XL,0xff
_SDNF_ADD__END:
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Y
	RET
.endif
