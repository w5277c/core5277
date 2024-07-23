;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;01.06.2024	konstantin@5277.ru			Начало, не тестировано
;-----------------------------------------------------------------------------------------------------------------------
;Поиск устройств на шине 1wire.
;!!!Процедура запрещает прерывания  на время выполнения!!!

	.include "./mem/ram_stk_alloc.inc"
	.include "./mem/ram_stk_free.inc"

;--------------------------------------------------------
DRV_1WIRE_SEARCH_INIT:
;--------------------------------------------------------
;Инициализация поиска
;IN: X-8B буффер
;--------------------------------------------------------
	PUSH TEMP
	PUSH LOOP_CNTR

	LDI TEMP,0xff
	LDI LOOP_CNTR,0x8
	MCALL RAM_FILL

	POP LOOP_CNTR
	POP TEMP
	RET

;--------------------------------------------------------
DRV_1WIRE_SEARCH:														;Возвращает ROM устройства и порядковый номер бита, указывающий либо на конец поиска(значение 64),
;--------------------------------------------------------;либо на различающийся бит в ROM у нескольких устройств. При LOOP_CNTR!=64 следует запомнить полученный ROM,
;Посик устройства.													;и выполнить процедуру заново. Таким образом получим список всех устройств.
;IN: ACCUM-порт,X-8B буффер, LOOP_CNTR-номер бита			;Для первого запуска LOOP_CNTR должен быть равен 0, а буфер заполнен 0xff(
;OUT: TEMP-DRV_RESULT_OK/ABSENT, LOOP_CNTR-номер бита
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_T16
	PUSH FLAGS
	PUSH ACCUM
	PUSH TEMP_EL

	MOV TEMP,ACCUM
	LDI ACCUM,0x01														;Выделяю в стеке 1 байт для буфера команды
	MCALL RAM_STK_ALLOC
	MOV ACCUM,TEMP

	MCALL C5_DISPATCHER_LOCK
	MCALL _DRV_1WIRE_RESET
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_1WIRE_SEARCH__END

	LDI TEMP,0xf0														;ROM SEARCH COMMAND
	MCALL _DRV_1WIRE_SEND_BYTE

	MOV TEMP_EL,LOOP_CNTR
	LDI LOOP_CNTR,0x00

_DRV_1WIRE_SEARCH__LOOP:
	LDI FLAGS,0x00
	CLI
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	MCALL PORT_MODE_IN
   MCALL _DRV_1WIRE_DELAY_4US
	MCALL PORT_GET														;Читаем состояние порта
	ROL FLAGS
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	MCALL PORT_MODE_IN
   MCALL _DRV_1WIRE_DELAY_4US
	MCALL PORT_GET														;Читаем состояние порта
	SEI
	ROL FLAGS

	PUSH_X
	MOV TEMP_H,LOOP_CNTR
	LSR TEMP_H
	LSR TEMP_H
	LSR TEMP_H
	ADD XL,TEMP_H
	ADC XH,C0x00														;Получаем адрес текущего байта
	MOV TEMP,LOOP_CNTR
	ANDI TEMP,0x07														;Получаем текущий номер бита в байте
	MCALL BITNUM_TO_NUM												;Получаем число
	LD TEMP_L,X															;Чтение байта
	MOV TEMP_H,TEMP_L

	CP LOOP_CNTR,TEMP_EL												;Начальные биты уже опрошены
	BRCC PC+0x04
	POP_X
	RJMP _DRV_1WIRE_SEARCH__LOOP_NEXT

	AND TEMP_H,TEMP													;Проверка на повторный запуск(если бит сброшен)
	BRNE _DRV_1WIRE_SEARCH__BIT_HI
	OR TEMP_L,TEMP														;Выставляем бит в HI и переходим к следующему
	ST X,TEMP_L															;Запись байта
	POP_X
	RJMP _DRV_1WIRE_SEARCH__LOOP_NEXT

_DRV_1WIRE_SEARCH__BIT_HI:
	MOV TEMP_H,TEMP
	COM TEMP_H
	AND TEMP_L,TEMP_H													;Сброс текущего бита
	CPI FLAGS,0x03
	BRNE PC+0x02
	OR TEMP_L,TEMP														;Установка бита
	ST X,TEMP_L															;Запись байта
	POP_X

	CPI FLAGS,0x01														;Проверка на конфликт
	BREQ _DRV_1WIRE_SEARCH__OK
	CPI FLAGS,0x02
	BREQ _DRV_1WIRE_SEARCH__OK

_DRV_1WIRE_SEARCH__LOOP_NEXT:
	C5_WAIT_500NS 45*2												;45us

	INC LOOP_CNTR
	CPI LOOP_CNTR,0x40
	BREQ PC+0x02
	RJMP _DRV_1WIRE_SEARCH__LOOP

_DRV_1WIRE_SEARCH__OK:
	LDI TEMP,DRV_RESULT_OK
_DRV_1WIRE_SEARCH__END:
	MCALL C5_DISPATCHER_UNLOCK

	LDI ACCUM,0x01														;Освобождаю память в стеке
	MCALL RAM_STK_FREE

	POP TEMP_EL
	POP ACCUM
	POP FLAGS
	POP_T16
	POP_Y
	POP_X
	RET
