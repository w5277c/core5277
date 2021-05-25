;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.07.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_PCINT_H
.else
.set DEF__C5_DRV_PCINT_H = 1
.message "Included driver hardware PCINT v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./mem/ram_fill8.inc"
.include	"./io/port_mode_in.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_set_hi.inc"
.include	"./io/port_get.inc"
.include	"./io/port_get_byte.inc"
.include	"./conv/bitnum_to_num.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_PCINT_H_RAM_PREV_STATES					= 0x00;?B - блок предыдущих состояний каждого пина(1 бит на пин)
	.EQU	_DRV_PCINT_H_RAM_ADDRS							= _DRV_PCINT_H_RAM_PREV_STATES + _C5_PCINT_PORTS_QNT
																			;?B - блок ид драйвера и адресов для каждого порта по 3 байта на пин(0x00 - не задано)
	.EQU	_DRV_PCINT_H_RAM_SIZE							= _C5_PCINT_PORTS_QNT + (_C5_PCINT_PORTS_QNT*8*3)
;--------------------------------------------------------
DRV_PCINT_H_INIT:
;--------------------------------------------------------
;Инициализация
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_PCINT_H_RAM_SIZE
	MCALL C5_RAM_REALLOC
	;Заполняем
	MOV XH,ZH
	MOV XL,ZL
	MOV LOOP_CNTR,ACCUM
	LDI TEMP,0xff
	MCALL RAM_FILL8
	;Активируем прерывания
	LDI TEMP_H,high(_DRV_PCINT_H_EVENT)
	LDI TEMP_L,low(_DRV_PCINT_H_EVENT)
	LDI ACCUM,C5_IR_PCINT0
	MCALL C5_IR_VECTOR_SET
	LDI ACCUM,C5_IR_PCINT1
	MCALL C5_IR_VECTOR_SET
	LDI ACCUM,C5_IR_PCINT2
	MCALL C5_IR_VECTOR_SET
	;Завершаем инициализацию
	MCALL C5_READY
;--------------------------------------------------------
;Основной код, разрешаем прерывания и
;включаем необходимые флаги
;IN: ACCUM - порт
;Y - адрес обработчика
;FLAGS - PID(0xFF - отключить)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	LDI ZH,high(PCINT_TABLE*2)
	LDI ZL,low(PCINT_TABLE*2)
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	MOV TEMP_H,TEMP
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LPM TEMP,Z+
	MOV ZL,TEMP
	;Проверка на не заданный порт регистра
	CPI ZL,0x00
	BRNE PC+0x02
	RJMP _DRV_PCINT_H_PROC__END
	LDI ZH,0x00
	PUSH ACCUM
	MOV TEMP,ACCUM
	ANDI TEMP,0x07
	MCALL BITNUM_TO_NUM
	MOV TEMP_L,TEMP													;Бит пина(число)
	LD ACCUM,Z
	;Проверка на освобождение пина
	CPI FLAGS,0xff
	BRNE _DRV_PCINT_H_PROC__NOT_RESET
	COM TEMP
	AND ACCUM,TEMP
	ST Z,ACCUM
	POP ACCUM
	RJMP _DRV_PCINT_H_PROC__END
_DRV_PCINT_H_PROC__NOT_RESET:
	OR ACCUM,TEMP
	ST Z,ACCUM
	POP ACCUM
	;Получаем состоянеи пина
	MCALL PORT_GET
	ROL TEMP_H

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	PUSH_Z
	LDI TEMP,low(_DRV_PCINT_H_RAM_PREV_STATES)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	;Прибавляем смещение относительно номера порта
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	;Записываю новое состояние бита
	LD TEMP,Z
	PUSH TEMP_L
	COM TEMP_L
	AND TEMP,TEMP_L
	POP TEMP_L
	SBRC TEMP_H,0x00
	OR TEMP,TEMP_L
	ST Z,TEMP
	;Получаем адрес блока выделенной памяти
	POP_Z

	;Прибавляем смещение на адреса
	LDI TEMP,low(_DRV_PCINT_H_RAM_ADDRS)
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	;прибавляем смещение относительно номера порта и бита
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	DEC TEMP
	LSL TEMP
	LSL TEMP
	LSL TEMP
	PUSH ACCUM
	ANDI ACCUM,0x07
	ADD TEMP,ACCUM
	;Умножаю на 3
	MOV ACCUM,TEMP
	LSL TEMP
	ADD TEMP,ACCUM
	POP ACCUM
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	;Записываем адрес перехода
	ST Z+,FLAGS
	ST Z+,YH
	ST Z+,YL

	;Разрешаем прерывание PCIE(2-0)
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	DEC TEMP
	MCALL BITNUM_TO_NUM
	LDS ACCUM,PCICR
	OR ACCUM,TEMP
	STS PCICR,ACCUM

_DRV_PCINT_H_PROC__END:
	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_DRV_PCINT_H_EVENT:
;--------------------------------------------------------
;Обработчик изменения состояния пина
;IN: ACCUM - номер строки в таблице прерываний
;При вызове ICALL первый бит FLAGS содержит состояние
;пина
;--------------------------------------------------------
	PUSH ACCUM
	PUSH LOOP_CNTR

	;Проверяем на ACCUM и обрабатываем только чаcть
	SUBI ACCUM,C5_IR_PCINT0
	MOV LOOP_CNTR,ACCUM
	;Получаем значение порта
	INC ACCUM
	SWAP ACCUM
	MCALL PORT_GET_BYTE

	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH FLAGS
	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET
	MOV XH,ZH
	MOV XL,ZL
	;Получаем адрес предидущих состояний
	LDI TEMP,low(_DRV_PCINT_H_RAM_PREV_STATES+0x01)
	ADD TEMP,LOOP_CNTR
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP
	LD TEMP,Z															;Старое значение
	ST Z,ACCUM
	MOV FLAGS,ACCUM
	EOR ACCUM,TEMP														;XOR

	LDI TEMP,low(_DRV_PCINT_H_RAM_ADDRS)
	SWAP LOOP_CNTR
	ADD TEMP,LOOP_CNTR
	LSR LOOP_CNTR
	ADD TEMP,LOOP_CNTR
	ADD XL,TEMP
	CLR TEMP
	ADC XH,TEMP

	LDI LOOP_CNTR,0x08
_DRV_PCINT_H_EVENT__LOOP:
	LD TEMP,X+
	LD ZH,X+
	LD ZL,X+

	LSR ACCUM
	BRCC _DRV_PCINT_H_EVENT__NEXT
	CPI TEMP,0xff
	BREQ _DRV_PCINT_H_EVENT__NEXT
	PUSH PID
	MOV PID,TEMP
	ICALL
	POP PID

_DRV_PCINT_H_EVENT__NEXT:
	LSR FLAGS
	DEC LOOP_CNTR
	BRNE _DRV_PCINT_H_EVENT__LOOP

	POP FLAGS
	POP TEMP
	POP_Z
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	RET
.endif
