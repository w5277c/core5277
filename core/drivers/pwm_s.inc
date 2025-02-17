;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.10.2021	w5277c@gmail.com			Начало, не тестировано
;31.11.2021	w5277c@gmail.com			Дописан код, не тестировано
;19.12.2024	konstantin@5277.ru			Оттестировано с применением VPORT+MCP23017
;18.02.2025	konstantin@5277.ru			Багфиксинг(ошибка в переходах)
;-----------------------------------------------------------------------------------------------------------------------
;DRV_OP_SET_PORT - установка порта,
;DRV_OP_SET - установка значение(от 0 до максимальная мощность)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_PWM_S
.else
.set DEF__C5_DRV_PWM_S = 1
.message "Included driver software PWM v0.2"

.include	"./mem/ram_fill.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/_port_offset.inc"
.include	"./io/reg_bit_hi.inc"
.include	"./io/reg_bit_lo.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_PWM_S_RAM_CHNLS_QNT			= 0x00	;1B - количество каналов
	.EQU	_DRV_PWM_S_RAM_CNTR					= 0x01	;1B - счетчик
	.EQU	_DRV_PWM_S_RAM_CNTR_TOP				= 0x02	;1B - верхний предел счетчика(максимальная мощность)
	.EQU	_DRV_PWM_S_RAM_ITEMS_DATA			= 0x03	;xB - структура данных для каждого канала
	;---ITEM-STRUCT---
	.EQU	_DRV_PWM_S_ITEM_PA					= 0x00	;2B - адрес PORT
	.EQU	_DRV_PWM_S_ITEM_BN					= 0x02	;1B - номер бита
	.EQU	_DRV_PWM_S_ITEM_THRLD				= 0x03	;1B - граница срабатывания

;--------------------------------------------------------
DRV_PWM_S_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: LOOP_CNTR-количество каналов, TEMP_H-ид таймера,
;TEMP_L-периодичность таймера(см. C5_TIMER_SET),
;TEMP-верхний предел счетчика(максимальная мощность)
;--------------------------------------------------------
	;Выделяем память (кол-во каналов*4+_DRV_PWM_S_RAM_ITEMS_DATA)
	MOV ACCUM,LOOP_CNTR
	LSL ACCUM
	LSL ACCUM
	SUBI ACCUM,(0x100-_DRV_PWM_S_RAM_ITEMS_DATA)
	MCALL C5_RAM_REALLOC

	;Записываем максимальное число для счетчиков
	STD Y+_DRV_PWM_S_RAM_CNTR_TOP,TEMP
	;Записываем кол-во каналов
	STD Y+_DRV_PWM_S_RAM_CHNLS_QNT,LOOP_CNTR
	;Устанавливаем счетчик в 0
	STD Y+_DRV_PWM_S_RAM_CNTR,C0x00

	;Заполняем
	MOVW XL,YL
	MOV LOOP_CNTR,ACCUM
	ADIW XL,_DRV_PWM_S_RAM_ITEMS_DATA
	SUBI LOOP_CNTR,_DRV_PWM_S_RAM_ITEMS_DATA
	LDI TEMP,0x00
	MCALL RAM_FILL

	;Настраиваем и запускаем таймер
	MOV TEMP,TEMP_H
	MOV ACCUM,TEMP_L
	LDI TEMP_H,high(_DRV_PWM_S_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_PWM_S_TIMER_EVENT)
	MCALL C5_TIMER_SET
	MCALL C5_TIMER_START

	;Завершаем инициализацию
	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_OP_SET_PORT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_PWM_S_OP_PORT_SET
	RJMP _DRV_PWM_S__PROC_END
	CPI FLAGS,DRV_OP_SET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_PWM_S_OP_POWER_SET
	RJMP _DRV_PWM_S__PROC_END

_DRV_PWM_S__PROC_END:
	POP TEMP
	POP_Y
	RET

