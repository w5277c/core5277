;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;14.11.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_MEMINFO_COPY
.else
.set DEF_C5_MEMINFO_COPY = 1

.if REPORT_INCLUDES == 0x01
.message "included C5_MEMINFO_COPY"
.endif

;--------------------------------------------------------
C5_MEMINFO_COPY:
;--------------------------------------------------------
;Записываем в память 4 байта:
;2Б - кол-во свободной памяти
;2Б - всего доступной памяти
;IN: Y - адрес для записи
;--------------------------------------------------------
	PUSH TEMP
	PUSH_Z
	PUSH_X

	LDS TEMP,SREG
	CLI
	LDS ZH,_C5_TOP_OF_FREE_RAM+0x00
	LDS ZL,_C5_TOP_OF_FREE_RAM+0x01
	LDS XH,_C5_TOP_OF_STACK+0x00
	LDS XL,_C5_TOP_OF_STACK+0x01
	STS SREG,TEMP

	SUB XL,ZL
	STD Y+0x01,XL
	SBC XH,ZH
	STD Y+0x00,XH
	LDI TEMP,low((_C5_STACK_END-_C5_FREE_RAM)-_C5_RAM_BORDER_SIZE)
	STD Y+0x03,TEMP
	LDI TEMP,high((_C5_STACK_END-_C5_FREE_RAM)-_C5_RAM_BORDER_SIZE)
	STD Y+0x02,TEMP

	POP_X
	POP_Z
	POP TEMP
	RET
.endif
