;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_FIND_USED_BLOCK_LT
.else
.set DEF_C5_RAM_FIND_USED_BLOCK_LT = 1

;--------------------------------------------------------
_C5_RAM_FIND_USED_BLOCK_LT:
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
	LDI ZH,high(_C5_DRIVERS_HEADER)
	LDI ZL,low(_C5_DRIVERS_HEADER)
_C5_RAM_FIND_USED_BLOCK_LT__LOOP1:
	MCALL _C5_RAM_FIND_USED_BLOCK_LT__BODY
	LDI TEMP,_C5_DRIVER_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,C5_DRIVERS_QNT
	BRNE _C5_RAM_FIND_USED_BLOCK_LT__LOOP1

	LDI LOOP_CNTR,0x00
	LDI ZH,high(_C5_TASKS_HEADER)
	LDI ZL,low(_C5_TASKS_HEADER)
_C5_RAM_FIND_USED_BLOCK_LT__LOOP2:
	MCALL _C5_RAM_FIND_USED_BLOCK_LT__BODY
	LDI TEMP,_C5_TASK_HEADER_SIZE
	ADD ZL,TEMP
	LDI TEMP,0x00
	ADC ZH,TEMP
	INC LOOP_CNTR
	CPI LOOP_CNTR,C5_TASKS_QNT
	BRNE _C5_RAM_FIND_USED_BLOCK_LT__LOOP2

	CPI XH,0xff
	BRNE _C5_RAM_FIND_USED_BLOCK_LT__OK
	CPI XL,0xff
	BRNE _C5_RAM_FIND_USED_BLOCK_LT__OK
	CLZ
	RJMP _C5_RAM_FIND_USED_BLOCK_LT__END
_C5_RAM_FIND_USED_BLOCK_LT__OK:
	MOV YH,XH
	MOV YL,XL
	SEZ
_C5_RAM_FIND_USED_BLOCK_LT__END:
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_X
	RET

_C5_RAM_FIND_USED_BLOCK_LT__BODY:
	;Проверяем на активную задачу
	LDD TEMP,Z+_C5_PROC_STATE
	ANDI TEMP,0x0f
	CPI TEMP,_C5_PROC_STATE_ABSENT
	BREQ _C5_RAM_FIND_USED_BLOCK_LT__NEXT
	LDD TEMP_H,Z+_C5_PROC_RAM_OFFSET+0x00
	LDD TEMP_L,Z+_C5_PROC_RAM_OFFSET+0x01
	LDD TEMP,Z+_C5_PROC_RAM_SIZE
	;Проверям, что адрес найденного блока меньше заданного
	CP YH,TEMP_H
	BRCS _C5_RAM_FIND_USED_BLOCK_LT__NEXT
	BRNE PC+0x03
	CP TEMP_L,YL
	BRCC _C5_RAM_FIND_USED_BLOCK_LT__NEXT
	;Заменяем результат, если полученный адрес больше предыдущего
	CP XH,TEMP_H
	BREQ PC+0x02
	BRCC _C5_RAM_FIND_USED_BLOCK_LT__NEXT
	CP XL,TEMP_L
	BRCC _C5_RAM_FIND_USED_BLOCK_LT__NEXT
	MOV XH,TEMP_H
	MOV XL,TEMP_L
	MOV ACCUM,TEMP
_C5_RAM_FIND_USED_BLOCK_LT__NEXT:
	RET
.endif
