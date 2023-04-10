;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.04.2023	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
;Пассивный buzzer(5кГц)
;DRV_OP_ON-включить,DRV_OP_OFF-выключить
;Частоту задаем через таймер
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_BUZZER
.else
.set DEF__C5_DRV_BUZZER = 1
.message "Included driver BUZZER v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./io/port_offsets.inc"
.include	"./io/reg_bit_hi.inc"
.include	"./io/reg_bit_set.inc"
.include	"./io/reg_bit_inv.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BUZZER_PA						= 0x00		;1B - адрес PORT
	.EQU	_DRV_BUZZER_BN						= 0x01		;1B - номер бита
	.EQU	_DRV_BUZZER_TIMER_ID				= 0x02		;1B - ид таймера

;--------------------------------------------------------
DRV_BUZZER_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: TEMP_H-ид таймера, TEMP_L-порт(7бит-инверсия порта)
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,0x03
	MCALL C5_RAM_REALLOC

	;Записываем ид таймера
	STD Y+_DRV_BUZZER_TIMER_ID,TEMP_H
	MOV TEMP,TEMP_H
	LDI TEMP_H,high(_DRV_BUZZER_TIMER_EVENT)
	PUSH TEMP_L
	LDI TEMP_L,low(_DRV_BUZZER_TIMER_EVENT)
	LDI ACCUM,0x04														;1/(0.000 050*2)/2 = 5кГц
	MCALL C5_TIMER_SET
	POP ACCUM
	PUSH ACCUM
	ANDI ACCUM,0x7f
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	POP TEMP_L
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_L
	LSL TEMP_L
	MCALL REG_BIT_SET													;Порт в LO, если 7бит выключен, иначе в HI(т.е. питание выключено)
	ANDI TEMP,0x80
	OR TEMP_H,TEMP														;Запоминаем старший бит в PA
	STD Y+_DRV_BUZZER_PA,TEMP_H
	STD Y+_DRV_BUZZER_BN,ACCUM

	;Завершаем инициализацию
	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции
;--------------------------------------------------------
	PUSH_Y
	PUSH TEMP

	MCALL C5_RAM_OFFSET

	CPI FLAGS,DRV_OP_ON
	BRNE _DRV_BUZZER__NO_ON
	LDD TEMP,Y+_DRV_BUZZER_TIMER_ID
	MCALL C5_TIMER_START
	RJMP _DRV_BUZZER__PROC_END
_DRV_BUZZER__NO_ON:
	CPI FLAGS,DRV_OP_OFF
	BRNE _DRV_BUZZER__PROC_END
	LDD TEMP,Y+_DRV_BUZZER_TIMER_ID
	MCALL C5_TIMER_STOP
	LDD TEMP,Y+_DRV_BUZZER_PA
	LSL TEMP
	MCALL REG_BIT_SET													;Порт в LO, если 7бит выключен, иначе в HI(т.е. питание выключено)

_DRV_BUZZER__PROC_END:
	POP TEMP
	POP_Y
	RET

;--------------------------------------------------------
_DRV_BUZZER_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP
	LDD TEMP,Y+_DRV_BUZZER_PA
	ANDI TEMP,0x7f
	LDD ACCUM,Y+_DRV_BUZZER_BN
	MCALL REG_BIT_INV
	POP TEMP
	POP ACCUM
	RET
.endif
