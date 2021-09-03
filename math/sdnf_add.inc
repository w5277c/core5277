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
.ifdef DEF_SDNF_ADD
.else
.set DEF_SDNF_ADD = 1

.if REPORT_INCLUDES == 0x01
.message "included SDNF_ADD"
.endif

;--------------------------------------------------------
SDNF_ADD:																;Signed Digital aNd Fractional
;--------------------------------------------------------
;Сумма знаковых целых с дробной частью (X + Z)
;IN: X, Z
;OUT: X, flag C-переполнение
;-START--------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP

	;Проверка на корректность значений
	CPI XL,0xff
	BREQ _SDNF_ADD__ERROR
	CPI ZL,0xff
	BREQ _SDNF_ADD__ERROR
	;Выношу знаки в отдельные регистры
	MOV TEMP_H,XL
	ANDI TEMP_H,0x80
	MOV TEMP_L,ZL
	ANDI TEMP_L,0x80
	ANDI XL,0x7f
	ANDI ZL,0x7f
	CPI XL,0x64
	BRCC _SDNF_ADD__ERROR
	CPI ZL,0x64
	BRCC _SDNF_ADD__ERROR

	;Проверяю знаки, выполняю сложение, если знаки одинаковые
	MOV TEMP,TEMP_H
	EOR TEMP,TEMP_L
	BREQ _SDNF_ADD__ADD
	;Иначе вычитание, но сначала нужно перенести большее по модулю в X
	CP XH,ZH
	BREQ _SDNF_ADD_L1
	BRCC _SDNF_ADD__SUB
	MOV TEMP,XH
	MOV XH,ZH
	MOV ZH,TEMP
	MOV TEMP,XL
	MOV XL,ZL
	MOV ZL,TEMP
	MOV TEMP,TEMP_H
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,TEMP
	RJMP _SDNF_ADD__SUB
_SDNF_ADD_L1:
	CP XL,ZL
	BRCC _SDNF_ADD__SUB
	MOV TEMP,XH
	MOV XH,ZH
	MOV ZH,TEMP
	MOV TEMP,XL
	MOV XL,ZL
	MOV ZL,TEMP
	MOV TEMP,TEMP_H
	MOV TEMP_H,TEMP_L
	MOV TEMP_L,TEMP
_SDNF_ADD__SUB:
	;Вычитаю
	SUB XL,ZL
	BRCC PC+0x04
	LDI TEMP,(256-100)
	SUB XL,TEMP
	SEC
	SBC XH,ZH
	OR XL,TEMP_H														;Восстанавливаю знак
	RJMP _SDNF_ADD__END
_SDNF_ADD__ADD:
	LDI TEMP,99
	ADD XL,ZL
	CP TEMP,XL
	BRCC PC+0x03
	SUBI XL,0x64
	SEC
	ADC XH,ZH
	OR XL,TEMP_H														;Восстанавливаю знак
	RJMP _SDNF_ADD__END
_SDNF_ADD__ERROR:
	LDI XL,0xff
_SDNF_ADD__END:
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
