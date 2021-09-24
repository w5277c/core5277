;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.01.2021  w5277c@gmail.com			Начало
;09.02.2021  w5277c@gmail.com			Поддержка фонового режима (буфер в 1 байт)
;08.04.2021  w5277c@gmail.com			Новая реализация, использую буфер ввода, не оттестировано
;09.04.2021  w5277c@gmail.com			Оттестирвано на INT0 и исправлены баги
;12.04.2021  w5277c@gmail.com			Оттестирвана программаня часть и исправлены баги
;22.04.2021  w5277c@gmail.com			Реализовано чтение непрерывного блока данных(COPY/PASTE), оттестировано на INT0
;05.09.2021	w5277c@gmail.com			INPUT_PORT->C5_IN_PORT
;-----------------------------------------------------------------------------------------------------------------------
;Читаем с порта UART последовательность, BAUDRATE идентичен логированию.
;Если порт поддержтвает внешние прерывания(INT0,...), то при ожидании старта прерывания будут разрешены,
;диспетчер также не блокируется, т.е. многопоточность не нарушается.
;Если порт не поддерживает внешние прерывания, то периодически выполняется проверка на UART START.
;Механизм без прерываний не надежен, может пропускать некоторые символы.
;Функционал имеет наивысший приоритет, т.е. при передаче по UART последовательности без пауз не выполняется ни одно
;прерывание, в качестве решения можно задать 2 для INPUT_BUFFER_SIZE(при TIMERS_SPEED_25US теряем 1 тик).
;-----------------------------------------------------------------------------------------------------------------------
.ifdef C5_IN_PORT
.ifdef DEF_INPUT
.else
.set DEF_INPUT = 1

.if REPORT_INCLUDES == 0x01
.message "included IN v0.2"
.endif

.include	"./core/wait.inc"

;--------------------------------------------------------
C5_INPUT_INIT:
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_Z

	RCALL C5_INPUT_CLEAR

	LDI ZH,high(PORTS_TABLE*2+((C5_IN_PORT>>4)*0x03+0x01))
	LDI ZL,low(PORTS_TABLE*2+((C5_IN_PORT>>4)*0x03+0x01))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ANDI ACCUM,~(EXP2(C5_IN_PORT & 0x0f))
	ST Z,ACCUM

	LDI ZH,high(PORTS_TABLE*2+((C5_IN_PORT>>4)*0x03))
	LDI ZL,low(PORTS_TABLE*2+((C5_IN_PORT>>4)*0x03))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(C5_IN_PORT & 0x0f)
	ST Z,ACCUM

.IF	C5_IN_PORT == INT0_PORT || C5_IN_PORT == INT1_PORT || C5_IN_PORT == INT2_PORT || C5_IN_PORT == INT3_PORT || C5_IN_PORT == INT4_PORT || C5_IN_PORT == INT5_PORT || C5_IN_PORT == INT6_PORT || C5_IN_PORT == INT7_PORT
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,high(_C5_INPUT_INT_HANDLER)
	LDI TEMP_L,low(_C5_INPUT_INT_HANDLER)
.IF	C5_IN_PORT == INT0_PORT
	LDI ACCUM,C5_IR_INT0
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC00)|(1<<ISC01))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF0)
	ORI TEMP_L,(1<<INT0)
.ENDIF
.IF	C5_IN_PORT == INT1_PORT
	LDI ACCUM,C5_IR_INT1
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF1)
	ORI TEMP_L,(1<<INT1)
.ENDIF
.IF	C5_IN_PORT == INT2_PORT
	LDI ACCUM,C5_IR_INT2
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF2)
	ORI TEMP_L,(1<<INT2)
.ENDIF
.IF	C5_IN_PORT == INT3_PORT
	LDI ACCUM,C5_IR_INT3
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF3)
	ORI TEMP_L,(1<<INT3)
.ENDIF
.IF	C5_IN_PORT == INT4_PORT
	LDI ACCUM,C5_IR_INT4
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF4)
	ORI TEMP_L,(1<<INT4)
.ENDIF
.IF	C5_IN_PORT == INT5_PORT
	LDI ACCUM,C5_IR_INT5
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF5)
	ORI TEMP_L,(1<<INT5)
.ENDIF
.IF	C5_IN_PORT == INT6_PORT
	LDI ACCUM,C5_IR_INT6
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF6)
	ORI TEMP_L,(1<<INT6)
.ENDIF
.IF	C5_IN_PORT == INT7_PORT
	LDI ACCUM,C5_IR_INT7
	MCALL C5_IR_VECTOR_SET
	LDS TEMP,EICRA
	ANDI TEMP,~((1<<ISC10)|(1<<ISC11))
	ORI TEMP,C5_ISC_FALLING_EDGE
	STS EICRA,TEMP
	LDS TEMP_H,EIFR
	LDS TEMP_L,EIMSK
	ORI TEMP_H,(1<<INTF7)
	ORI TEMP_L,(1<<INT7)
