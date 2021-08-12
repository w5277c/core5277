;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020	w5277c@gmail.com			Начало
;01.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;03.07.2021	w5277c@gmail.com			Оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5__RAM_RESIZE
.else
.set DEF_C5__RAM_RESIZE = 1

.include	"./core/ram/_ram_find_used_block_mt.inc"
.include	"./core/ram/_ram_find_used_block_lt.inc"
.include	"./core/ram/_ram_find_free_block.inc"

;--------------------------------------------------------
_C5_RAM_RESIZE:
;--------------------------------------------------------
;Изменение размера выделенного блока
;IN:Y-адрес блока, TEMP-новый размер
;OUT:Y-новый адрес блока
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM

	MOVW XL,YL

	;Находим следующий блок
	MCALL _C5_RAM_FIND_USED_BLOCK_MT
	;Z=true - блок найден, иначе находимся на вершине
	BREQ PC+0x04
	MOVW YL,XL
	MOV ACCUM,TEMP
	RJMP _C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP

	;Считаем дельту между блоками
	SUB YL,XL
	SBC YH,XH
	CPI YH,0x00
	BRNE PC+0x03
	CP YL,TEMP
	BRCS PC+0x04
	MOVW YL,XL
	MOV ACCUM,TEMP
	;Свободной памяти между текущими блоками достаточно для расширения
	BRNE _C5_RAM_RESIZE__SUCCESS

	;Выполняем поиск свободного блока
	MOV ACCUM,TEMP
	MCALL _C5_RAM_FIND_FREE_BLOCK
	;Если даже блок не найден, то процедура возвращает адрес вершины блоков

	;Копируем старый блок на новое место
	PUSH LOOP_CNTR
	MOV LOOP_CNTR,ACCUM
	MOVW ZL,YL
	MCALL RAM_COPY8
	POP LOOP_CNTR

_C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP:
	MCALL _C5_RAM_CHECK_STACK
	BRNE _C5_RAM_RESIZE__ERROR


	;Проверяем на полное освобождение (нужно учесть, что перед текущим блоком может быть пустое пространство)
	CPI ACCUM,0x00
	BRNE _C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
	MCALL _C5_RAM_FIND_USED_BLOCK_LT
	BREQ _C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
	;Блок не найден, значит восстанавливаю изначальное состояние
	LDI_Y _C5_FREE_RAM
	RJMP _C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP2
_C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP__HAVE_BLOCK:

_C5_RAM_RESIZE__HEADER_UPDATE_ON_TOP2:
	;Обновляем вершину безусловно, так как мы на вершине
	MOVW XL,YL
	ADD XL,ACCUM
	ADC XH,C0x00
	STS _C5_TOP_OF_FREE_RAM+0x00,XH
	STS _C5_TOP_OF_FREE_RAM+0x01,XL

_C5_RAM_RESIZE__SUCCESS:
	SEZ
	RJMP _C5_RAM_RESIZE__END
_C5_RAM_RESIZE__ERROR:
	CLZ
_C5_RAM_RESIZE__END:
	POP ACCUM
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
