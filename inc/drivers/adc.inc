;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;15.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/mem/ram_offset.inc"
.include	"./inc/mem/ram_realloc.inc"
.include	"./inc/core/wait.inc"
.include	"./inc/core/res_lock.inc"
.include	"./inc/core/res_unlock.inc"
.include	"./inc/math/div32x16.inc"

;---CONSTANTS--------------------------------------------
	.EQU	_DRV_ADC_RAM_SIZE									= 0x0a;Размер используемой памяти
	.EQU	_DRV_ADC_ST_DONE									= 0x00;Завершено
	.EQU	_DRV_ADC_ST_BUSY									= 0x01;Занято
	;---DATA---
	.EQU	_DRV_ADC_TIMER_ID									= 0x00;1B - ид таймера
	.EQU	_DRV_ADC_TIMER_PERIOD							= 0x01;1B - период таймера
	.EQU	_DRV_ADC_ITER_QNT									= 0x02;1B - количество итераций
	.EQU	_DRV_ADC_ITER_CNTR								= 0x03;1B - счетчик итераций
	.EQU	_DRV_ADC_ZERO_VAL									= 0x04;2B - значение нуля (15-й бит - суммировать по модулю)
	.EQU	_DRV_ADC_STATE										= 0x06;1B - состояние драйвера
	.EQU	_DRV_ADC_SUM										= 0x07;3B - сумма показаний

;--------------------------------------------------------
DRV_ADC_INIT:
;--------------------------------------------------------
;Инициализация
;IN: ACCUM - ид таймера
;--------------------------------------------------------
	PUSH ACCUM
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH_Z

	PUSH ACCUM
	;Выделяем память
	LDI ACCUM,_DRV_ADC_RAM_SIZE
	MCALL C5_RAM_REALLOC
	POP ACCUM

	;Устанавливаю параметры таймера
	STD Z+_DRV_ADC_TIMER_ID,ACCUM
	MOV TEMP,ACCUM
	LDI TEMP_H,high(_DRV_ADC_TIMER_EVENT)
	LDI TEMP_L,low(_DRV_ADC_TIMER_EVENT)
	LDI ACCUM,0x01
	MCALL C5_TIMER_SET

.IFDEF ADCSRB
	STS ADCSRB,ACCUM
.ENDIF
	STS ADMUX,ACCUM
	STS ADCSRA,ACCUM

	;Прописываем прерывание
	LDI TEMP_H,high(_DRV_ADC_EVENT)
	LDI TEMP_L,low(_DRV_ADC_EVENT)
	LDI ACCUM,C5_IR_ADC
	MCALL C5_IR_VECTOR_SET

	;Завершаем инициализацию
	POP_Z
	POP TEMP_L
	POP TEMP_H
	POP ACCUM
	MCALL C5_READY
;--------------------------------------------------------
;Основной код.
;IN: ACCUM - ADC порт(ADC0...)
;TEMP_EH - опорное напряжение(VREF), TEMP_EL - делитель,
;TEMP_H - кол-во итераций для вычисления среднего,
;TEMP_L - период таймера для итераций
;X - значение нуля(15-й бит - считать по модулю)
;OUT: TEMP_H,TEMP_L - результат (15-бит знак минуса)
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP
	PUSH ACCUM

	;Блокируем ресурс ADC
	PUSH ACCUM
;	LDI ACCUM,C5_RES_ADC
;	MCALL C5_RES_LOCK
	POP ACCUM

	;Увеличиваем размер выделенной памяти на _DRV_ADC_STRUCT_SIZE
	MCALL C5_RAM_OFFSET

	STD Z+_DRV_ADC_ZERO_VAL+0x00,XH
	STD Z+_DRV_ADC_ZERO_VAL+0x01,XL

	LDI TEMP,0x00
	STD Z+_DRV_ADC_SUM+0x00,TEMP
	STD Z+_DRV_ADC_SUM+0x01,TEMP
	STD Z+_DRV_ADC_SUM+0x02,TEMP
	STD Z+_DRV_ADC_ITER_QNT,TEMP_H
	STD Z+_DRV_ADC_ITER_CNTR,TEMP

	;Выставляем состояние - занято
	LDI TEMP,_DRV_ADC_ST_BUSY
	STD Z+_DRV_ADC_STATE,TEMP
	;Включаю ADC
	_C5_POWER_ON PRADC
	;Задаю параметры ADC и запускаю
	OR ACCUM,TEMP_EH
	STS ADMUX,ACCUM													;ADLAR=0 - направление представления байт (right)
	ORI TEMP_EL,(0<<ADEN)|(1<<ADSC)|(1<<ADIE)|(1<<ADATE)	;ADIE=1 - Разрешаем прерывание по окончанию кнвертирования
	STS ADCSRA,TEMP_EL												;ADATE=0 - Выключен режим AutoTriggering


	;Есть вообще что делать?
	CPI TEMP_H,0x00
	BREQ DRV_ADC__WAIT_END

	;Необходимо использвать таймер?
	CPI TEMP_H,0x02
	BRCS DRV_ADC__NO_TIMER
	LDD TEMP,Z+_DRV_ADC_TIMER_ID
	MOV ACCUM,TEMP_L
	MCALL C5_TIMER_SET_PERIOD
	MCALL C5_TIMER_START
	RJMP DRV_ADC__WAIT_RESULT