.ENDIF
	STS EIFR,TEMP_H
	STS EIMSK,TEMP_L
	POP TEMP_L
	POP TEMP_H
	POP TEMP
.ENDIF
	POP_Z
	POP ACCUM
	RET

;--------------------------------------------------------
C5_INPUT_GET:
;--------------------------------------------------------
;Извлекаем символ из буфера
;OUT: TEMP-символ
;--------------------------------------------------------
	MCALL _C5_INPUT_SHIFT
.IF	C5_IN_PORT != INT0_PORT && C5_IN_PORT != INT1_PORT && C5_IN_PORT != INT2_PORT && C5_IN_PORT != INT3_PORT && C5_IN_PORT != INT4_PORT && C5_IN_PORT != INT5_PORT && C5_IN_PORT != INT6_PORT && C5_IN_PORT != INT7_PORT
	CPI TEMP,0x00
	BRNE _C5_INPUT_GET__END
	MCALL _C5_INPUT_READ_BYTES
	MCALL _C5_INPUT_SHIFT
_C5_INPUT_GET__END:
.ENDIF
	RET

;--------------------------------------------------------
C5_INPUT_WAIT:
;--------------------------------------------------------
;Ожидаем и извлекаем символ из буфера
;OUT: TEMP-символ
;--------------------------------------------------------
_C5_INPUT_WAIT__LOOP:
	MCALL _C5_INPUT_SHIFT
	CPI TEMP,0x00
	BRNE _C5_INPUT_WAIT_DONE
.IF	C5_IN_PORT == INT0_PORT || C5_IN_PORT == INT1_PORT || C5_IN_PORT == INT2_PORT || C5_IN_PORT == INT3_PORT || C5_IN_PORT == INT4_PORT || C5_IN_PORT == INT5_PORT || C5_IN_PORT == INT6_PORT || C5_IN_PORT == INT7_PORT
	MCALL C5_WAIT
.ELSE
	MCALL _C5_INPUT_READ_BYTES
.ENDIF
	RJMP _C5_INPUT_WAIT__LOOP
_C5_INPUT_WAIT_DONE:
	RET

;--------------------------------------------------------
C5_INPUT_CLEAR:
;--------------------------------------------------------
;Очищаем буфер
;--------------------------------------------------------
	PUSH TEMP

	LDI TEMP,0x00
	STS _C5_INPUT_BUFFER_POS,TEMP

	POP TEMP
	RET

;--------------------------------------------------------
_C5_INPUT_SHIFT:
;--------------------------------------------------------
;Сдвигаем данные в буфере
;OUT: TEMP-первый символ
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDS ACCUM,SREG
	PUSH ACCUM
	CLI

	LDI TEMP,0x00
	LDS LOOP_CNTR,_C5_INPUT_BUFFER_POS
	CPI LOOP_CNTR,0x00
	BREQ _C5_INPUT_SHIFT__END
	MOV ACCUM,LOOP_CNTR
	DEC ACCUM
	STS _C5_INPUT_BUFFER_POS,ACCUM
	LDI_Z _C5_INPUT_BUFFER
	LD TEMP,Z
_C5_INPUT_SHIFT__LOOP:
	LDD ACCUM,Z+0x01
	ST Z+,ACCUM
	DEC LOOP_CNTR
	BRNE _C5_INPUT_SHIFT__LOOP

_C5_INPUT_SHIFT__END:
	POP ACCUM
	STS SREG,ACCUM
	POP LOOP_CNTR
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_C5_INPUT_READ_BYTES:
;--------------------------------------------------------
;Ожидаем бит START UART'а
;--------------------------------------------------------
.IF	C5_IN_PORT != INT0_PORT && C5_IN_PORT != INT1_PORT && C5_IN_PORT != INT2_PORT && C5_IN_PORT != INT3_PORT && C5_IN_PORT != INT4_PORT && C5_IN_PORT != INT5_PORT && C5_IN_PORT != INT6_PORT && C5_IN_PORT != INT7_PORT
	PUSH TEMP
	PUSH ACCUM
	PUSH_Z
	PUSH_Y

	LDI ZH,high(PORTS_TABLE*2+(((C5_IN_PORT>>4)*0x03)+0x02))
	LDI ZL,low(PORTS_TABLE*2+(((C5_IN_PORT>>4)*0x03)+0x02))
	LPM ZL,Z
	CLR ZH

	CLI
	LDI_Y 0xffff
_C5_INPUT_READ_BYTES__LOOP:
	SBIW YL,0x01
	BREQ _C5_INPUT_READ_BYTES__DONE
	LD TEMP,Z
	ANDI TEMP,EXP2 (C5_IN_PORT & 0x0f)
	BRNE _C5_INPUT_READ_BYTES__LOOP

	LDI_Y 0x0020

	LDI ACCUM,0x11
	DEC ACCUM
	BRNE PC-0x01

	PUSH_Z
	MCALL _C5_INPUT_READ_PACKET
	POP_Z

	CPI TEMP,0x00
	BREQ _C5_INPUT_READ_BYTES__DONE
	LDS ACCUM,_C5_INPUT_BUFFER+INPUT_BUFFER_SIZE-0x01
	CPI ACCUM,0x00
	BREQ _C5_INPUT_READ_BYTES__LOOP
