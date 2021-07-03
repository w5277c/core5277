;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;06.02.2021	w5277c@gmail.com			Начало
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_MOV
.else
.set DEF_PRIM_MOV = 1

.if REPORT_INCLUDES == 0x01
.message "included PRIM_MOV"
.endif

.include	"./prim/_offsets.inc"

;--------------------------------------------------------
PRIM_MOV:
;--------------------------------------------------------
;Запись значения
;IN: Y-адрес на память с переменными, TEMP_H-смещение для
;записи значения, TEMP_L-смещение на значение,
;ACCUM-тип примитива
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH_X
	PUSH_Z

	MCALL _PRIM_OFFSETS_L

	MOV LOOP_CNTR,ACCUM
	ANDI LOOP_CNTR,0x0f
__PRIM_MOV__LOOP:
	LD TEMP,X+
	ST Z+,TEMP
	DEC LOOP_CNTR
	BRNE __PRIM_MOV__LOOP

	POP_Z
	POP_X
	POP LOOP_CNTR
	POP TEMP
	RET
.endif
