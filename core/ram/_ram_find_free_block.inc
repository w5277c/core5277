;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;28.05.2020  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RAM_FIND_FREE_BLOCK
.else
.set DEF_C5_RAM_FIND_FREE_BLOCK = 1

.include	"./core/ram/_ram_find_used_block_mt.inc"
.include	"./core/ram/_ram_check_stack.inc"

;--------------------------------------------------------
_C5_RAM_FIND_FREE_BLOCK:
;--------------------------------------------------------
;Ищем блок свободной памяти запрошенного размера
;IN: ACCUM - размер запрошенного блока
;OUT: Y - адрес блока, flag Z - результат(true-найдено)
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI YH,high(_C5_FREE_RAM-0x01)
	LDI YL,low(_C5_FREE_RAM-0x01)
	LDI TEMP,0x00
	LDI LOOP_CNTR,0x00

_C5_RAM_FIND_FREE_BLOCK__LOOP:
	MOV LOOP_CNTR,TEMP
	MOV XH,YH
	MOV XL,YL
	PUSH ACCUM
	MCALL _C5_RAM_FIND_USED_BLOCK_MT
	MOV TEMP,ACCUM
	POP ACCUM
	BRNE _C5_RAM_FIND_FREE_BLOCK__ON_TOP
	MOV ZH,YH
	MOV ZL,YL
	SUB ZL,XL
	SBC ZH,XH															;!!! не влияет на флаг Z
	CPI ZH,0x00
	BRNE _C5_RAM_FIND_FREE_BLOCK__GOT_BLOCK
	SUB ZL,ACCUM
	BRCS _C5_RAM_FIND_FREE_BLOCK__LOOP
	SUB ZL,LOOP_CNTR
	BRCC _C5_RAM_FIND_FREE_BLOCK__GOT_BLOCK
	RJMP _C5_RAM_FIND_FREE_BLOCK__LOOP
_C5_RAM_FIND_FREE_BLOCK__GOT_BLOCK:
	MOV YH,XH
	MOV YL,XL
	MCALL _C5_RAM_CHECK_STACK
	RJMP _C5_RAM_FIND_FREE_BLOCK__END
_C5_RAM_FIND_FREE_BLOCK__ON_TOP:
	LDS YH,_C5_TOP_OF_FREE_RAM+0x00
	LDS YL,_C5_TOP_OF_FREE_RAM+0x01
	MCALL _C5_RAM_CHECK_STACK
	RJMP _C5_RAM_FIND_FREE_BLOCK__END
_C5_RAM_FIND_FREE_BLOCK__FAIL:
	CLZ
_C5_RAM_FIND_FREE_BLOCK__END:
	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
