;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.05.2021	w5277c@gmail.com			Начало, не тестировано
;31.05.2021	w5277c@gmail.com			Исправлены основные ошибки
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер логирования (UART 2400 8n2, работает только с портом B)
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF__C5_DRV_LOGGER
.else
.set DEF__C5_DRV_LOGGER = 1
.message "Included driver logger v0.1"

.include	"./core/ram/ram_offset.inc"
.include	"./core/ram/ram_realloc.inc"
.include	"./core/wait.inc"
.include	"./core/timer_set.inc"
.include	"./core/timer_start.inc"
.include	"./core/timer_stop.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_LOGGER_PIN_NUM							= 0x00	;1B - число бита для пина передачи данных
	.EQU	_DRV_LOGGER_TIMER_ID							= 0x01	;1B - ид таймера
	.EQU	_DRV_LOGGER_R_POS								= 0x02	;1B - позиция в буфере для чтения
	.EQU	_DRV_LOGGER_W_POS								= 0x03	;1B - позиция в буфере для записи
	.EQU	_DRV_LOGGER_SIZE								= 0x04	;1B - размер буфера
	.EQU	_DRV_LOGGER_BIT_NUM							= 0x05	;1B - номер текущего бита
	.EQU	_DRV_LOGGER_ACCUM								= 0x06	;1B - аккумулятор (байт подготовленный для передачи/приема)
	.EQU	_DRV_LOGGER_BUFFER							= 0x07	;Круговой буфер
	;---
	.EQU	_DRV_LOGGER_HEADER_SIZE						= 0x07	;7 байт заголовок
;--------------------------------------------------------
DRV_LOGGER_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_L-порт TX, TEMP_H-размер буфера(max:0x7f),
;ACCUM-ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_LOGGER_HEADER_SIZE
	ADD ACCUM,TEMP_H
	MCALL C5_RAM_REALLOC

	;Инициализируем порт направления данных
	ANDI TEMP_L,0x0f
	MOV TEMP,TEMP_L
	MCALL BITNUM_TO_NUM
	STD Z+_DRV_LOGGER_PIN_NUM,TEMP
	CLI
	IN TEMP_L,DDRB
	OR TEMP_L,TEMP
	OUT DDRB,TEMP_L
	IN TEMP_L,PORTB
	OR TEMP_L,TEMP
	OUT DDRB,TEMP_L
	SEI

	STD Z+_DRV_LOGGER_R_POS,C0x00
	STD Z+_DRV_LOGGER_W_POS,C0x00
	STD Z+_DRV_LOGGER_BIT_NUM,C0x00
	STD Z+_DRV_LOGGER_SIZE,TEMP_H

	POP TEMP
	;Инициализируем таймер
	STD Z+_DRV_LOGGER_TIMER_ID,TEMP
	LDI TEMP_H,high(DRV_LOGGER_TIMER_EVENT)
	LDI TEMP_L,low(DRV_LOGGER_TIMER_EVENT)
	LDI ACCUM,0x08
	MCALL C5_TIMER_SET

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, помещаем данные в буфер(ждем если буфер
;заполнен)
;IN: Y-src адрес, LOOP_CNTR-длина данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH LOOP_CNTR

	MCALL C5_RAM_OFFSET

	;Адрес Y в FLASH?
	SBRS YH,0x07
	RJMP PC+0x04
	LSL YL
	ROL YH
	ORI YH,0x80

	PUSH_Y
	MOVW YL,ZL
	POP_Z
	MOV ACCUM,ZH
	ANDI ZH,0x7f

	LDD TEMP_EL,Y+_DRV_LOGGER_SIZE
	LDD TEMP_L,Y+_DRV_LOGGER_W_POS

DRV_LOGGER__LOOP:
	;Проверяем на свободное место в буфере, иначе ждем освобождения 1 байта
	LDD TEMP_H,Y+_DRV_LOGGER_R_POS
	MOV TEMP_EH,TEMP_EL
	DEC TEMP_EH
	CPI TEMP_H,0x00
	BRNE PC+0x03
	CP TEMP_L,TEMP_EH
	BREQ DRV_LOGGER__BUFFER_FULL
	MOV TEMP_EH,TEMP_H
	SUB TEMP_EH,TEMP_L
	BRCS DRV_LOGGER__BUFFER_NOT_FULL
	CPI TEMP_EH,0x01
	BRNE DRV_LOGGER__BUFFER_NOT_FULL
DRV_LOGGER__BUFFER_FULL:
	MCALL C5_WAIT
	RJMP DRV_LOGGER__LOOP
