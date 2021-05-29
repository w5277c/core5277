;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com			Начало
;09.08.2020  w5277c@gmail.com			Разбиение на файлы
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;22.01.2020  w5277c@gmail.com			Учет CORE_FREQ
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
.IF TS_MODE == TS_MODE_TIME
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
.ELSE
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
C5_WAIT_2MS__LOOP1:
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,(59*CORE_FREQ)/10
C5_WAIT_2MS__LOOP2:
	LDI TEMP_L,0x4f
C5_WAIT_2MS__LOOP3:
	NOP
	DEC TEMP_L
	BRNE C5_WAIT_2MS__LOOP3
	DEC TEMP_H
	BRNE C5_WAIT_2MS__LOOP2
	POP TEMP_L
	POP TEMP_H

	SUBI TEMP,0x01
	SBCI TEMP_L,0x00
	SBCI TEMP_H,0x00
	CPI TEMP,0x00
	BRNE C5_WAIT_2MS__LOOP1
	CPI TEMP_L,0x00
	BRNE C5_WAIT_2MS__LOOP1
	CPI TEMP_H,0x00
	BRNE C5_WAIT_2MS__LOOP1

	POP TEMP
	POP TEMP_L
	POP TEMP_H
.ENDIF
	RET
.endif
