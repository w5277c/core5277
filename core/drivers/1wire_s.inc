;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.03.2020  w5277c@gmail.com        Начало
;30.08.2020  w5277c@gmail.com        Адаптировано под изменения ядра
;13.09.2020  w5277c@gmail.com        Добавлена дополнительная проверка на присутствие датчика
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/io/port_mode_in.inc"
.include	"./inc/io/port_mode_out.inc"
.include	"./inc/io/port_set_lo.inc"
.include	"./inc/io/port_get.inc"
.include	"./inc/core/res_lock.inc"
.include	"./inc/core/res_unlock.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_1WIRE_PORT						= 0x00			;1b - Порт шины

	.EQU	_DRV_1WIRE_RAM_SIZE					= 0x01			;1 байт необходимо выделить
;--------------------------------------------------------
DRV_1WIRE_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - порт шины
;--------------------------------------------------------
	PUSH ACCUM
	LDI ACCUM,_DRV_1WIRE_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM
	STD Z+_DRV_1WIRE_PORT,ACCUM

	;Инициализация порта (IN, WO_PULL)
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Y - адрес на блок данных на запись
;(15-ый бит false - RAM, true - ROM)
;TEMP_H - длина данных на запись,
;TEMP_L - длина данных для чтения.
;X - адрес на блок для чтения
;OUT: TEMP - 0x00-успех
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	;Сичтываем информацию о пине
	MCALL C5_RAM_OFFSET
	LDD ACCUM,Z+_DRV_1WIRE_PORT

	MOV ZH,YH
	MOV ZL,YL
	;Умножаем на 2 адрес рабоыт с ROM
	SBRS ZH,0x07
	RJMP PC+0x04
	LSL ZL
	ROL ZH
	ORI ZH,0x80

	;Занимаем ресурс(TIMER_B)
;	PUSH ACCUM
;_DRV_1WIRE_PROC__WAIT_RES:
;	LDI ACCUM,C5_RES_TIMER_B
;	MCALL C5_RES_LOCK
;	BRNE _DRV_1WIRE_PROC__WAIT_RES
;	POP ACCUM

	MCALL C5_DISPATCHER_LOCK
	;Проверка на присутствие
	MCALL PORT_SET_LO
	MCALL PORT_MODE_OUT
	LDI TEMP,(540*2)/6												;500us(480-960)
	MCALL C5_WAIT_500NS												;TODO по факту получл 489мс
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL PORT_MODE_IN
	LDI TEMP,60*2														;60us
	MCALL C5_WAIT_500NS
	MCALL PORT_GET														;Читаем состояние порта
	BRCS _DRV_1WIRE_PROC__ERROR
	LDI TEMP,(440*2)/4												;440us(420 min)
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL C5_WAIT_500NS
	MCALL PORT_GET														;Читаем состояние порта
	BRCC _DRV_1WIRE_PROC__ERROR
	CPI TEMP_H,0x00
	BREQ _DRV_1WIRE_PROC__WRITE_SKIP
_DRV_1WIRE_PROC__WRITE_LOOP:
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	MCALL _DRV_1WIRE_WRITE_BYTE
	DEC TEMP_H
	BRNE _DRV_1WIRE_PROC__WRITE_LOOP
_DRV_1WIRE_PROC__WRITE_SKIP:

	CPI TEMP_L,0x00
	BREQ _DRV_1WIRE_PROC__READ_SKIP
_DRV_1WIRE_PROC__READ_LOOP:
	MCALL _DRV_1WIRE_READ_BYTE
	ST X+,TEMP
	DEC TEMP_L
	BRNE _DRV_1WIRE_PROC__READ_LOOP
_DRV_1WIRE_PROC__READ_SKIP:

	LDI TEMP,0x00
	RJMP _DRV_1WIRE_PROC__END
_DRV_1WIRE_PROC__ERROR:
	LDI TEMP,0x01
_DRV_1WIRE_PROC__END:
	MCALL C5_DISPATCHER_UNLOCK
	;Освобождаем ресурс
;	LDI ACCUM,C5_RES_TIMER_B
;	MCALL C5_RES_UNLOCK

	POP ACCUM
	POP TEMP_L
	POP TEMP_H
	POP_Z
	POP_X
	RET

;--------------------------------------------------------
_DRV_1WIRE_READ_BYTE:
;--------------------------------------------------------
;Чтене байта
;IN: ACCUM - порт
;OUT: TEMP - байт
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,0x08
_DRV_1WIRE_READ_BYTE__LOOP:
   CLI																	;(old 14us)
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	MCALL PORT_MODE_IN
   MCALL _DRV_1WIRE_DELAY_4US
	MCALL PORT_GET														;Читаем состояние порта
	SEI
	ROR TEMP_L
   LDI TEMP,45*2														;45us
   MCALL C5_WAIT_500NS
	DEC LOOP_CNTR
	BRNE _DRV_1WIRE_READ_BYTE__LOOP
	MOV TEMP,TEMP_L
	POP LOOP_CNTR
	POP TEMP_L
	RET

;--------------------------------------------------------
_DRV_1WIRE_WRITE_BYTE:
;--------------------------------------------------------
;Запись байта
;IN: TEMP - байт, ACCUM - порт
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH LOOP_CNTR

	MOV TEMP_L,TEMP
	LDI LOOP_CNTR,0x08
_DRV_1WIRE_WRITE_BYTE__LOOP:
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	LSR TEMP_L
	BRCC _DRV_1WIRE_WRITE_BYTE__LO
	MCALL PORT_MODE_IN
_DRV_1WIRE_WRITE_BYTE__LO:
   LDI TEMP,60*2														;60us
   MCALL C5_WAIT_500NS
	MCALL PORT_MODE_IN
	DEC LOOP_CNTR
	BRNE _DRV_1WIRE_WRITE_BYTE__LOOP

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_1WIRE_DELAY_1US:
;--------------------------------------------------------
;Пауза в 16 тактов
;--------------------------------------------------------
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	RET

;--------------------------------------------------------
_DRV_1WIRE_DELAY_4US:
;--------------------------------------------------------
;Пауза в 64 такта
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,14
	DEC TEMP
	BRNE PC-0x01
	POP TEMP
   RET
