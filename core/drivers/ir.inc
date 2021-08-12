;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;10.06.2021	w5277c@gmail.com			Начало
;19.06.2021	w5277c@gmail.com			Частичный баг фиксинг
;20.06.2021	w5277c@gmail.com			Успешный тест RX, доработка TX
;26.06.2021	w5277c@gmail.com			Все заново
;01.07.2021	w5277c@gmail.com			Первые успешные тесты
;04.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
;Драйвер инфракрасного приемника и передачтика, использует INT и TIMER C (стандарт NEC возможно адаптирую для SIRC)
;-----------------------------------------------------------------------------------------------------------------------
;TODO: Добавить поддержку PCINT драйвера
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__C5_DRV_IR
.else
.set DEF__C5_DRV_IR = 1
.message "Included driver IR v0.2"

	.include	"./core/ram/ram_offset.inc"
	.include	"./core/ram/ram_realloc.inc"
	.include	"./core/eint_enable.inc"
	.include	"./core/eint_disable.inc"
	.include	"./io/port_offsets.inc"
	.include	"./io/reg_bit_hi.inc"
	.include	"./io/reg_bit_lo.inc"
	.include	"./io/reg_bit_inv.inc"

;---CONSTANTS--------------------------------------------
	;---VARS---
	.EQU	_DRV_IR_RX_PA									= 0x00	;1B - Адрес порта для приема
	.EQU	_DRV_IR_RX_PN									= 0x01	;1B - Число для пина порта приема
	.EQU	_DRV_IR_TX_PA									= 0x02	;1B - Адрес порта для передачи
	.EQU	_DRV_IR_TX_PN									= 0x03	;1B - Число для пина порта передачи
	.EQU	_DRV_IR_LED_PA									= 0x04	;1B - Адрес порта для индикации
	.EQU	_DRV_IR_LED_PN									= 0x05	;1B - Число для пина порта индикации
	.EQU	_DRV_IR_INT_ID									= 0x06	;1B - ИД прерывания C5_IR_INTn
	.EQU	_DRV_IR_FREQ									= 0x07	;1B - Частота передатчика
	.EQU	_DRV_IR_STATE									= 0x08	;1B - Состояние драйвера
	.EQU	_DRV_IR_ACCUM									= 0x09	;1B - Аккумулятор
	.EQU	_DRV_IR_PULSE_CNTR							= 0x0b	;2B - Счетчик пульсаций
	.EQU	_DRV_IR_TIMEOUT								= 0x0d	;2B - Таймаут ожидания данных
	.EQU	_DRV_IR_SRC_ADDR								= 0x0f	;2B - Адрес блока данных для передачи
	.EQU	_DRV_IR_DST_ADDR								= 0x11	;2B - Адрес блока данных для приема
	.EQU	_DRV_IR_BIT_POS								= 0x13	;1B - Позиция текущего бита
	.EQU	_DRV_IR_BYTE_POS								= 0x14	;1B - Позиция текущего байта
	.EQU	_DRV_IR_TX_LEN									= 0x15	;1B - Количество бит для передачи
	.EQU	_DRV_IR_RX_LEN									= 0x16	;1B - Максимальное количество бит для приема
	;---
	.EQU	_DRV_IR_RAM_SIZE								= 0x17	;Необходимо выделить
	;---

	.EQU	DRV_IR_FREQ_40K								= (CORE_FREQ*313)/(100*2)-1
	.EQU	DRV_IR_FREQ_38K								= (CORE_FREQ*325)/(100*2)-1
	.EQU	DRV_IR_FREQ_36K								= (CORE_FREQ*350)/(100*2)-1


	;---TIMINGS---
	.EQU	_DRV_IR_PLEN									= 0x002c-2						;44*0.000013=0.000572, минимальная длина пульсации
	.EQU	_DRV_IR_PREN									= _DRV_IR_PLEN*0x10		;0.009152, первая часть преамбулы
	.EQU	_DRV_IR_PRDI									= _DRV_IR_PLEN*0x08		;0.004576, вторая часть преамбулы
	.EQU	_DRV_IR_DTEN									= _DRV_IR_PLEN*0x01		;0.000572, данные, первая часть
	.EQU	_DRV_IR_DTDI0									= _DRV_IR_PLEN*0x01		;0.000572, данные, вторая часть, 0
	.EQU	_DRV_IR_DTDI1									= _DRV_IR_PLEN*0x03		;0.001716, данные, вторая часть, 0
	.EQU	_DRV_IR_PRLEFT									= _DRV_IR_PLEN*(0x04+0x04)	;0.004576, остаток от таймаута преамбулы, не может быть больше
	.EQU	_DRV_IR_PRMAX									= _DRV_IR_PLEN*(0x18+0x04)	;0.016016, максимальная длина преамбулы
	.EQU	_DRV_IR_DTLEFT									= _DRV_IR_PLEN*(0x04)		;0.001716, остаток от бита, не может быть больше
	.EQU	_DRV_IR_DTMAX									= _DRV_IR_PLEN*(0x04+0x01)	;0.002860, максимальная длина бита
	.EQU	_DRV_IR_DTTRHLD								= _DRV_IR_PLEN*(0x02)		;
	;---TX-STATES-IDS---
	.EQU	_DRV_IR_ST_PREN								= 0x10;
	.EQU	_DRV_IR_ST_PRDI								= 0x01;
	.EQU	_DRV_IR_ST_DTEN								= 0x12;
	.EQU	_DRV_IR_ST_DTDI0								= 0x03;
	.EQU	_DRV_IR_ST_DTDI1								= 0x04;
	.EQU	_DRV_IR_ST_TXDONE								= 0x05;
	;---RX-STATES-IDS---
	.EQU	_DRV_IR_ST_DTW									= 0x06;
	.EQU	_DRV_IR_ST_PR									= 0x07;
	.EQU	_DRV_IR_ST_DT									= 0x08;
	;---RESULT-STATES-IDS---
	.EQU	DRV_IR_ST_READY								= 0x80;
	.EQU	DRV_IR_ST_FAIL									= 0x81;
	.EQU	DRV_IR_ST_OVERFLOW							= 0x82;

