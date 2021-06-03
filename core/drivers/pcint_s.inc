;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;25.05.2021	w5277c@gmail.com			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_PCINT_S
.else
.set DEF__C5_DRV_PCINT_S = 1
.message "Included driver software PCINT v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./mem/ram_fill8.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_PCINT_S_RAM_ITEMS_QNT					= 0x00	;1B - количество портов
	.EQU	_DRV_PCINT_S_RAM_ITEMS_DATA				= 0x01	;xB - структура данных для каждого порта
	;---ITEM-STRUCT---
	.EQU	_DRV_PCINT_S_ITEM_PID						= 0x00	;1B - ид процесса
	.EQU	_DRV_PCINT_S_ITEM_PORT						= 0x01	;2B - порт(PINx+bit num)
	.EQU	_DRV_PCINT_S_ITEM_ADDR						= 0x03	;2B - адрес обработчика события
	.EQU	_DRV_PCINT_S_ITEM_LAST_STATE				= 0x05	;1B - последнее состояние (0x80 первый опрос)

;--------------------------------------------------------
DRV_PCINT_S_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: LOOP_CNTR-количество портов, FLAGS-ид таймера,
;ACCUM-периодичность таймера(см. C5_TIMER_SET)
;--------------------------------------------------------
	;Выделяем память (кол-во протов*6+1)
	PUSH ACCUM
	MOV ACCUM,LOOP_CNTR
	LSL ACCUM
	MOV TEMP,ACCUM
	LSL ACCUM
	ADD ACCUM,TEMP
	SUBI ACCUM,(0x100-0x01)
	MCALL C5_RAM_REALLOC
	;Записываем длину
	STD Z+_DRV_PCINT_S_RAM_ITEMS_QNT,LOOP_CNTR
	;Заполняем
	MOV XH,ZH
	MOV XL,ZL
	MOV LOOP_CNTR,ACCUM
	LDI TEMP,0xff
	MCALL RAM_FILL8

	POP ACCUM
	;Настраиваем и запускаем таймер
	MOV TEMP,FLAGS
	LDI TEMP_H,high(_DRV_PCINT_S_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_PCINT_S_TIMER_EVENT)
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	;Завершаем инициализацию
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, устанавливаем параметры порта
;IN: ACCUM-порт(7bit-подтяжка), TEMP_L-номер порта
;Y-адрес обработчика, FLAGS-PID(0xFF - отключить)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	;Номер прота*6+1
	LSL TEMP
	MOV TEMP,TEMP_L
	LSL TEMP
	ADD TEMP,TEMP_L
	SUBI TEMP,(0x100-0x01)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	;Настраиваем порт
	MOV TEMP,ACCUM
	ANDI ACCUM,0x7f
	MCALL PORT_MODE_IN
	LSL TEMP
	MCALL PORT_SET

	LDI TEMP,0xff
	ST Z+_DRV_PCINT_S_ITEM_PID,TEMP
	ST Z+_DRV_PCINT_S_ITEM_LAST_STATE,TEMP
	MCALL PORT_OFFSETS
	ST Z+_DRV_PCINT_S_ITEM_PORT+0x00,TEMP
	ST Z+_DRV_PCINT_S_ITEM_PORT+0x01,ACCUM
	ST Z+_DRV_PCINT_S_ITEM_ADDR+0x00,YH
	ST Z+_DRV_PCINT_S_ITEM_ADDR+0x01,YL
	ST Z+_DRV_PCINT_S_ITEM_PID,FLAGS

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_PCINT_S_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик таймера
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	LD FLAGS,Z+															;_DRV_PCINT_S_RAM_ITEMS_QNT
	MOV LOOP_CNTR,FLAGS
_DRV_PCINT_S_TIMER_EVENT__ITEMS_LOOP:
	LDD TEMP,Z+_DRV_PCINT_S_ITEM_PID
	CPI TEMP,0xff
	BREQ _DRV_PCINT_S_TIMER_EVENT__ITEMS_NEXT
	LDI YH,0x00
	LDD YL,Z+_DRV_PCINT_S_ITEM_PORT+0x00
	LDS TEMP_H,Y
	AND TEMP_H,ACCUM
	LDD TEMP_L,Z+_DRV_PCINT_S_ITEM_LAST_STATE
	LDD ACCUM,Z+_DRV_PCINT_S_ITEM_PORT+0x01
	CPI TEMP_L,0xff
	BRNE PC+0x03
	STD Z+_DRV_PCINT_S_ITEM_LAST_STATE,TEMP_H
	RJMP _DRV_PCINT_S_TIMER_EVENT__ITEMS_NEXT
	EOR TEMP_L,TEMP_H
	BREQ _DRV_PCINT_S_TIMER_EVENT__ITEMS_NEXT
	STD Z+_DRV_PCINT_S_ITEM_LAST_STATE,TEMP_H
	CLR ACCUM
	CPI TEMP_H,0x00
	BRNE PC+0x02
	LDI ACCUM,0x01
	PUSH_Z
	PUSH FLAGS
	LDD TEMP,Z+_DRV_PCINT_S_ITEM_ADDR+0x00
	LDD ZL,Z+_DRV_PCINT_S_ITEM_ADDR+0x01
	MOV ZH,TEMP
	SUB FLAGS,LOOP_CNTR
	ICALL																	;FLAGS-состояние пина(0bit), accum-номер
	POP FLAGS
	POP_Z
_DRV_PCINT_S_TIMER_EVENT__ITEMS_NEXT:
	ADIW ZL,0x06
	DEC LOOP_CNTR
	BRNE _DRV_PCINT_S_TIMER_EVENT__ITEMS_LOOP

	POP LOOP_CNTR
	POP TEMP_L
	POp TEMP_H
	POP TEMP
	POP ACCUM
	POP_Z
	POP_Y
	RET
.endif
