;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.04.2023	konstantin@5277.ru			Начало
;01.08.2023	konstantin@5277.ru			Черновой вариант
;06.08.2023	konstantin@5277.ru			Оттестирован DRV_OP_ASYNC_START и DRV_OP_SYNC_START без модуляции
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер BUZZER(активный и пассивный)
;DRV_OP_ON-включить,
;DRV_OP_OFF-выключить
;DRV_OP_SYNC_START-проиграть(ожидать завершения)
;DRV_OP_ASYNC_START-проиграть(без ожидания завершения)
;DRV_OP_STOP-завершить проигрование
;DRV_OP_BEEP-подать сигнал
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_BUZZER
.else
.set DEF__C5_DRV_BUZZER = 1
.message "Included driver BUZZER v0.2"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/timer_stop.inc"
.include	"./io/port_offsets.inc"
.include	"./io/reg_bit_hi.inc"
.include	"./io/reg_bit_sset.inc"
.include	"./io/reg_bit_inv.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_BUZZER_ST_READY				= 0x00				;Готов к работе
	.EQU	_DRV_BUZZER_ST_BUSY				= 0x01				;Занят проигрыванием

	.EQU	_DRV_BUZZER_PA						= 0x00		;1B - адрес PORT
	.EQU	_DRV_BUZZER_BN						= 0x01		;1B - номер бита
	.EQU	_DRV_BUZZER_STATE					= 0x02		;1B - состояние
	.EQU	_DRV_BUZZER_ASYNC_TIMER_ID		= 0x03		;1B - ид таймера для асинхронного вызова
	.EQU	_DRV_BUZZER_MOD_TIMER_ID		= 0x04		;1B - ид таймера для модуляции
	.EQU	_DRV_BUZZER_BIT_TIME				= 0x05		;1B - количество циклов для 1 бита
	.EQU	_DRV_BUZZER_BIT_TIME_CNTR		= 0x06		;1B - счетчик для цилов для 1 бита
	.EQU	_DRV_BUZZER_BIT_DATA				= 0x07		;2B - 16 бит сигнала
	.EQU	_DRV_BUZZER_BIT_CNTR				= 0x09		;1B - счетчик бит
	;---
	.EQU	_DRV_BUZZER_RAM_SIZE				= 0x0a

;--------------------------------------------------------
DRV_BUZZER_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
;IN: TEMP_EH-ид таймера для асинхронного вызова(
;0xff-без асинхронности),
;TEMP_EL-ид таймера для модуляции(0xff-без модуляции),
;TEMP_H-время выполнения 1 бита(X*20мс)
;TEMP_L-порт(7бит-инверсия порта)
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_BUZZER_RAM_SIZE
	MCALL C5_RAM_REALLOC

	STD Y+_DRV_BUZZER_ASYNC_TIMER_ID,TEMP_EH
	STD Y+_DRV_BUZZER_MOD_TIMER_ID,TEMP_EL
	STD Y+_DRV_BUZZER_BIT_TIME,TEMP_H

	PUSH TEMP_L
	MOV ACCUM,TEMP_L
	ANDI ACCUM,0x7f
	MCALL PORT_OFFSETS
	POP TEMP
	SBRC TEMP,0x07
	ORI TEMP_H,0x80
	PUSH TEMP
	STD Y+_DRV_BUZZER_PA,TEMP_H
	STD Y+_DRV_BUZZER_BN,ACCUM
	MOV TEMP,TEMP_L													;OUT
	MCALL REG_BIT_HI
	MOV TEMP,TEMP_H
	CLC
	POP TEMP
	MCALL REG_BIT_SSET

	CPI TEMP_EH,0xff
	BREQ _DRV_BUZZER_INIT__SKIP_ASYNC_TIMER
	;Инициируем таймер отсчета времени выполнения 1 бита
	LDI TEMP_H,high(_DRV_BUZZER_ASYNC_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_BUZZER_ASYNC_TIMER_EVENT)
	LDI ACCUM,(20/2)|0x80											;20мс(0.002*10)
	MOV TEMP,TEMP_EH
	MCALL C5_TIMER_SET