;--------------------------------------------------------
DRV_IR_INIT:
;--------------------------------------------------------
;Инициализация
;IN: TEMP_H-порт RX, TEMP_L-порт TX,
;TEMP_EH-ид прерывания C5_IR_INTn
;TEMP_EL-порт индикации передачи (0xff-не используется)
;FLAGS-частота DRV_IR_FREQ_x
;--------------------------------------------------------
	LDI ACCUM,_DRV_IR_RAM_SIZE
	MCALL C5_RAM_REALLOC

	;Настройка таймера C
	LDI_X _DRV_IR_TIMER_C_EVENT
	MCALL C5_TIMER_C_SET_VECTOR

	;C5_IR_INTn
	STD Y+_DRV_IR_INT_ID,TEMP_EH
	;FREQ
	STD Y+_DRV_IR_FREQ,FLAGS
	;RX
	MOV ACCUM,TEMP_H
	STD Y+_DRV_IR_RX_PA,C0xff
	CP ACCUM,C0xff
	BREQ PC+0x0d
	PUSH TEMP_L
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_LO													;MODE IN
	MOV TEMP,TEMP_H
	MCALL REG_BIT_HI
	STD Y+_DRV_IR_RX_PA,TEMP_H
	STD Y+_DRV_IR_RX_PN,ACCUM
	POP TEMP_L
	;TX
	MOV ACCUM,TEMP_L
	STD Y+_DRV_IR_TX_PA,C0xff
	CP ACCUM,C0xff
	BREQ PC+0x0b
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI													;MODE OUT
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_IR_TX_PA,TEMP_H
	STD Y+_DRV_IR_TX_PN,ACCUM
	;LED
	MOV ACCUM,TEMP_EL
	STD Y+_DRV_IR_LED_PA,C0xff
	CP ACCUM,C0xff
	BREQ PC+0x0b
	MCALL PORT_OFFSETS
	MOV TEMP,TEMP_L
	MCALL REG_BIT_HI													;MODE OUT
	MOV TEMP,TEMP_H
	MCALL REG_BIT_LO
	STD Y+_DRV_IR_LED_PA,TEMP_H
	STD Y+_DRV_IR_LED_PN,ACCUM

	;Устанавливаю состояние READY
	LDI ACCUM,DRV_IR_ST_READY
	STD Y+_DRV_IR_STATE,ACCUM

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: Z-src адрес, X-dst адрес,
;TEMP_EH-TX длина, TEMP_EL-RX максимальная длина
;TEMP_H,TEMP_L-таймаут ожидания данных(2ms)
;OUT: TEMP_H-DRV_IR_RESULT_x, TEMP_L-длина полученных
;данных
;--------------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH ACCUM
	PUSH TEMP

	MCALL C5_RAM_OFFSET_Y

	;Запоминаем SRC
	SBRS ZH,0x07
	RJMP PC+0x04
	;Умножаем на 2 адрес работы с ROM
	LSL ZL
	ROL ZH
	ORI ZH,0x80
	STD Y+_DRV_IR_SRC_ADDR+0x00,ZH
	STD Y+_DRV_IR_SRC_ADDR+0x01,ZL
	;Запоминаем DST
	STD Y+_DRV_IR_DST_ADDR+0x00,XH
	STD Y+_DRV_IR_DST_ADDR+0x01,XL
	;Запоминаем таймаут ожидания первого байта
	STD Y+_DRV_IR_TIMEOUT+0x00,TEMP_H
	STD Y+_DRV_IR_TIMEOUT+0x01,TEMP_L
	;Запминаем длину TX и RX
	STD Y+_DRV_IR_TX_LEN,TEMP_EH
	STD Y+_DRV_IR_RX_LEN,TEMP_EL

	;Передача?
	CPI TEMP_EH,0x00
	BREQ _DRV_IR_PROC__TX_SKIP
	;Сброс позиции байта и бита
	STD Y+_DRV_IR_BYTE_POS,C0x00
	STD Y+_DRV_IR_BIT_POS,C0x00
	;Устанавливаю состояние передачи
	LDI TEMP,_DRV_IR_ST_PREN
	STD Y+_DRV_IR_STATE,TEMP
	;Включаю индикатор передачи
	LDD TEMP,Y+_DRV_IR_LED_PA
	LDD ACCUM,Y+_DRV_IR_LED_PN
	CP TEMP,C0xff
	BREQ PC+0x03
	MCALL REG_BIT_HI
	MCALL C5_DISPATCHER_LOCK
	;Устанавливаем счетчик пульсаций и запускаем таймер
	LDI_X _DRV_IR_PREN
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	LDD TEMP,Y+_DRV_IR_FREQ
	MCALL C5_TIMER_C_START
	;Жду окончания передачи
