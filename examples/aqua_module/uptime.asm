;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;30.08.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/core/wait_1s.inc"
.include	"./inc/io/log_bytes.inc"
.include	"./inc/io/log_romstr.inc"

TSK_UPTIME_INIT:
	LDI ACCUM,0x05
	MCALL CORE5277_RAM_REALLOC

	MCALL CORE5277_READY
;--------------------------------------------------------
TSK_UPTIME_LOOP:
	MOV YH,ZH
	MOV YL,ZL

	MCALL CORE5277_DISPATCHER_LOCK
	MCALL CORE5277_UPTIME_WRITE
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE
	LDI TEMP,0x05
	MCALL CORE5277_LOG_BYTES
	CORE5277_LOG_ROMSTR LOGSTR_NEW_LINE
	MCALL CORE5277_DISPATCHER_UNLOCK

	LDI TEMP,0x01
	MCALL CORE5277_WAIT_1S

	RJMP TSK_UPTIME_LOOP
