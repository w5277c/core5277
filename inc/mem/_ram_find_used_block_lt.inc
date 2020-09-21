;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277_RAM_FIND_USED_BLOCK_LT
.else
.set DEF_CORE5277_RAM_FIND_USED_BLOCK_LT = 1

;--------------------------------------------------------
_CORE5277_RAM_FIND_USED_BLOCK_LT:
;--------------------------------------------------------
;Ищем ближайший занятый блок выделяемой памяти в задачах,
;адрес которого будет меньше, чем указанный адрес
;IN: Y - адрес текущего блока
;OUT: Y - адрес найденного блока, ACCUM - длина блока,
;flag Z - результат(true-найдено)
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI XH,0x00
	LDI XL,0x00

	LDI LOOP_CNTR,0x00
	LDI ZH,high(_CORE5277_DRIVERS_HEADER)
	LDI ZL,low(_CORE5277_DRIVERS_HEADER)
_CORE5277_RAM_FIND_USED_BLOCK_LT__LOOP1:
	MCALL _CORE5277_RAM_FIND_USED_BLOCK_LT__BODY
	LDI TEMP,_CORE5277_DRIVER_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_CORE5277_DRIVERS_MAX_QNT
	BRNE _CORE5277_RAM_FIND_USED_BLOCK_LT__LOOP1

	LDI LOOP_CNTR,0x00
	LDI ZH,high(_CORE5277_TASKS_HEADER)
	LDI ZL,low(_CORE5277_TASKS_HEADER)
_CORE5277_RAM_FIND_USED_BLOCK_LT__LOOP2:
	MCALL _CORE5277_RAM_FIND_USED_BLOCK_LT__BODY
	LDI TEMP,_CORE5277_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,_CORE5277_TASKS_MAX_QNT
	BRNE _CORE5277_RAM_FIND_USED_BLOCK_LT__LOOP2

	CPI XH,0xff
	BRNE _CORE5277_RAM_FIND_USED_BLOCK_LT__OK
	CPI XL,0xff
	BRNE _CORE5277_RAM_FIND_USED_BLOCK_LT__OK
	CLZ
	RJMP _CORE5277_RAM_FIND_USED_BLOCK_LT__END
_CORE5277_RAM_FIND_USED_BLOCK_LT__OK:
	MOV YH,XH
	MOV YL,XL
	SEZ
_CORE5277_RAM_FIND_USED_BLOCK_LT__END:
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_X
	RET

_CORE5277_RAM_FIND_USED_BLOCK_LT__BODY:
	;Проверяем на активную задачу
	LDD TEMP,Z+_CORE5277_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_CORE5277_PROC_STATE_ABSENT
	BREQ _CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT
	LDD TEMP_H,Z+_CORE5277_PROC_RAM_OFFSET+0x00
	LDD TEMP_L,Z+_CORE5277_PROC_RAM_OFFSET+0x01
	LDD TEMP,Z+_CORE5277_PROC_RAM_SIZE
	;Проверям, что адрес найденного блока меньше заданного
	CP YH,TEMP_H
	BRCS _CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT
	BRNE PC+0x03
	CP TEMP_L,YL
	BRCC _CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT
	;Заменяем результат, если полученный адрес больше предыдущего
	CP XH,TEMP_H
	BREQ PC+0x02
	BRCC _CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT
	CP XL,TEMP_L
	BRCC _CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT
	MOV XH,TEMP_H
	MOV XL,TEMP_L
	MOV ACCUM,TEMP
_CORE5277_RAM_FIND_USED_BLOCK_LT__NEXT:
	RET
.endif