_DRV_IR_PROC__SENDING_WAIT:
	LDD TEMP,Y+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_ST_TXDONE
	BRNE _DRV_IR_PROC__SENDING_WAIT
	MCALL __DRV_IR_EINT_LED_OFF
	MCALL C5_DISPATCHER_UNLOCK

_DRV_IR_PROC__TX_SKIP:
	;Прием?
	CPI TEMP_EL,0x00
	BREQ _DRV_IR_PROC__RX_SKIP

	;Сброс позиции байта и бита
	STD Y+_DRV_IR_BYTE_POS,C0x00
	STD Y+_DRV_IR_BIT_POS,C0x00
	;Устанавливаю состояние приема
	LDI TEMP,_DRV_IR_ST_DTW
	STD Y+_DRV_IR_STATE,TEMP
	;Включаю прерывание для RX порта
	LDD TEMP_H,Y+_DRV_IR_INT_ID
	LDI TEMP_L,C5_ISC_FALLING_EDGE
	PUSH_Y
	LDI YH,high(_DRV_IR_EINT_EVENT)
	LDI YL,low(_DRV_IR_EINT_EVENT)
	MCALL C5_EINT_ENABLE
	POP_Y
	;Запоминаем текущий UPTIME
	CLI
	LDS ZH,_C5_UPTIME+0x03
	LDS ZL,_C5_UPTIME+0x04
	SEI