DRV_LOGGER__BUFFER_NOT_FULL:
	;Считываем байт с источника
	SBRS ACCUM,0x07
	LD TEMP,Z+
	SBRC ACCUM,0x07
	LPM TEMP,Z+
	;Записываем байт в буфер
	PUSH_Y
	ADIW YL,_DRV_LOGGER_HEADER_SIZE
	ADD YL,TEMP_L
	ADC YH,C0x00
	ST Y,TEMP
	POP_Y
	;Инкрементируем адрес записи
	INC TEMP_L
	CP TEMP_L,TEMP_EL
	BRNE PC+0x02
	CLR TEMP_L
	STD Y+_DRV_LOGGER_W_POS,TEMP_L

	;Запускаем таймер, если буфер пуст(т.е. логирвание остановлено)
	CP TEMP_EH,C0x00
	BRNE PC+0x06
	LDD TEMP,Y+_DRV_LOGGER_TIMER_ID
	MCALL C5_TIMER_START
	MCALL C5_SOFT_UART_MODE_SET

	DEC LOOP_CNTR
	BRNE DRV_LOGGER__LOOP

	POP LOOP_CNTR
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_Y
	RET


;--------------------------------------------------------
DRV_LOGGER_TIMER_EVENT:
;--------------------------------------------------------
;Событие от таймера
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	CLI

	MCALL C5_RAM_OFFSET

	LDD TEMP_EH,Z+_DRV_LOGGER_BIT_NUM
	CPI TEMP_EH,0x00
	BRNE __DRV_LOGGER_TIMER_EVENT__NO_START
		;START (TX=LO)
		LDD ACCUM,Z+_DRV_LOGGER_PIN_NUM
		COM ACCUM
		IN TEMP,PORTB
		AND TEMP,ACCUM
		OUT PORTB,TEMP
		;Считываем в ACCUM байт из буфера
		PUSH_Z
		LDD TEMP,Z+_DRV_LOGGER_R_POS
		ADIW ZL,_DRV_LOGGER_HEADER_SIZE
		ADD ZL,TEMP
		ADC ZH,C0x00
		LD ACCUM,Z
		POP_Z
		STD Z+_DRV_LOGGER_ACCUM,ACCUM
		RJMP __DRV_LOGGER_TIMER_EVENT__NEXT
__DRV_LOGGER_TIMER_EVENT__NO_START:
	CPI TEMP_EH,0x0a
	BREQ __DRV_LOGGER_TIMER_EVENT__NEXT
	CPI TEMP_EH,0x0b
	BRNE __DRV_LOGGER_TIMER_EVENT__NOT_FINISH
		LDD TEMP,Z+_DRV_LOGGER_TIMER_ID
		LDD TEMP_H,Z+_DRV_LOGGER_W_POS
		LDD TEMP_L,Z+_DRV_LOGGER_R_POS
		LDD TEMP_EL,Z+_DRV_LOGGER_SIZE
		INC TEMP_L
		CP TEMP_L,TEMP_EL
		BRNE PC+0x02
		CLR TEMP_L
		CP TEMP_L,TEMP_H
		BRNE PC+0x05
		MCALL C5_TIMER_STOP
		MCALL C5_SOFT_UART_MODE_RESET
		STD Z+_DRV_LOGGER_R_POS,TEMP_L
		STD Z+_DRV_LOGGER_BIT_NUM,C0x00
		RJMP __DRV_LOGGER_TIMER_EVENT__END
__DRV_LOGGER_TIMER_EVENT__NOT_FINISH:
	CPI TEMP_EH,0x09
	BREQ __DRV_LOGGER_TIMER_EVENT__STOP
__DRV_LOGGER_TIMER_EVENT__DATA:
		LDD ACCUM,Z+_DRV_LOGGER_ACCUM
		LDD TEMP_H,Z+_DRV_LOGGER_PIN_NUM

		IN TEMP,PORTB
		OR TEMP,TEMP_H
		LSR ACCUM
		BRCS PC+0x03
		COM TEMP_H
		AND TEMP,TEMP_H
		OUT PORTB,TEMP

		STD Z+_DRV_LOGGER_ACCUM,ACCUM
		RJMP __DRV_LOGGER_TIMER_EVENT__NEXT
__DRV_LOGGER_TIMER_EVENT__STOP:
		;STOP (TX=HI)
		LDD ACCUM,Z+_DRV_LOGGER_PIN_NUM
		IN TEMP,PORTB
		OR TEMP,ACCUM
		OUT PORTB,TEMP

		RJMP __DRV_LOGGER_TIMER_EVENT__NEXT
__DRV_LOGGER_TIMER_EVENT__NEXT:
		INC TEMP_EH
		STD Z+_DRV_LOGGER_BIT_NUM,TEMP_EH
__DRV_LOGGER_TIMER_EVENT__END:

	SEI
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	RET
.endif