DRV_ADC__NO_TIMER:
	LDS TEMP,ADCSRA
	ORI TEMP,(1<<ADEN)												;Запускаем ADC
	STS ADCSRA,TEMP
DRV_ADC__WAIT_RESULT:
	;Ждем результат
	MCALL C5_WAIT
	LDD TEMP,Z+_DRV_ADC_STATE
	CPI TEMP,_DRV_ADC_ST_DONE
	BRNE DRV_ADC__WAIT_RESULT

	;Выключаю ADC
	_C5_POWER_OFF PRADC

	CLR XH
	MOV XL,TEMP_H
	CLR TEMP_EH
	LDD TEMP_EL,Z+_DRV_ADC_SUM+0x00
	LDD TEMP_H,Z+_DRV_ADC_SUM+0x01
	LDD TEMP_L,Z+_DRV_ADC_SUM+0x02
	MCALL C5_DIV32X16

	;Разблокируем ресурс ADC
;	LDI ACCUM,C5_RES_ADC
;	MCALL C5_RES_UNLOCK

DRV_ADC__WAIT_END:
	POP ACCUM
	POP TEMP
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_X
	RET

;--------------------------------------------------------
_DRV_ADC_TIMER_EVENT:
;--------------------------------------------------------
;Обработчик по таймеру
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z

	MCALL C5_RAM_OFFSET
	LDD TEMP,Z+_DRV_ADC_STATE
	CPI TEMP,_DRV_ADC_ST_DONE
	BREQ _DRV_ADC_TIMER_EVENT__STOP
	;Еще раз запускаю ADC
	LDS TEMP,ADCSRA
	ORI TEMP,(1<<ADEN)|(1<<ADSC)
	STS ADCSRA,TEMP
	RJMP _DRV_ADC_TIMER_EVENT__END
_DRV_ADC_TIMER_EVENT__STOP:
	LDD TEMP,Z+_DRV_ADC_TIMER_ID
	MCALL C5_TIMER_STOP
_DRV_ADC_TIMER_EVENT__END:

	POP_Z
	POP TEMP
	RET
;--------------------------------------------------------
_DRV_ADC_EVENT:
;--------------------------------------------------------
;Обработчик перывания по завершенному вычислению ADC
;--------------------------------------------------------
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH_Z

	LDS TEMP_L,ADCL
	LDS TEMP_H,ADCH

	MCALL C5_RAM_OFFSET
	LDD TEMP_EH,Z+_DRV_ADC_ZERO_VAL+0x00
	LDD TEMP_EL,Z+_DRV_ADC_ZERO_VAL+0x01
	;Проверка на вычисление по модулю
	SBRS TEMP_EH,0x07
	RJMP _DRV_ADC_EVENT__NO_MOD
	ANDI TEMP_EH,0x7f
	;Если полученное значение больше значения нуля, то пропускаем
	CP TEMP_EH,TEMP_H
	BRCS _DRV_ADC_EVENT__NO_MOD
	BRNE PC+0x03
	CP TEMP_EL,TEMP_L
	BRCS _DRV_ADC_EVENT__NO_MOD
	;Меняем местами
	MOV TEMP,TEMP_EH
	MOV TEMP_EH,TEMP_H
	MOV TEMP_H,TEMP
	MOV TEMP,TEMP_EL
	MOV TEMP_EL,TEMP_L
	MOV TEMP_L,TEMP
_DRV_ADC_EVENT__NO_MOD:
	SUB TEMP_L,TEMP_EL
	SBC TEMP_H,TEMP_EH
	;Прибавляю к сумме
	LDD TEMP,Z+_DRV_ADC_SUM+0x02
	ADD TEMP,TEMP_L
	STD Z+_DRV_ADC_SUM+0x02,TEMP
	LDD TEMP,Z+_DRV_ADC_SUM+0x01
	ADC TEMP,TEMP_H
	STD Z+_DRV_ADC_SUM+0x01,TEMP
	LDI TEMP_L,0x00
	LDD TEMP,Z+_DRV_ADC_SUM+0x00
	ADC TEMP,TEMP_L
	STD Z+_DRV_ADC_SUM+0x00,TEMP

	;Инкрементирую итерацию и проверяю на конец
	LDD TEMP,Z+_DRV_ADC_ITER_CNTR
	INC TEMP
	STD Z+_DRV_ADC_ITER_CNTR,TEMP
	LDD TEMP_L,Z+_DRV_ADC_ITER_QNT
	CP TEMP_L,TEMP
	BRNE _DRV_ADC_EVENT__NEXT
	;Отключаю таймер и выставляю состояние DONE
	LDD TEMP,Z+_DRV_ADC_TIMER_ID
	MCALL C5_TIMER_STOP
	LDI TEMP,_DRV_ADC_ST_DONE
	STD Z+_DRV_ADC_STATE,TEMP
_DRV_ADC_EVENT__NEXT:

	POP_Z
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	RET