_DRV_IR_PROC__RECEIVING_WAIT:
	LDD TEMP,Y+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_ST_DTW
	BRNE _DRV_IR_PROC__NO_RX_WAIT
		;Проверка FIRST TIMEOUT
		LDD TEMP,Y+_DRV_IR_TIMEOUT+0x00
		LDD ACCUM,Y+_DRV_IR_TIMEOUT+0x01
		CPI TEMP,0x00
		BRNE PC+0x03
		CPI ACCUM,0x00
		BREQ _DRV_IR_PROC__RECEIVING_WAIT
		CLI
		LDS TEMP_H,_C5_UPTIME+0x03
		LDS TEMP_L,_C5_UPTIME+0x04
		SEI
		SUB TEMP_L,ZL
		SBC TEMP_H,ZH
		CP TEMP_H,TEMP
		BRCS _DRV_IR_PROC__RECEIVING_WAIT
		BRNE PC+0x03
		CP TEMP_L,ACCUM
		BRCS _DRV_IR_PROC__RECEIVING_WAIT
		;Достигли таймаута
		MCALL C5_TIMER_C_STOP
		LDD TEMP_H,Y+_DRV_IR_INT_ID
		MCALL C5_EINT_DISABLE
		LDI TEMP,DRV_IR_ST_FAIL
		STD Y+_DRV_IR_STATE,TEMP
		RJMP _DRV_IR_PROC__RX_DONE
_DRV_IR_PROC__NO_RX_WAIT:
		SBRS TEMP,0x07
		RJMP _DRV_IR_PROC__RECEIVING_WAIT
_DRV_IR_PROC__RX_DONE:
_DRV_IR_PROC__RX_SKIP:
	LDD TEMP_H,Y+_DRV_IR_STATE
	CPI TEMP_H,_DRV_IR_ST_TXDONE
	BRNE PC+0x02
	LDI TEMP_H,DRV_IR_ST_READY
	LDD TEMP_L,Y+_DRV_IR_BYTE_POS

	POP TEMP
	POP ACCUM
	POP_Z
	POP_Y
	RET

;--------------------------------------------------------
_DRV_IR_TIMER_C_EVENT:
;--------------------------------------------------------
;Обработчик прерывания от таймера C
;IN: PID,Y-адрес на выделенную память
;--------------------------------------------------------
	PUSH_X
	PUSH TEMP
	PUSH ACCUM

	LDD ACCUM,Y+_DRV_IR_STATE
	SBRS ACCUM,0x04
	RJMP _DRV_IR_TIMER_C_EVENT__CNTR_DEC
_DRV_IR_TIMER_C_EVENT__PULSE:
	LDD TEMP,Y+_DRV_IR_TX_PA
	LDD ACCUM,Y+_DRV_IR_TX_PN
	MCALL REG_BIT_INV
_DRV_IR_TIMER_C_EVENT__CNTR_DEC:
	LDD XH,Y+_DRV_IR_PULSE_CNTR+0x00
	LDD XL,Y+_DRV_IR_PULSE_CNTR+0x01
	SBIW XL,0x01
	BREQ _DRV_IR_TIMER_C_EVENT__SATE_CHECK
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
_DRV_IR_TIMER_C_EVENT__DONE:
	POP ACCUM
	POP TEMP
	POP_X
	RET

_DRV_IR_TIMER_C_EVENT__SATE_CHECK:
	LDD ACCUM,Y+_DRV_IR_STATE
	CPI ACCUM,_DRV_IR_ST_PREN
	BRNE _DRV_IR_TIMER_C_EVENT__SCNPREN
	LDI TEMP,_DRV_IR_ST_PRDI
	STD Y+_DRV_IR_STATE,TEMP
	LDI_X _DRV_IR_PRDI
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	RJMP _DRV_IR_TIMER_C_EVENT__DONE
_DRV_IR_TIMER_C_EVENT__SCNPREN:
	CPI ACCUM,_DRV_IR_ST_PRDI
	BREQ PC+0x05
	CPI ACCUM,_DRV_IR_ST_DTDI0
	BREQ PC+0x03
	CPI ACCUM,_DRV_IR_ST_DTDI1
	BRNE _DRV_IR_TIMER_C_EVENT__SCNPRDI
	LDD TEMP,Y+_DRV_IR_BIT_POS
	CP TEMP,C0x00
	BRNE _DRV_IR_TIMER_C_EVENT__SKIP_ACCUM_LOADING
	LDD TEMP,Y+_DRV_IR_BYTE_POS
	PUSH_Z
	LDD ZH,Y+_DRV_IR_SRC_ADDR+0x00
	LDD ZL,Y+_DRV_IR_SRC_ADDR+0x01
	MOV ACCUM,ZH
	ANDI ZH,0x7f
	ADD ZL,TEMP
	ADC ZH,C0x00
	INC TEMP
	STD Y+_DRV_IR_BYTE_POS,TEMP
	SBRC ACCUM,0x07
	RJMP PC+0x03
	LD TEMP,Z
	RJMP PC+02
	LPM TEMP,Z
	POP_Z
	STD Y+_DRV_IR_ACCUM,TEMP
