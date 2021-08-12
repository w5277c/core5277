;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019	w5277c@gmail.com			Начало
;09.08.2020	w5277c@gmail.com			Разбиение на файлы
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;22.01.2020	w5277c@gmail.com			Учет CORE_FREQ
;31.05.2021	w5277c@gmail.com			Реализация на таймере
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_WAIT_2MS
.else
.set DEF_C5_WAIT_2MS = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_WAIT_2MS"
.endif

;TODO TIMER_MARK?
;--------------------------------------------------------
C5_WAIT_2MS:
;--------------------------------------------------------
;Ждем истечения времени с момента прошлого сна или
;с момента вызова C5_TIMER_MARK
;IN TEMP_H,TEMP_L,TEMP - время в 0.002s
;--------------------------------------------------------
.if TS_MODE == TS_MODE_TIME
	LDS r2,SREG															;Запоминаем регистр флагов
	MCALL C5_DISPATCHER_LOCK
	_C5_MACRO__PUSH_RDS												;Пмещаем в стек рабочие регистры задачи
	PUSH r2																;Помещаем в стек регистр флагов

	MOV PID,_PID														;Затираю PID, так как он уже записан в стек
	MCALL _C5_PROC_HEADER_GET

	MOV r3,ACCUM														;Точно необходимо?
	;Записываем время окончания паузы
	LDS ACCUM,SREG
	PUSH ACCUM
	CLI
	LDS ACCUM,_C5_UPTIME+0x04
	ADD ACCUM,TEMP
	STD Z+_C5_TASK_EXECTIME+0x02,ACCUM
	LDS ACCUM,_C5_UPTIME+0x03
	ADC ACCUM,TEMP_L
	STD Z+_C5_TASK_EXECTIME+0x01,ACCUM
	LDS ACCUM,_C5_UPTIME+0x02
	ADC ACCUM,TEMP_H
	STD Z+_C5_TASK_EXECTIME+0x00,ACCUM
	POP ACCUM
	STS SREG,ACCUM
	MOV ACCUM,r3

	LDI TEMP,_C5_PROC_STATE_TIME_WAIT
	MCALL _C5_SUSPEND__BODY
.else
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH ACCUM
	PUSH FLAGS

	LDS ACCUM,_C5_UPTIME+0x04
C5_WAIT_2MS__LOOP:
	LDS FLAGS,_C5_UPTIME+0x04
	SUB FLAGS,ACCUM
	BREQ C5_WAIT_2MS__LOOP

	LDS ACCUM,_C5_UPTIME+0x04
	SUB TEMP,FLAGS
	SBC TEMP_L,C0x00
	SBC TEMP_H,C0x00
	BRCC C5_WAIT_2MS__LOOP

	POP FLAGS
	POP ACCUM
	POP TEMP
	POP TEMP_L
	POP TEMP_H
.endif
	RET
.endif