_C5_INPUT_READ_BYTES__DONE:

	POP_Y
	POP_Z
	POP ACCUM
	POP TEMP
	RET
.ENDIF
	RET
;--------------------------------------------------------
_C5_INPUT_READ_PACKET:
;--------------------------------------------------------
;Читаем биты данных и проверям STOP бит UART'а
;OUT:TEMP последний считанный символ
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM
	PUSH TRY_CNTR

	LDS TRY_CNTR,_C5_INPUT_BUFFER_POS
_C5_INPUT_READ_PACKET__BYTES_LOOP:
	;DATA BITS
	LDI LOOP_CNTR,0x08
_C5_INPUT_READ_PACKET__BITES_LOOP:
	LD ACCUM,Z
	LSR TEMP
	NOP
	NOP
	NOP
	NOP
	ANDI ACCUM,EXP2 (C5_IN_PORT & 0x0f)
	BREQ PC+0x02
	ORI TEMP,0x80
	;WAIT
	LDI ACCUM,0x13
	DEC ACCUM
	BRNE PC-0x01

	DEC LOOP_CNTR
	BRNE _C5_INPUT_READ_PACKET__BITES_LOOP
	;STOP BIT
	LD ACCUM,Z
	ANDI ACCUM,EXP2 (C5_IN_PORT & 0x0f)
	BREQ _C5_INPUT_READ_PACKET__DONE

	CPI TRY_CNTR,INPUT_BUFFER_SIZE
	BREQ _C5_INPUT_READ_PACKET__ERROR
	LDI_Z _C5_INPUT_BUFFER
	ADD ZL,TRY_CNTR
	CLR ACCUM
	ADC ZH,ACCUM
	ST Z,TEMP
	INC TRY_CNTR

	LDI ZH,high(PORTS_TABLE*2+(((C5_IN_PORT>>4)*0x03)+0x02))
	LDI ZL,low(PORTS_TABLE*2+(((C5_IN_PORT>>4)*0x03)+0x02))
	LPM ZL,Z
	CLR ZH

	LDI LOOP_CNTR,0x0a
_C5_INPUT_READ_PACKET__NEXT_START_WAIT:
	LD ACCUM,Z
	ANDI ACCUM,EXP2 (C5_IN_PORT & 0x0f)
	BREQ _C5_INPUT_READ_PACKET__GOT_NEXT_START
	DEC LOOP_CNTR
	BRNE _C5_INPUT_READ_PACKET__NEXT_START_WAIT
	RJMP _C5_INPUT_READ_PACKET__DONE
_C5_INPUT_READ_PACKET__GOT_NEXT_START:
	;WAIT
	LDI ACCUM,0x16;0x13
	DEC ACCUM
	BRNE PC-0x01
	RJMP _C5_INPUT_READ_PACKET__BYTES_LOOP
_C5_INPUT_READ_PACKET__ERROR:
	LDI TEMP,0x00
_C5_INPUT_READ_PACKET__DONE:
	STS _C5_INPUT_BUFFER_POS,TRY_CNTR
	POP TRY_CNTR
	POP ACCUM
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_C5_INPUT_INT_HANDLER:
;--------------------------------------------------------
;Обработчик внешнего прерывания
;--------------------------------------------------------

	PUSH TEMP
	LDS TEMP,SREG
	CLI

	PUSH_Z
	PUSH TEMP
	LDI_Z PORTS_TABLE*2+(((C5_IN_PORT>>4)*0x03)+0x02)
	LPM ZL,Z
	CLR ZH

	MCALL _C5_INPUT_READ_PACKET

	LDS TEMP,EIFR
.IF	C5_IN_PORT == INT0_PORT
	ORI TEMP,(1<<INTF0)
.ENDIF
.IF	C5_IN_PORT == INT1_PORT
	ORI TEMP,(1<<INTF1)
.ENDIF
.IF	C5_IN_PORT == INT2_PORT
	ORI TEMP,(1<<INTF2)
.ENDIF
.IF	C5_IN_PORT == INT3_PORT
	ORI TEMP,(1<<INTF3)
.ENDIF
.IF	C5_IN_PORT == INT4_PORT
	ORI TEMP,(1<<INTF4)
.ENDIF
.IF	C5_IN_PORT == INT5_PORT
	ORI TEMP,(1<<INTF5)
.ENDIF
.IF	C5_IN_PORT == INT6_PORT
	ORI TEMP,(1<<INTF6)
.ENDIF
.IF	C5_IN_PORT == INT7_PORT
	ORI TEMP,(1<<INTF7)
.ENDIF
	STS EIFR,TEMP

	POP TEMP
	POP_Z

	STS SREG,TEMP
	POP TEMP
	RET
.ENDIF
.ENDIF
