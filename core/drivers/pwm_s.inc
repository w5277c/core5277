;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;30.10.2021	w5277c@gmail.com			Начало, не тестировано
;31.11.2021	w5277c@gmail.com			Дписан код, не тестировано
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_PWM_S
.else
.set DEF__C5_DRV_PWM_S = 1
.message "Included driver software PWM v0.1"

.include	"./mem/ram_fill.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_offsets.inc"
.include	"./io/reg_bit_hi.inc"
.include	"./io/reg_bit_lo.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_PWM_S_RAM_CHNLS_QNT				= 0x00		;1B - количество каналов
	.EQU	_DRV_PWM_S_RAM_CNTR						= 0x01		;1B - счетчик
	.EQU	_DRV_PWM_S_RAM_CNTR_TOP					= 0x02		;1B - верхний предел счетчика(максимальная мощность)
	.EQU	_DRV_PWM_S_RAM_ITEMS_DATA				= 0x03		;xB - структура данных для каждого канала
	;---ITEM-STRUCT---
	.EQU	_DRV_PWM_S_ITEM_PA						= 0x00		;1B - адрес PORT
	.EQU	_DRV_PWM_S_ITEM_BN						= 0x01		;1B - номер бита
	.EQU	_DRV_PWM_S_ITEM_THRLD					= 0x02		;1B - граница срабатывания
	;---OPERATIONS---
	.EQU	DRV_PWM_S_OP_PORT_SET					= 0x00		;Устанавливаем порт канала
	.EQU	DRV_PWM_S_OP_POWER_SET					= 0x01		;Устанавливаем мощность канала

;--------------------------------------------------------
DRV_PWM_S_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: LOOP_CNTR-количество каналов, TEMP_H-ид таймера,
;TEMP_L-периодичность таймера(см. C5_TIMER_SET),
;TEMP-верхний предел счетчика(максимальная мощность)
;--------------------------------------------------------
	;Выделяем память (кол-во каналов*4+2)
	MOV ACCUM,LOOP_CNTR
	LSL ACCUM
	ADD ACCUM,LOOP_CNTR
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

	CPI FLAGS,DRV_PWM_S_OP_PORT_SET
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_PWM_S_OP_PORT_SET
	RJMP _DRV_PWM_S__PROC_END
	CPI FLAGS,DRV_PWM_S_OP_POWER_SET
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

	MOV TEMP,TEMP_L
	LSL TEMP_L
	ADD TEMP_L,TEMP
	SUBI TEMP_L,(0x100-_DRV_PWM_S_RAM_ITEMS_DATA)
	ADD YL,TEMP_L
	ADC YH,C0x00

	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_PWM_S_ITEM_PA,TEMP_H
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
	PUSH_Z

	MOV TEMP,TEMP_L
	LSL TEMP_L
	ADD TEMP_L,TEMP
	SUBI TEMP_L,(0x100-_DRV_PWM_S_RAM_ITEMS_DATA)
	MOVW ZL,YL
	ADD ZL,TEMP_L
	ADC ZH,C0x00

	STD Z+_DRV_PWM_S_ITEM_THRLD,TEMP_H
	STD Y+_DRV_PWM_S_RAM_CNTR,C0x00

	POP_Z
	POP TEMP_L
	RET

;--------------------------------------------------------
_DRV_PWM_S_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру
;*обработчики программных таймеров могут не сохранять
;значения регистров Y,Z
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH ACCUM

	LDD LOOP_CNTR,Y+_DRV_PWM_S_RAM_CHNLS_QNT
	CPI LOOP_CNTR,0x00
	BREQ _DRV_PWM_S_TIMER_EVENT__END

	MOVW ZL,YL
	LDI TEMP,_DRV_PWM_S_RAM_ITEMS_DATA
	ADD ZL,TEMP
	ADC ZH,C0x00

	LDD TEMP_H,Y+_DRV_PWM_S_RAM_CNTR_TOP
	LDD TEMP_L,Y+_DRV_PWM_S_RAM_CNTR
	CPI TEMP_L,0x00
	BRNE _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP
_DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP:
	LDD TEMP,Z+_DRV_PWM_S_ITEM_THRLD
	CP TEMP,TEMP_L
	BREQ PC+0x05+_MCALL_SIZE
	LDD TEMP,Z+_DRV_PWM_S_ITEM_PA
	LDD ACCUM,Z+_DRV_PWM_S_ITEM_BN
	CPI ACCUM,0x00
	BREQ PC+0x01+_MCALL_SIZE
	MCALL REG_BIT_HI
	ADIW ZL,0x03
	DEC LOOP_CNTR
	BRNE _DRV_PWM_S_TIMER_EVENT__POWER_ON_LOOP
	RJMP _DRV_PWM_S_TIMER_EVENT__DONE

_DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP:
	LDD TEMP,Z+_DRV_PWM_S_ITEM_THRLD
	CP TEMP,TEMP_L
	BRNE PC+0x07+_MCALL_SIZE
	CP TEMP,TEMP_H
	BREQ PC+0x05+_MCALL_SIZE
	LDD TEMP,Z+_DRV_PWM_S_ITEM_PA
	LDD ACCUM,Z+_DRV_PWM_S_ITEM_BN
	CPI ACCUM,0x00
	BREQ PC+0x01+_MCALL_SIZE
	MCALL REG_BIT_LO
	ADIW ZL,0x03
	DEC LOOP_CNTR
	BRNE _DRV_PWM_S_TIMER_EVENT__CNTR_CHECK_LOOP

_DRV_PWM_S_TIMER_EVENT__DONE:
	INC TEMP_L
	CP TEMP_H,TEMP_L
	BRNE PC+0x02
	CLR TEMP_L
	STD Y+_DRV_PWM_S_RAM_CNTR,TEMP_L

_DRV_PWM_S_TIMER_EVENT__END:
	POP ACCUM
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
.endif