_DRV_IR_TIMER_C_EVENT__SKIP_ACCUM_LOADING:
	LDI TEMP,_DRV_IR_ST_DTEN
	STD Y+_DRV_IR_STATE,TEMP
	LDI_X _DRV_IR_DTEN
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	RJMP _DRV_IR_TIMER_C_EVENT__DONE
_DRV_IR_TIMER_C_EVENT__SCNPRDI:
	CPI ACCUM,_DRV_IR_ST_DTEN
	BRNE _DRV_IR_TIMER_C_EVENT__SCNDTEN
	LDD TEMP,Y+_DRV_IR_BYTE_POS
	LDD ACCUM,Y+_DRV_IR_TX_LEN
	CP ACCUM,TEMP
	BRCC PC+0x06
	MCALL C5_TIMER_C_STOP
	LDI TEMP,_DRV_IR_ST_TXDONE
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_TIMER_C_EVENT__DONE
	;Инкрементируем номер бита
	LDD TEMP,Y+_DRV_IR_BIT_POS
	INC TEMP
	ANDI TEMP,0x07
	STD Y+_DRV_IR_BIT_POS,TEMP
	;Считываем бит
	LDD ACCUM,Y+_DRV_IR_ACCUM
	LSR ACCUM
	STD Y+_DRV_IR_ACCUM,ACCUM
	LDI TEMP,_DRV_IR_ST_DTDI0
	LDI_X _DRV_IR_DTDI0
	BRCC PC+0x04
	LDI TEMP,_DRV_IR_ST_DTDI1
	LDI_X _DRV_IR_DTDI1
	STD Y+_DRV_IR_STATE,TEMP
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	RJMP _DRV_IR_TIMER_C_EVENT__DONE

_DRV_IR_TIMER_C_EVENT__SCNDTEN:
	CPI ACCUM,_DRV_IR_ST_PR
	BRNE _DRV_IR_TIMER_C_EVENT__NO_RXPR
	;Достигли таймаута
	MCALL C5_TIMER_C_STOP
	LDI TEMP,_DRV_IR_ST_DTW
	STD Y+_DRV_IR_STATE,TEMP
	STD Y+_DRV_IR_BYTE_POS,C0x00
	STD Y+_DRV_IR_BIT_POS,C0x00
	MCALL __DRV_IR_EINT_LED_OFF
	RJMP _DRV_IR_TIMER_C_EVENT__DONE

_DRV_IR_TIMER_C_EVENT__NO_RXPR:
	CPI ACCUM,_DRV_IR_ST_DT
	BRNE _DRV_IR_TIMER_C_EVENT__NO_RX_DT
	;Достигли таймаута
	MCALL C5_TIMER_C_STOP
	LDI TEMP,DRV_IR_ST_READY
	STD Y+_DRV_IR_STATE,TEMP
	MCALL __DRV_IR_EINT_LED_OFF
_DRV_IR_TIMER_C_EVENT__NO_RX_DT:
	RJMP _DRV_IR_TIMER_C_EVENT__DONE

;--------------------------------------------------------
_DRV_IR_EINT_EVENT:
;--------------------------------------------------------
;Обработчик внешнего прерывания
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP
	PUSH ACCUM

	MCALL C5_RAM_OFFSET_Y

	LDD TEMP,Y+_DRV_IR_STATE
	CPI TEMP,_DRV_IR_ST_DTW
	BRNE _DRV_IR_EINT_EVENT__NO_DTW
	;Запоминаем метку времени для TIMEOUT(x2ms)
	LDI_X _DRV_IR_PRMAX
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	LDD TEMP,Y+_DRV_IR_FREQ
	MCALL C5_TIMER_C_START
	LDI TEMP,_DRV_IR_ST_PR
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END

