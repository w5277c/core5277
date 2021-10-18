;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;16.10.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_OUT_UPTIME
.else
.set DEF_C5_OUT_UPTIME = 1
.ifdef LOGGING_PORT


.include	"./mem/ram_stk_alloc.inc"
.include	"./mem/ram_stk_free.inc"
.include	"./math/divnx16.inc"
.include	"./core/io/out_str.inc"

;--------------------------------------------------------
C5_OUT_UPTIME:
;--------------------------------------------------------
;Вывод десятеричного UPTIME в миллисекундах
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH_Z
	PUSH LOOP_CNTR
	PUSH FLAGS
	PUSH TEMP
	PUSH ACCUM
	LDI ACCUM,0x06+0x0e
	MCALL RAM_STK_ALLOC

	LDI_Z _C5_UPTIME+0x05
	ADIW YL,0x06
	LDI LOOP_CNTR,0x05
	LDS FLAGS,SREG
	CLI
_C5_OUT_UPTIME__LOOP1:
	LD TEMP,-Z
	ROL TEMP
	ST -Y,TEMP
	DEC LOOP_CNTR
	BRNE _C5_OUT_UPTIME__LOOP1
	ROL LOOP_CNTR
	ST -Y,LOOP_CNTR
	STS SREG,FLAGS

	MOVW ZL,YL
	ADIW ZL,0x06+0x0d
	LDI ACCUM,0x06
	LDI LOOP_CNTR,0x0d
_C5_OUT_UPTIME__LOOP2:
	LDI_X 0x0a	
	MCALL DIVNX16
	SUBI XL,(0x100-0x30)
	ST -Z,XL
	DEC LOOP_CNTR
	BRNE _C5_OUT_UPTIME__LOOP2
	STD Z+0x0d,C0x00

	MCALL C5_OUT_STR
	STS SREG,FLAGS
	

	LDI ACCUM,0x06+0x0e
	MCALL RAM_STK_FREE
	POP ACCUM
	POP TEMP
	POP FLAGS
	POP LOOP_CNTR
	POP_Z
	POP_Y
	POP_X
	RET
.endif
.endif