_DRV_BUZZER_INIT__SKIP_ASYNC_TIMER:

	CPI TEMP_EL,0xff
	BREQ _DRV_BUZZER_INIT__SKIP_MOD_TIMER
	;Инициируем таймер модуляции
	LDI TEMP_H,high(_DRV_BUZZER_MOD_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_BUZZER_MOD_TIMER_EVENT)
	LDI ACCUM,0x04														;1/(0.000 050*2)/2 = 5кГц
	MOV TEMP,TEMP_EL
	MCALL C5_TIMER_SET
_DRV_BUZZER_INIT__SKIP_MOD_TIMER:


	LDI TEMP,_DRV_BUZZER_ST_READY
	STD Y+_DRV_BUZZER_STATE,TEMP
	;Завершаем инициализацию
	MCALL C5_READY

;--------------------------------------------------------
;Основной код
;IN: FLAGS-тип операции(см. заголовок файла)
;OUT: TEMP-результат(DRV_RESULT_...)
;--------------------------------------------------------
	PUSH_Y

	MCALL C5_RAM_OFFSET

	LDD TEMP,Y+_DRV_BUZZER_STATE
	CPI TEMP,_DRV_BUZZER_ST_READY
	BRNE _DRV_BUZZER__BUSY
	CPI FLAGS,DRV_OP_ON
	BRNE PC+0x02+_MCALL_SIZE+0x01
	LDI FLAGS,0x01
	MCALL _DRV_BUZZER_OP_SET
	RJMP _DRV_BUZZER__PROC_OK
	CPI FLAGS,DRV_OP_OFF
	BRNE PC+0x02+_MCALL_SIZE+0x01
	LDI TEMP,0x00
	MCALL _DRV_BUZZER_OP_SET
	RJMP _DRV_BUZZER__PROC_OK
	CPI FLAGS,DRV_OP_SYNC_START
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUZZER_OP_SYNC_START
	RJMP _DRV_BUZZER__PROC_OK
	CPI FLAGS,DRV_OP_ASYNC_START
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUZZER_OP_ASYNC_START
	RJMP _DRV_BUZZER__PROC_END
	CPI FLAGS,DRV_OP_BEEP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUZZER_OP_BEEP
	RJMP _DRV_BUZZER__PROC_OK

	RJMP _DRV_BUZZER__UNSUPPORTED
_DRV_BUZZER__BUSY:
	CPI FLAGS,DRV_OP_STOP
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUZZER_OP_STOP
	RJMP _DRV_BUZZER__PROC_OK

_DRV_BUZZER__UNSUPPORTED:
	LDI TEMP,DRV_RESULT_UNSUPPORTED
	RJMP _DRV_BUZZER__PROC_END
_DRV_BUZZER__PROC_OK:
	LDI TEMP,DRV_RESULT_OK
_DRV_BUZZER__PROC_END:
	POP_Y
	RET


;--------------------------------------------------------
_DRV_BUZZER_OP_BEEP:
;--------------------------------------------------------
;Издаем звуковой сигнал(ожидаем завершения)
;--------------------------------------------------------
	PUSH_T32
	PUSH TEMP

	LDI TEMP,0x01
	MCALL _DRV_BUZZER_OP_SET
	MCALL _DRV_BUZZER__WAIT
	LDI TEMP,0x00
	MCALL _DRV_BUZZER_OP_SET
	MCALL _DRV_BUZZER__WAIT

	POP TEMP
	POP_T32
	RET

