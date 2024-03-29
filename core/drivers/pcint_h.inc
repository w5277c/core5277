;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.07.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;03.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
;TODO непомерное использование RAM(216 байт), нужна оптимизация
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_PCINT_H
.else
.set DEF__C5_DRV_PCINT_H = 1
.message "Included driver hardware PCINT v0.2"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./mem/ram_fill.inc"
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
	MOVW XL,YL
	MOV LOOP_CNTR,ACCUM
	LDI TEMP,0xff
	MCALL RAM_FILL
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
;IN: ACCUM-порт, X-адрес обработчика,FLAGS-PID
;(0xFF-отключить)
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	LDI_Z PCINT_TABLE*2
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	MOV TEMP_H,TEMP
	ADD ZL,TEMP
	ADC ZH,C0x00
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

	MOVW ZL,YL
	LDI TEMP,low(_DRV_PCINT_H_RAM_PREV_STATES)
	ADD ZL,TEMP
	ADC ZH,C0x00
	;Прибавляем смещение относительно номера порта
	MOV TEMP,ACCUM
	SWAP TEMP
	ANDI TEMP,0x07
	ADD ZL,TEMP
	ADC ZH,C0x00
	;Записываю новое состояние бита
	LD TEMP,Z
	PUSH TEMP_L
	COM TEMP_L
	AND TEMP,TEMP_L
	POP TEMP_L
	SBRC TEMP_H,0x00
	OR TEMP,TEMP_L
	ST Z,TEMP

	MOVW ZL,YL
	;Прибавляем смещение на адреса
	LDI TEMP,low(_DRV_PCINT_H_RAM_ADDRS)
	ADD ZL,TEMP
	ADC ZH,C0x00
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
	ADC ZH,C0x00
	;Записываем адрес перехода
	ST Z+,FLAGS
	ST Z+,XH
	ST Z+,XL

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
	POP_Y
	RET

;--------------------------------------------------------
_DRV_PCINT_H_EVENT:
;--------------------------------------------------------
;Обработчик изменения состояния пина
;IN: ACCUM-номер строки в таблице прерываний
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
	PUSH_Y
	PUSH_Z
	PUSH TEMP
	PUSH FLAGS
	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	MOVW XL,YL
	;Получаем адрес предидущих состояний
	LDI TEMP,low(_DRV_PCINT_H_RAM_PREV_STATES+0x01)
	ADD TEMP,LOOP_CNTR
	ADD XL,TEMP
	ADC XH,C0x00
	LD TEMP,X															;Старое значение
	ST X,ACCUM
	MOV FLAGS,ACCUM
	EOR ACCUM,TEMP														;XOR

	LDI TEMP,low(_DRV_PCINT_H_RAM_ADDRS)
	SWAP LOOP_CNTR
	ADD TEMP,LOOP_CNTR
	LSR LOOP_CNTR
	ADD TEMP,LOOP_CNTR
	ADD YL,TEMP
	ADC YH,C0x00

	LDI LOOP_CNTR,0x08
_DRV_PCINT_H_EVENT__LOOP:
	LD TEMP,Y+
	LD ZH,Y+
	LD ZL,Y+

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
	POP_Y
	POP_X
	POP LOOP_CNTR
	POP ACCUM
	RET
.endif
