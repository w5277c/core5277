;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020  w5277c@gmail.com        Начало
;02.08.2020  w5277c@gmail.com        Разбиение на файлы
;30.08.2020  w5277c@gmail.com        Устранены основные ошибки
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_LOG_TASKSDUMP
.else
.set DEF_C5_LOG_TASKSDUMP = 1
.ifdef LOGGING_PORT

.include	"./inc/io/log_taskdump.inc"
.include	"./inc/io/log_bytes.inc"
.include	"./inc/io/log_romstr.inc"
.include	"./inc/io/logstr_done.inc"
.include	"./inc/io/logstr_new_line.inc"

_LOGSTR_TASKSDUMP:
	.db   0x0d,0x0a,"---TASKSDUMP:",0x00
_LOGSTR_RES_QUEUE:
	.db   0x0d,0x0a,"RQD:",0x00,0x00

;--------------------------------------------------------
C5_LOG_TASKSDUMP:
;--------------------------------------------------------
;Логирование информации о задачах
;--------------------------------------------------------
	PUSH TEMP
	PUSH YH
	PUSH YL
	PUSH PID

	C5_LOG_ROMSTR _LOGSTR_TASKSDUMP

;;TODO вводить причину рестарта
;;	;Выводим дамп ядра, если перезагрузка не по питанию и не по пину сброса
;;	IN TEMP,MCUSR
;;	ANDI TEMP,(1<<BORF)|(1<<EXTRF)|(1<<PORF)
;;	BRNE PC+0x03


	C5_LOG_ROMSTR _LOGSTR_RES_QUEUE
	LDI YH,high(_C5_RESOURCE_QUEUE)
	LDI YL,low(_C5_RESOURCE_QUEUE)
	LDI TEMP,_C5_RES_QUEUE_SIZE
	MCALL C5_LOG_BYTES

	LDI PID,0x00
_C5_LOG_TASKSDUMP__TASK_LOOP:
	MCALL C5_LOG_TASKDUMP
	INC PID
	CPI PID,_C5_TASKS_MAX_QNT
	BRNE _C5_LOG_TASKSDUMP__TASK_LOOP
	C5_LOG_ROMSTR LOGSTR_DONE

	POP PID
	POP YL
	POP YH
	POP TEMP
	RET
.endif
.endif
