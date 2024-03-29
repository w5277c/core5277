;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.06.2020	w5277c@gmail.com        Начало
;02.08.2020	w5277c@gmail.com        Разбиение на файлы
;30.08.2020	w5277c@gmail.com        Устранены основные ошибки
;27.10.2020	w5277c@gmail.com        Обновлена информация об авторских правах
;05.07.2021	w5277c@gmail.com			Z->Y
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_TASKSDUMP
.else
.set DEF_C5_OUT_TASKSDUMP = 1
.ifdef LOGGING_PORT

.include	"./core/io/out_taskdump.inc"
.include	"./core/io/out_bytes.inc"
.include	"./core/io/out_romstr.inc"
.include	"./core/io/outstr_done.inc"

_OUTSTR_TASKSDUMP:
	.db   0x0d,0x0a,"---TASKSDUMP:",0x00
_OUTSTR_RES_QUEUE:
	.db   0x0d,0x0a,"RQD:",0x00,0x00

;--------------------------------------------------------
C5_OUT_TASKSDUMP:
;--------------------------------------------------------
;Логирование информации о задачах
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z
	PUSH PID

	C5_OUT_ROMSTR _OUTSTR_TASKSDUMP

;;TODO вводить причину рестарта
;;	;Выводим дамп ядра, если перезагрузка не по питанию и не по пину сброса
;;	IN TEMP,MCUSR
;;	ANDI TEMP,(1<<BORF)|(1<<EXTRF)|(1<<PORF)
;;	BRNE PC+0x03


	C5_OUT_ROMSTR _OUTSTR_RES_QUEUE
	LDI_Z _C5_RESOURCE_QUEUE
	LDI TEMP,_C5_RES_QUEUE_SIZE
	MCALL C5_OUT_BYTES

	LDI PID,0x00
_C5_OUT_TASKSDUMP__TASK_LOOP:
	MCALL C5_OUT_TASKDUMP
	INC PID
	CPI PID,C5_TASKS_QNT
	BRNE _C5_OUT_TASKSDUMP__TASK_LOOP
	C5_OUT_ROMSTR OUTSTR_DONE

	POP PID
	POP_Z
	POP TEMP
	RET
.endif
.endif
