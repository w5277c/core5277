;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_COMMON
.else
.set DEF_PRIM_COMMON = 1

	;---TYPE-AND-SIZE---
	.EQU	PRIM_BYTE								= 0x11
	.EQU	PRIM_SHORT								= 0x22
	.EQU	PRIM_SDNF								= 0x32
	.EQU	PRIM_INT									= 0x44
	.EQU	PRIM_FLOAT								= 0x54
	.EQU	PRIM_UPTIME								= 0x65
	.EQU	PRIM_LONG								= 0x78
	.EQU	PRIM_DOUBLE								= 0x88

.MACRO PRIM_SUB
;	.IF @0=SDNF
;	.ELSEIF @0=FLOAT
;	.ELSEIF @0=DOUBLE
;	.ELSE
		PUSH LOOP_CNTR
		LDI LOOP_CNTR,(@0 & 0x0f)
		MCALL PRIM_SUB
		POP LOOP_CNTR
;	.ENDIF
.ENDMACRO

.MACRO PRIM_CP
;	.IF @0=SDNF
;	.ELSEIF @0=FLOAT
;	.ELSEIF @0=DOUBLE
;	.ELSE
		PUSH LOOP_CNTR
		LDI LOOP_CNTR,(@0 & 0x0f)
		MCALL PRIM_CP
		POP LOOP_CNTR
;	.ENDIF
.ENDMACRO


.endif