_DRV_IR_EINT_EVENT__NO_DTW:
	CPI TEMP,_DRV_IR_ST_PR
	BRNE _DRV_IR_EINT_EVENT__NO_PR
	;Проверяем на минимальную преамбулу
	LDD XH,Y+_DRV_IR_PULSE_CNTR+0x00
	LDD XL,Y+_DRV_IR_PULSE_CNTR+0x01
	LDI TEMP,high(_DRV_IR_PRLEFT)
	CP TEMP,XH
	BRCS _DRV_IR_EINT_EVENT__RST
	BRNE PC+0x04
	LDI TEMP,low(_DRV_IR_PRLEFT)
	CP TEMP,XL
	BRCS _DRV_IR_EINT_EVENT__RST
	;Включаем индикатор активности
	LDD TEMP,Y+_DRV_IR_LED_PA
	LDD ACCUM,Y+_DRV_IR_LED_PN
	CP TEMP,C0xff
	BREQ PC+0x03
	MCALL REG_BIT_HI
_DRV_IR_EINT_EVENT__DT_SKIP_ACCUM_SAVE:
	;Устанавливаем таймаут для данных
	LDI_X _DRV_IR_DTMAX
	STD Y+_DRV_IR_PULSE_CNTR+0x00,XH
	STD Y+_DRV_IR_PULSE_CNTR+0x01,XL
	LDI TEMP,_DRV_IR_ST_DT
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
_DRV_IR_EINT_EVENT__NO_PR:
	CPI TEMP,_DRV_IR_ST_DT
	BRNE _DRV_IR_EINT_EVENT__END
	;Проверяем на минимальную преамбулу
	LDD XH,Y+_DRV_IR_PULSE_CNTR+0x00
	LDD XL,Y+_DRV_IR_PULSE_CNTR+0x01
	LDI TEMP,high(_DRV_IR_DTLEFT)
	CP TEMP,XH
	BRCS _DRV_IR_EINT_EVENT__RST
	BRNE PC+0x04
	LDI TEMP,low(_DRV_IR_DTLEFT)
	CP TEMP,XL
	BRCS _DRV_IR_EINT_EVENT__RST
	LDD TEMP,Y+_DRV_IR_BIT_POS
	LDD ACCUM,Y+_DRV_IR_ACCUM
	CPI XL,_DRV_IR_DTTRHLD
	ROR ACCUM
	STD Y+_DRV_IR_ACCUM,ACCUM
	INC TEMP
	ANDI TEMP,0x07
	STD Y+_DRV_IR_BIT_POS,TEMP
	BRNE _DRV_IR_EINT_EVENT__DT_SKIP_ACCUM_SAVE
	LDD TEMP,Y+_DRV_IR_BYTE_POS
	LDD ACCUM,Y+_DRV_IR_ACCUM
	PUSH_Z
	LDD ZH,Y+_DRV_IR_DST_ADDR+0x00
	LDD ZL,Y+_DRV_IR_DST_ADDR+0x01
	ADD ZL,TEMP
	ADC ZH,C0x00
	ST Z,ACCUM
	POP_Z
	INC TEMP
	STD Y+_DRV_IR_BYTE_POS,TEMP
	LDD ACCUM,Y+_DRV_IR_RX_LEN
	INC ACCUM
	CP TEMP,ACCUM
	BRNE _DRV_IR_EINT_EVENT__DT_SKIP_ACCUM_SAVE
	;Достигли переполнения
	MCALL C5_TIMER_C_STOP
	LDD TEMP_H,Y+_DRV_IR_INT_ID
	MCALL C5_EINT_DISABLE
	LDI TEMP,DRV_IR_ST_OVERFLOW
	STD Y+_DRV_IR_STATE,TEMP
	RJMP _DRV_IR_EINT_EVENT__END
_DRV_IR_EINT_EVENT__RST:
	;Достигли переполнения
	MCALL C5_TIMER_C_STOP
	LDI TEMP,_DRV_IR_ST_DTW
	STD Y+_DRV_IR_STATE,TEMP
	STD Y+_DRV_IR_BYTE_POS,C0x00
	STD Y+_DRV_IR_BIT_POS,C0x00
	MCALL __DRV_IR_EINT_LED_OFF
	RJMP _DRV_IR_EINT_EVENT__END

_DRV_IR_EINT_EVENT__END:

	POP ACCUM
	POP TEMP
	POP_Y
	POP_X
	RET

__DRV_IR_EINT_LED_OFF:
	;Включаем индикатор активности
	LDD TEMP,Y+_DRV_IR_LED_PA
	LDD ACCUM,Y+_DRV_IR_LED_PN
	CP TEMP,C0xff
	BREQ PC+0x03
	MCALL REG_BIT_LO
	RET
.endif