;--------------------------------------------------------
_DRV_BUZZER_OP_SET:
;--------------------------------------------------------
;Управляем buzzero'ом
;IN: FLAGS 0-выкл, !0-вкл
;--------------------------------------------------------
	PUSH TEMP

	LDD TEMP,Y+_DRV_BUZZER_MOD_TIMER_ID
	CPI TEMP,0xFF
	BREQ _DRV_BUZZER_OP_SET__NO_TIMER
	CPI FLAGS,0x00
	BREQ PC+0x01+_MCALL_SIZE+0x01
	MCALL C5_TIMER_START
	RJMP _DRV_BUZZER_OP_SET__END
	MCALL C5_TIMER_STOP
	CLC
	MCALL _DRV_BUZZER_SET
	RJMP _DRV_BUZZER_OP_SET__END

_DRV_BUZZER_OP_SET__NO_TIMER:
	MOV TEMP,FLAGS
	ADD TEMP,C0xFF
	MCALL _DRV_BUZZER_SET

_DRV_BUZZER_OP_SET__END:
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUZZER_OP_SYNC_START:
;--------------------------------------------------------
;Воспроизводим последовательность(синхронно)
;IN: TEMP_H,TEMP_L-16 бит сигнала(каждый бит задает
;состояние buzzer в течении 1 периода
;LOOP_CNTR-количество используемых бит(1-16)
;--------------------------------------------------------
	PUSH_T32
	PUSH TEMP

	LDI TEMP,_DRV_BUZZER_ST_BUSY
	STD Y+_DRV_BUZZER_STATE,TEMP

_DRV_BUZZER_OP_SYNC_START__LOOP:
	LDI FLAGS,0x00
	LSL TEMP_L
	ROL TEMP_H
	ROL FLAGS
	MCALL _DRV_BUZZER_OP_SET
	PUSH TEMP_H
	PUSH TEMP_L
	MCALL _DRV_BUZZER__WAIT
	POP TEMP_L
	POP TEMP_H
	DEC LOOP_CNTR
	BRNE _DRV_BUZZER_OP_SYNC_START__LOOP

	MCALL _DRV_BUZZER_OP_STOP

	POP TEMP
	POP_T32
	RET


;--------------------------------------------------------
_DRV_BUZZER_OP_ASYNC_START:
;--------------------------------------------------------
;Воспроизводим последовательность(асинхронно)
;IN: TEMP_H,TEMP_L-16 бит сигнала(каждый бит задает
;состояние buzzer в течении 1 периода
;LOOP_CNTR-количество используемых бит(1-16)
;OUT: TEMP-результат(DRV_RESULT_...)
;--------------------------------------------------------
	LDI TEMP,_DRV_BUZZER_ST_BUSY
	STD Y+_DRV_BUZZER_STATE,TEMP

	LDD TEMP,Y+_DRV_BUZZER_ASYNC_TIMER_ID
	CPI TEMP,0xff
	BREQ _DRV_BUZZER_OP_ASYNC_START__FAIL

	STD Y+_DRV_BUZZER_BIT_DATA+0x00,TEMP_H
	STD Y+_DRV_BUZZER_BIT_DATA+0x01,TEMP_L
	STD Y+_DRV_BUZZER_BIT_CNTR,LOOP_CNTR
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,0x01
	STD Y+_DRV_BUZZER_BIT_TIME_CNTR,LOOP_CNTR
	POP LOOP_CNTR
	MCALL C5_TIMER_START

	LDI TEMP,DRV_RESULT_OK

	RJMP _DRV_BUZZER_OP_ASYNC_START__END
_DRV_BUZZER_OP_ASYNC_START__FAIL:
	LDI TEMP,DRV_RESULT_ABSENT

_DRV_BUZZER_OP_ASYNC_START__END:
	RET

;--------------------------------------------------------
_DRV_BUZZER_OP_STOP:
;--------------------------------------------------------
;Останавливаем воспроизведение последовательности
;--------------------------------------------------------
	LDI TEMP,_DRV_BUZZER_ST_READY
	STD Y+_DRV_BUZZER_STATE,TEMP

	LDD TEMP,Y+_DRV_BUZZER_ASYNC_TIMER_ID
	CPI TEMP,0xff
	BREQ PC+0x01+_MCALL_SIZE
	MCALL C5_TIMER_STOP
	CLC
	MCALL _DRV_BUZZER_SET

	RET

;--------------------------------------------------------
_DRV_BUZZER__WAIT:
;--------------------------------------------------------
;Выдерживаем паузу
;--------------------------------------------------------
	LDI TEMP_L,0x00
	LDD TEMP,Y+_DRV_BUZZER_BIT_TIME
	LSL TEMP																;*10
	ROL TEMP_L
	MOV TEMP_EH,TEMP_L
	MOV TEMP_EL,TEMP
	LSL TEMP
	ROL TEMP_L
	LSL TEMP
	ROL TEMP_L
	ADD TEMP,TEMP_EL
	ADC TEMP_L,TEMP_EH
	LDI TEMP_H,0x00
	MCALL C5_WAIT_2MS
	RET

;--------------------------------------------------------
_DRV_BUZZER_SET:
;--------------------------------------------------------
;Управление пинтанием buzzer
;IN: flag C
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	LDD TEMP, Y+_DRV_BUZZER_PA
	LDD ACCUM,Y+_DRV_BUZZER_BN
	MCALL REG_BIT_SSET
	POP ACCUM
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_BUZZER_ASYNC_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру для асинхронного вызова
;--------------------------------------------------------
	PUSH_T16
	PUSH TEMP

	LDD TEMP,Y+_DRV_BUZZER_STATE
	CPI TEMP,_DRV_BUZZER_ST_BUSY
	BRNE _DRV_BUZZER_ASYNC_TIMER_EVENT__END

	LDD TEMP_H,Y+_DRV_BUZZER_BIT_TIME_CNTR
	DEC TEMP_H
	STD Y+_DRV_BUZZER_BIT_TIME_CNTR,TEMP_H
	BRNE _DRV_BUZZER_ASYNC_TIMER_EVENT__END

	LDD TEMP,Y+_DRV_BUZZER_BIT_TIME
	STD Y+_DRV_BUZZER_BIT_TIME_CNTR,TEMP

	LDD TEMP,Y+_DRV_BUZZER_BIT_CNTR
	CPI TEMP,0x00
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_BUZZER_OP_STOP
	RJMP _DRV_BUZZER_ASYNC_TIMER_EVENT__END
	DEC TEMP
	STD Y+_DRV_BUZZER_BIT_CNTR,TEMP

	LDD TEMP,Y+_DRV_BUZZER_MOD_TIMER_ID
	LDD TEMP_H,Y+_DRV_BUZZER_BIT_DATA+0x00
	LDD TEMP_L,Y+_DRV_BUZZER_BIT_DATA+0x01
	LSL TEMP_L
	ROL TEMP_H
	STD Y+_DRV_BUZZER_BIT_DATA+0x00,TEMP_H
	STD Y+_DRV_BUZZER_BIT_DATA+0x01,TEMP_L
	ROL TEMP_L
	CPI TEMP,0xff
	BRNE _DRV_BUZZER_ASYNC_TIMER_EVENT__MOD
	ROR TEMP_L
	MCALL _DRV_BUZZER_SET
	RJMP _DRV_BUZZER_ASYNC_TIMER_EVENT__END

_DRV_BUZZER_ASYNC_TIMER_EVENT__MOD:
	ROR TEMP_L
	BRCS PC+0x01+_MCALL_SIZE+0x01
	MCALL C5_TIMER_STOP
	RJMP _DRV_BUZZER_ASYNC_TIMER_EVENT__END
	MCALL C5_TIMER_START

_DRV_BUZZER_ASYNC_TIMER_EVENT__END:
	POP TEMP
	POP_T16
	RET

;--------------------------------------------------------
_DRV_BUZZER_MOD_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик перывания по таймеру для модуляции
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
