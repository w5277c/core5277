;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_UPTIME_CP
.else
.set DEF_UPTIME_CP = 1

.include	"./prim/common.inc"

;--------------------------------------------------------
UPTIME_CP:
;--------------------------------------------------------
;Выичтание UPTIME'ов
;IN: X-адрес арг.1, Y-адрес арг.2
;OUT: Flags C - арг.2>арг.1, Flag Z - арг.1=арг.2
;--------------------------------------------------------
	PUSH_X
	PUSH_Y
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	LDI LOOP_CNTR,UPTIME_SIZE
	CLC
	LDS TEMP,SREG
__UPTIME_CP__LOOP:
	STS SREG,TEMP
	LD TEMP_H,X+
	LD TEMP_L,Y+
	CP TEMP_H,TEMP_L
	BRNE __UPTIME_CP__END
	LDS TEMP,SREG
	DEC LOOP_CNTR
	BRNE __UPTIME_CP__LOOP
__UPTIME_CP__END:
	STS SREG,TEMP

	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP
	POP_Y
	POP_X
.endif
