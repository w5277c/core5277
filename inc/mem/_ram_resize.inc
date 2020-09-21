;-----------------------------------------------------------------------------------------------------------------------
;Владельцем данного исходного кода является Удовиченко Константин Александрович, емайл:w5277c@gmail.com, по всем
;правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_CORE5277__RAM_RESIZE
.else
.set DEF_CORE5277__RAM_RESIZE = 1

.include	"./inc/mem/_ram_find_used_block_mt.inc"
.include	"./inc/mem/_ram_find_used_block_lt.inc"
.include	"./inc/mem/_ram_find_free_block.inc"

;--------------------------------------------------------
_CORE5277_RAM_RESIZE:
;--------------------------------------------------------
;Изменение размера выделенного блока
;IN:Y - адрес блока, TEMP - новый размер
;OUT:Y - новый адрес блока
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MOV XH,YH
	MOV XL,YL

	;Находим следующий блок
	MCALL _CORE5277_RAM_FIND_USED_BLOCK_MT
	;Z=true - блок найден, иначе находимся на вершине
	BREQ PC+0x05
	MOV YH,XH
	MOV YL,XL
	MOV ACCUM,TEMP
	RJMP _CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP

	;Считаем дельту между блоками
	SUB YL,XL
	SBC YH,XH
	CPI YH,0x00
	BRNE PC+0x03
	CP YL,TEMP
	BRCS PC+0x05
	MOV YH,XH
	MOV YL,XL
	MOV ACCUM,TEMP
	;Свободной памяти между текущими блоками достаточно для расширения
	BRNE _CORE5277_RAM_RESIZE__SUCCESS

	;Выполняем поиск свободного блока
	MOV ACCUM,TEMP
	MCALL _CORE5277_RAM_FIND_FREE_BLOCK
	;Если даже блок не найден, то процедура возвращает адрес вершины блоков

	;Копируем старый блок на новое место
	PUSH LOOP_CNTR
	MOV LOOP_CNTR,ACCUM
	MOV ZH,YH
	MOV ZL,YL
	MCALL CORE5277_RAM_COPY8
	POP LOOP_CNTR

_CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP:
	MCALL _CORE5277_RAM_CHECK_STACK
	BRNE _CORE5277_RAM_RESIZE__ERROR


	;Проверяем на полное освобождение (нужно учесть, что перед текущим блоком может быть пустое пространство)
	CPI ACCUM,0x00
	BRNE _CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
	MCALL _CORE5277_RAM_FIND_USED_BLOCK_LT
	BREQ _CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
	;Блок не найден, значит восстанавливаю изначальное состояние
	LDI YH,high(_CORE5277_FREE_RAM)
	LDI YL,low(_CORE5277_FREE_RAM)
	RJMP _CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
_CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP__HAVE_BLOCK:

_CORE5277_RAM_RESIZE__HEADER_UPDATE_ON_TOP2:
	;Обновляем вершину безусловно, так как мы на вершине
	MOV XH,YH
	MOV XL,YL
	ADD XL,ACCUM
	LDI ACCUM,0x00
	ADC XH,ACCUM
	STS _CORE5277_TOP_OF_FREE_RAM+0x00,XH
	STS _CORE5277_TOP_OF_FREE_RAM+0x01,XL

_CORE5277_RAM_RESIZE__SUCCESS:
	SEZ
	RJMP _CORE5277_RAM_RESIZE__END
_CORE5277_RAM_RESIZE__ERROR:
	CLZ
_CORE5277_RAM_RESIZE__END:
	POP ACCUM
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