;--------------------------------------------------------
_DRV_PWM_S_OP_PORT_SET:
;--------------------------------------------------------
;Устанавливаем порт канала
;IN: ACCUM-порт канала, TEMP_L-номер канала(0-...)
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	LSL TEMP_L
	LSL TEMP_L
	SUBI TEMP_L,(0x100-_DRV_PWM_S_RAM_ITEMS_DATA)
	ADD YL,TEMP_L
	ADC YH,C0x00

	MCALL _PORT_OFFSET
	ADIW ZL,0x02
	STD Y+_DRV_PWM_S_ITEM_PA+0x00,ZH
	STD Y+_DRV_PWM_S_ITEM_PA+0x01,ZL
	STD Y+_DRV_PWM_S_ITEM_BN,ACCUM
	STD Y+_DRV_PWM_S_ITEM_THRLD,C0x00

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	RET

;--------------------------------------------------------
_DRV_PWM_S_OP_POWER_SET:
;--------------------------------------------------------
;Устанавливаем мощность канала
;IN: TEMP_H-мощность канала, TEMP_L-номер канала(0-...)
;--------------------------------------------------------
	PUSH TEMP_L

	LSL TEMP_L
	LSL TEMP_L
	SUBI TEMP_L,(0x100-_DRV_PWM_S_RAM_ITEMS_DATA)
	ADD YL,TEMP_L
	ADC YH,C0x00
	STD Y+_DRV_PWM_S_ITEM_THRLD,TEMP_H

	POP TEMP_L
	RET

;--------------------------------------------------------
_DRV_PWM_S_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH ACCUM

	LDD LOOP_CNTR,Y+_DRV_PWM_S_RAM_CHNLS_QNT
	CPI LOOP_CNTR,0x00
	BREQ _DRV_PWM_S_TIMER_EVENT__END

	LDD TEMP_L,Y+_DRV_PWM_S_RAM_CNTR
	LDD ACCUM,Y+_DRV_PWM_S_RAM_CNTR_TOP
	MOV TEMP,TEMP_L
	INC TEMP
	CP ACCUM,TEMP
	BRNE PC+0x02
	CLR TEMP
	STD Y+_DRV_PWM_S_RAM_CNTR,TEMP

	ADIW YL,_DRV_PWM_S_RAM_ITEMS_DATA

	CPI TEMP_L,0x00										;Проверка на начало периода
	BRNE _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP

	PUSH_Y
	PUSH LOOP_CNTR
_DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP:
	LDD TEMP,Y+_DRV_PWM_S_ITEM_THRLD					;Включаем все порты если порог не 0
	CPI TEMP,0x00										;Порог=0, включать не нужно
	BREQ _DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP_NEXT
	LDD ZH,Y+_DRV_PWM_S_ITEM_PA+0x00
	LDD ZL,Y+_DRV_PWM_S_ITEM_PA+0x01
	LDD ACCUM,Y+_DRV_PWM_S_ITEM_BN
	CPI ACCUM,0x00										;=0 если порт не активирован
	BREQ _DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP_NEXT
	MCALL REG_BIT_HI_BY_Z
_DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP_NEXT:
	ADIW YL,0x04
	DEC LOOP_CNTR
	BRNE _DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP
	POP LOOP_CNTR
	POP_Y

_DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP:
	LDD TEMP,Y+_DRV_PWM_S_ITEM_THRLD
	CP TEMP,TEMP_L
	BRNE _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP_NEXT
	LDD ZH,Y+_DRV_PWM_S_ITEM_PA+0x00
	LDD ZL,Y+_DRV_PWM_S_ITEM_PA+0x01
	LDD ACCUM,Y+_DRV_PWM_S_ITEM_BN
	CPI ACCUM,0x00
	BREQ _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP_NEXT
	MCALL REG_BIT_LO_BY_Z
_DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP_NEXT:
	ADIW YL,0x04
	DEC LOOP_CNTR
	BRNE _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP

_DRV_PWM_S_TIMER_EVENT__END:
	POP ACCUM
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	RET
.endif
