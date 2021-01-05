;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_COMMON
.else
.set DEF_PRIM_COMMON = 1

	;---SIZE---
	.EQU	LONG_SIZE								= 0x08
	.EQU	UPTIME_SIZE								= 0x05
	.EQU	INT_SIZE									= 0x04
	.EQU	SHORT_SIZE								= 0x02
	.EQU	BYTE_SIZE								= 0x01

.MACRO PRIM_SUB
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,@0
	MCALL PRIM_SUB
	POP LOOP_CNTR
.ENDMACRO

.MACRO PRIM_CP
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,@0
	MCALL PRIM_CP
	POP LOOP_CNTR
.ENDMACRO


.endif
