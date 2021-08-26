;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;24.03.2020	w5277c@gmail.com			Начало
;30.08.2020	w5277c@gmail.com			Адаптировано под изменения ядра
;13.09.2020	w5277c@gmail.com			Добавлена дополнительная проверка на присутствие датчика
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;25.04.2021	w5277c@gmail.com			Подправлены тайминги, учтен CORE_FREQ
;03.07.2021	w5277c@gmail.com			Z->Y
;26.08.2021	w5277c@gmail.com			Блокировка перываний в процедуре передачи байта
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_1WIRE
.else
.set DEF__C5_DRV_1WIRE = 1
.message "Included driver 1Wire v0.3"

.include	"./io/port_mode_in.inc"
.include	"./io/port_mode_out.inc"
.include	"./io/port_set_lo.inc"
.include	"./io/port_get.inc"
.include	"./core/res_lock.inc"
.include	"./core/res_unlock.inc"

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
	STD Y+_DRV_1WIRE_PORT,ACCUM

	;Инициализация порта (IN, WO_PULL)
	MCALL PORT_MODE_IN
	MCALL PORT_SET_LO

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-адрес на блок данных на передачу
;(15-ый бит false-RAM, true-ROM)
;TEMP_H-длина данных на запись,
;TEMP_L-длина данных для чтения.
;X-адрес на блок для приема
;OUT: TEMP-0x00-успех
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH ACCUM

	;Сичтываем информацию о пине
	MCALL C5_RAM_OFFSET_Y
	LDD ACCUM,Y+_DRV_1WIRE_PORT

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
	LDI TEMP,low((250-10)/(16/CORE_FREQ))						;500us(480-960) по факту получл 489мс
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL PORT_MODE_IN
	C5_WAIT_500NS 60*2												;60us
	MCALL PORT_GET														;Читаем состояние порта
	BRCS _DRV_1WIRE_PROC__ERROR
	LDI TEMP,low((220-10)/(16/CORE_FREQ))						;440us(420 min)
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL _C5_WAIT_500NS
	MCALL PORT_GET														;Читаем состояние порта
	BRCC _DRV_1WIRE_PROC__ERROR
	CPI TEMP_H,0x00
	BREQ _DRV_1WIRE_PROC__SEND_SKIP
_DRV_1WIRE_PROC__SEND_LOOP:
	SBRC ZH,0x07
	RJMP PC+0x03
	LD TEMP,Z+
	RJMP PC+04
	ANDI ZH,0b01111111
	LPM TEMP,Z+
	ORI ZH,0b10000000
	MCALL _DRV_1WIRE_SEND_BYTE
	DEC TEMP_H
	BRNE _DRV_1WIRE_PROC__SEND_LOOP
_DRV_1WIRE_PROC__SEND_SKIP:

	CPI TEMP_L,0x00
	BREQ _DRV_1WIRE_PROC__RECV_SKIP
_DRV_1WIRE_PROC__RECV_LOOP:
	MCALL _DRV_1WIRE_RECV_BYTE
	ST X+,TEMP
	DEC TEMP_L
	BRNE _DRV_1WIRE_PROC__RECV_LOOP
_DRV_1WIRE_PROC__RECV_SKIP:

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
	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_1WIRE_RECV_BYTE:
;--------------------------------------------------------
;Чтене байта
;IN: ACCUM-порт
;OUT: TEMP-байт
;--------------------------------------------------------
	PUSH TEMP_L
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,0x08
_DRV_1WIRE_RECV_BYTE__LOOP:
   CLI																	;(old 14us)
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	MCALL PORT_MODE_IN
   MCALL _DRV_1WIRE_DELAY_4US
	MCALL PORT_GET														;Читаем состояние порта
	SEI
	ROR TEMP_L
	C5_WAIT_500NS 45*2												;45us
	DEC LOOP_CNTR
	BRNE _DRV_1WIRE_RECV_BYTE__LOOP
	MOV TEMP,TEMP_L
	POP LOOP_CNTR
	POP TEMP_L
	RET

;--------------------------------------------------------
_DRV_1WIRE_SEND_BYTE:
;--------------------------------------------------------
;Запись байта
;IN: TEMP-байт, ACCUM-порт
;--------------------------------------------------------
	PUSH TEMP
	PUSH TEMP_L
	PUSH LOOP_CNTR

	MOV TEMP_L,TEMP
	LDI LOOP_CNTR,0x08
_DRV_1WIRE_SEND_BYTE__LOOP:
	CLI
	MCALL PORT_MODE_OUT
	MCALL _DRV_1WIRE_DELAY_1US
	LSR TEMP_L
	BRCC PC+0x03
	MCALL PORT_MODE_IN
	SEI
	C5_WAIT_500NS 60*2												;60us
	MCALL PORT_MODE_IN
	DEC LOOP_CNTR
	BRNE _DRV_1WIRE_SEND_BYTE__LOOP

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP
	RET

;--------------------------------------------------------
_DRV_1WIRE_DELAY_1US:
;--------------------------------------------------------
;Пауза в 16 тактов
;--------------------------------------------------------
;TODO необходимо проверить тайминги на частотах !=16
.IF CORE_FREQ > 16
	NOP
.ENDIF
.if CORE_FREQ > 8
	NOP
	NOP
	NOP
.ENDIF
.if CORE_FREQ > 4
	NOP
	NOP
.ENDIF
	RET

;--------------------------------------------------------
_DRV_1WIRE_DELAY_4US:
;--------------------------------------------------------
;Пауза в 64 такта
;--------------------------------------------------------
	PUSH TEMP
	LDI TEMP,CORE_FREQ-1
	DEC TEMP
	BRNE PC-0x01
	POP TEMP
	RET
.endif
