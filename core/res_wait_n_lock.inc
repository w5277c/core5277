;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;04.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;28.01.2022	w5277c@gmail.com			Код переработан(не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RES_WAIT_N_LOCK
.else
.set DEF_C5_RES_WAIT_N_LOCK = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RES_WAIT_N_LOCK"
.endif

.include "./core/wait.inc"

;--------------------------------------------------------
C5_RES_WAIT_N_LOCK:
;--------------------------------------------------------
;Занимаем ресурс, если ресурс занят, то ждем освобождения
;IN:ACCUM-ID ресурса,
;TEMP_H/TEMP_L/TEMP-время ожидания в тиках ядра(2ms),
;0x000000-не ждать
;OUT:флаг Z-результат(HI-успех)
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR
	PUSH FLAGS

	LDS LOOP_CNTR,SREG												;Получаем UPTIME
	CLI
	LDS ZH,_C5_UPTIME+0x02
	LDS ZL,_C5_UPTIME+0x03
	LDS FLAGS,_C5_UPTIME+0x04
	STS SREG,LOOP_CNTR
	ADD TEMP,FLAGS														;Вычисляем точку времени таймаута
	ADC TEMP_L,ZL
	ADC TEMP_H,ZH

_C5_RES_WAIT_N_LOCK__WAIT_LOOP:
	MCALL C5_DISPATCHER_LOCK
	LDI_Z _C5_RESOURCE_QUEUE
	LDI LOOP_CNTR,_C5_RES_QUEUE_SIZE/0x02						;Размер очереди должен быть кратный двум
_C5_RES_WAIT_N_LOCK__QUEUE_LOOP:
	LDD TEMP,Z+0x00													;Получаем из очереди RES_ID
	CPI TEMP,0xff														;Проверка на конец списка
	BREQ _C5_RES_WAIT_N_LOCK__UNLOCKED
	CP TEMP,ACCUM
	BRNE PC+0x06														;Не наш ресурс
	LDD TEMP,Z+0x01													;Получаем из очереди PID задачи
	CPI TEMP,0xff
	BRNE PC+0x02														;Ид ожидающей задачи уже задан
	STD Z+0x01,_PID													;Записываем ид ожидающей задачи
	RJMP _C5_RES_WAIT_N_LOCK__WAIT								;Ресурс занят
	ADIW ZL,0x02
	DEC LOOP_CNTR
	BRNE _C5_RES_WAIT_N_LOCK__QUEUE_LOOP

_C5_RES_WAIT_N_LOCK__WAIT:											;Очередь переполнена или ресурс занят, ждем.
	MCALL C5_DISPATCHER_UNLOCK
	LDS LOOP_CNTR,SREG												;Получаем UPTIME
	CLI
	LDS ZH,_C5_UPTIME+0x02
	LDS ZL,_C5_UPTIME+0x03
	LDS FLAGS,_C5_UPTIME+0x04
	STS SREG,LOOP_CNTR
	CP TEMP_H,ZH														;Проверяем на таймаут
	BRCS _C5_RES_WAIT_N_LOCK__FAIL
	BRNE PC+0x06
	CP TEMP_L,ZL
	BRCS _C5_RES_WAIT_N_LOCK__FAIL
	BRNE PC+0x03
	CP FLAGS,TEMP
	BRCC _C5_RES_WAIT_N_LOCK__FAIL
	MCALL C5_WAIT
	RJMP _C5_RES_WAIT_N_LOCK__WAIT_LOOP

_C5_RES_WAIT_N_LOCK__FAIL:
	CLZ
	RJMP _C5_RES_WAIT_N_LOCK__END
_C5_RES_WAIT_N_LOCK__UNLOCKED:
	ST Z+,ACCUM															;Помещаем в очередь ид ресурса
	ST Z,C0xff															;Ид ожидаемой задачи не задан
	MCALL C5_DISPATCHER_UNLOCK
	SEZ

_C5_RES_WAIT_N_LOCK__END:
	POP FLAGS
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP_Z
	RET
.endif
