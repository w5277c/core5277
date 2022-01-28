;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;04.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;28.01.2022	w5277c@gmail.com			Код переработан(не тестировано)
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_RES_UNLOCK
.else
.set DEF_C5_RES_UNLOCK = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_RES_UNLOCK"
.endif

.include "./core/notify.inc"
.include "./mem/ram_copy.inc"

;--------------------------------------------------------
C5_RES_UNLOCK:
;--------------------------------------------------------
;Освобождаем ресурс
;IN:ACCUM-ID ресурса
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP
	PUSH FLAGS
	PUSH LOOP_CNTR

	MCALL C5_DISPATCHER_LOCK
	LDI_Z _C5_RESOURCE_QUEUE
	LDI LOOP_CNTR,_C5_RES_QUEUE_SIZE/0x02						;Размер очереди должен быть кратный двум
_C5_RES_UNLOCK__QUEUE_LOOP:
	LD TEMP,Z+															;Получаем из очереди RES_ID
	LD FLAGS,Z+															;Получаем из очереди PID задачи
	CPI FLAGS,0xff														;Проверка на конец списка
	BREQ _C5_RES_UNLOCK__END
	CP TEMP,ACCUM
	BREQ _C5_RES_UNLOCK__CONTAINS										;Запись найдена
	DEC LOOP_CNTR
	BRNE _C5_RES_UNLOCK__QUEUE_LOOP
	RJMP _C5_RES_UNLOCK__END

_C5_RES_UNLOCK__CONTAINS:
	DEC LOOP_CNTR
	BREQ _C5_RES_UNLOCK__DATA_MOVE_SKIP
	MOVW XL,ZL
	SBIW XL,0x02
	LSL LOOP_CNTR
	MCALL RAM_COPY
_C5_RES_UNLOCK__DATA_MOVE_SKIP:
	STS _C5_RESOURCE_QUEUE+_C5_RES_QUEUE_SIZE-0x01,C0xff

	MCALL C5_NOTIFY

_C5_RES_UNLOCK__END:
	MCALL C5_DISPATCHER_UNLOCK

	POP LOOP_CNTR
	POP FLAGS
	POP TEMP
	POP_Z
	POP_X
	RET
.endif
