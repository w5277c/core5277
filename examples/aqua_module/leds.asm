;-----------------------------------------------------------------------------------------------------------------------
;Разработчиком и полноправным владельцем данного исходного кода является Удовиченко Константин Александрович,
;емайл:w5277c@gmail.com, по всем правовым вопросам обращайтесь на email.
;-----------------------------------------------------------------------------------------------------------------------
;06.09.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./inc/core/wait_2ms.inc"
.include	"./inc/io/port_mode_out.inc"
.include	"./inc/io/port_set_hi.inc"
.include	"./inc/io/port_set_lo.inc"

TSK_LEDS_DATA:
	.db	PB4,PC3,PB2,PB0,PB1,PD6,PD5,PD6,PB1,PB0,PB2,PC3,0xff

TSK_LEDS_INIT:
	MCALL CORE5277_READY
;--------------------------------------------------------
TSK_LEDS_INFINITE_LOOP:
	LDI ZH,high(TSK_LEDS_DATA*0x02)
	LDI ZL,low(TSK_LEDS_DATA*0x02)

TSK_LEDS_LED_LOOP:
	LPM ACCUM,Z+
	CPI ACCUM,0xff
	BREQ TSK_LEDS_INFINITE_LOOP

	MCALL CORE5277_PORT_MODE_OUT
	MCALL CORE5277_PORT_SET_HI

	LDI TEMP_H,0x00													;Пауза в 100 мс
	LDI TEMP_L,0x00
	LDI TEMP,0x32
	MCALL CORE5277_WAIT_2MS

	MCALL CORE5277_PORT_SET_LO
	RJMP TSK_LEDS_LED_LOOP
