;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.01.2021  w5277c@gmail.com			Начало
;09.02.2021  w5277c@gmail.com			Поддержка фонового режима (буфер в 1 байт)
;08.04.2021  w5277c@gmail.com			Новая реализация, использую буфер ввода, не оттестировано
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_IN
.else
.set DEF_IN = 1

.if REPORT_INCLUDES == 0x01
.message "included IN"
.endif

.include	"./core/wait.inc"

;Ожидает символ с порта по методу UART.
;BAUDRATE идентичен логированию.
;Если порт поддержтвает внешние прерывания(INT0,...),
;то при ожидании старта прерывания будут разрешены,
;диспетчер также не блокируется. Т.е. многопоточность
;не нарушается.
;Во время приема октета все прерывания запрещаются.
;Полученный символ записывается в буфер ввода
;Если порт не поддерживает внешние прерывания,
;то вся деятельность ядра будет полностью остановлена
;пока не будет получен символ.
;Для корректной работы все остальные обработчики
;прерываний должны разрешать внешние прерывания,
;что бы попасть в данный обработчик максимально быстро.

;--------------------------------------------------------
C5_IN_INIT:
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_Z

	RCALL C5_IN_CLEAR

	LDI ZH,high(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03+0x01))
	LDI ZL,low(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03+0x01))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ANDI ACCUM,~(EXP2(INPUT_PORT & 0x0f))
	ST Z,ACCUM

	LDI ZH,high(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03))
	LDI ZL,low(PORTS_TABLE*2+((INPUT_PORT>>4)*0x03))
	LPM ZL,Z
	CLR ZH
	LD ACCUM,Z
	ORI ACCUM,EXP2(INPUT_PORT & 0x0f)
	ST Z,ACCUM

.IF	INPUT_PORT == INT0_PORT || INPUT_PORT == INT1_PORT || INPUT_PORT == INT2_PORT || INPUT_PORT == INT3_PORT || INPUT_PORT == INT4_PORT || INPUT_PORT == INT5_PORT || INPUT_PORT == INT6_PORT || INPUT_PORT == INT7_PORT
	PUSH TEMP
	PUSH TEMP_H
	PUSH TEMP_L
	LDI TEMP_H,high(_IN_INT_HANDLER)
	LDI TEMP_L,low(_IN_INT_HANDLER)
.IF	INPUT_PORT == INT0_PORT
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
.IF	INPUT_PORT == INT1_PORT
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
.IF	INPUT_PORT == INT2_PORT
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
.IF	INPUT_PORT == INT3_PORT
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
.IF	INPUT_PORT == INT4_PORT
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
.IF	INPUT_PORT == INT5_PORT
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
.IF	INPUT_PORT == INT6_PORT
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
.IF	INPUT_PORT == INT7_PORT
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
C5_IN_GET:
;--------------------------------------------------------
;Извлекаем символ из буфера
;OUT: TEMP-символ
;--------------------------------------------------------
.IFDEF INPUT_PORT
	MCALL _C5_IN_SHIFT
.IF	INPUT_PORT != INT0_PORT && INPUT_PORT != INT1_PORT && INPUT_PORT != INT2_PORT && INPUT_PORT != INT3_PORT && INPUT_PORT != INT4_PORT && INPUT_PORT != INT5_PORT && INPUT_PORT != INT6_PORT && INPUT_PORT != INT7_PORT
	CPI TEMP,0x00
	BRNE _C5_IN_GET__END
	MCALL _IN_READ_BYTES
	MCALL _C5_IN_SHIFT
.ENDIF
.ELSE
	LDI TEMP,0x00
.ENDIF
_C5_IN_GET__END:
	RET

;--------------------------------------------------------
C5_IN_WAIT:
;--------------------------------------------------------
;Ожидаем и извлекаем символ из буфера
;OUT: TEMP-символ
;--------------------------------------------------------
.IFDEF INPUT_PORT
_C5_IN_WAIT__LOOP:
	MCALL _C5_IN_SHIFT
	CPI TEMP,0x00
	BRNE _C5_IN_WAIT_DONE
.IF	INPUT_PORT == INT0_PORT || INPUT_PORT == INT1_PORT || INPUT_PORT == INT2_PORT || INPUT_PORT == INT3_PORT || INPUT_PORT == INT4_PORT || INPUT_PORT == INT5_PORT || INPUT_PORT == INT6_PORT || INPUT_PORT == INT7_PORT
	MCALL C5_WAIT
.ELSE
	MCALL _IN_READ_BYTES
.ENDIF
	RJMP _C5_IN_WAIT__LOOP
_C5_IN_WAIT_DONE:
	RET
.ELSE
	LDI TEMP,0x00
.ENDIF
	RET

;--------------------------------------------------------
C5_IN_CLEAR:
;--------------------------------------------------------
;Очищаем буфер
;--------------------------------------------------------
	PUSH_Z
	PUSH TEMP
	LDI TEMP,0x00
	LDI_Z _C5_INPUT_BUFFER
.IF INPUT_BUFFER_SIZE > 0x01
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,INPUT_BUFFER_SIZE
	PUSH ACCUM
	LDS ACCUM,SREG
	CLI
.ENDIF
_C5_IN_CLEAR__LOOP:
	ST Z+,TEMP
.IF INPUT_BUFFER_SIZE > 0x02
	DEC LOOP_CNTR
	BRNE _C5_IN_CLEAR__LOOP
	POP ACCUM
	STS SREG,ACCUM
	POP LOOP_CNTR
.ENDIF
	POP TEMP
	POP_Z
	RET

;--------------------------------------------------------
_C5_IN_SHIFT:
;--------------------------------------------------------
;Сдвигаем данные в буфере
;OUT: TEMP-первый символ
;--------------------------------------------------------
	PUSH_Z
	PUSH ACCUM
	LDI_Z _C5_INPUT_BUFFER
	LD TEMP,Z
	CPI TEMP,0x00
	BREQ _C5_IN_SHIFT__END
.IF INPUT_BUFFER_SIZE > 0x01
.IF INPUT_BUFFER_SIZE > 0x02
	PUSH LOOP_CNTR
	LDI LOOP_CNTR,INPUT_BUFFER_SIZE-0x01
.ENDIF
	LDS ACCUM,SREG
	PUSH ACCUM
	CLI
_C5_IN_SHIFT__LOOP:
	LDD ACCUM,Z+0x01
	ST Z+,ACCUM
.IF INPUT_BUFFER_SIZE > 0x02
	DEC LOOP_CNTR
	BRNE _C5_IN_SHIFT__LOOP
	POP LOOP_CNTR
.ENDIF
	POP ACCUM
	STS SREG,ACCUM
.ENDIF
	LDI ACCUM,0x00
	ST Z,ACCUM
_C5_IN_SHIFT__END:
	POP ACCUM
	POP_Z
	RET

;--------------------------------------------------------
_IN_READ_BYTES:
;--------------------------------------------------------
;Ожидаем бит START UART'а
;--------------------------------------------------------
.IFDEF INPUT_PORT
.IF	INPUT_PORT != INT0_PORT && INPUT_PORT != INT1_PORT && INPUT_PORT != INT2_PORT && INPUT_PORT != INT3_PORT && INPUT_PORT != INT4_PORT && INPUT_PORT != INT5_PORT && INPUT_PORT != INT6_PORT && INPUT_PORT != INT7_PORT
	PUSH TEMP
	PUSH ACCUM
	PUSH_Z
	PUSH_Y

	LDI ZH,high(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LDI ZL,low(PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02))
	LPM ZL,Z
	CLR ZH

	CLI
	LDI_Y 0xffff
_IN_READ_BYTES__LOOP:
	SBIW YL,0x01
	BREQ _IN_READ_BYTES__DONE
	LD TEMP,Z
	ANDI TEMP,EXP2 (INPUT_PORT & 0x0f)
	BRNE _IN_READ_BYTES__LOOP

	LDI_Y 0x0080

	LDI ACCUM,0x10;0x12
	DEC ACCUM
	BRNE PC-0x01

	MCALL _IN_READ_BYTE

	CPI TEMP,0x00
	BREQ _IN_READ_BYTES__DONE
	LDS ACCUM,_C5_INPUT_BUFFER+INPUT_BUFFER_SIZE-0x01
	CPI ACCUM,0x00
	BREQ _IN_READ_BYTES_LOOP
_IN_SOFT_START_DETECT__DONE:

	POP_Y
	POP_Z
	POP ACCUM
	POP TEMP
	RET

.ENDIF
.ENDIF

;--------------------------------------------------------
_IN_READ_BYTE:
;--------------------------------------------------------
;Читаем биты данных и проверям STOP бит UART'а
;OUT:TEMP последний считанный символ
;--------------------------------------------------------
	PUSH LOOP_CNTR
	PUSH ACCUM

	;DATA BITS
	LDI LOOP_CNTR,0x08
_IN_READ_BYTE__LOOP:
	LSR TEMP
	LD ACCUM,Z
	NOP
	NOP
	NOP
	NOP
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
	BREQ PC+0x02
	ORI TEMP,0x80
	;WAIT
	LDI ACCUM,0x13;0x08
	DEC ACCUM
	BRNE PC-0x01
	DEC LOOP_CNTR
	BRNE _IN_READ_BYTE__LOOP
	;STOP BIT
	LD ACCUM,Z
	ANDI ACCUM,EXP2 (INPUT_PORT & 0x0f)
	BREQ _IN_READ_BYTE__ERROR

	LDI_Z _C5_INPUT_BUFFER
	LDI LOOP_CNTR,INPUT_BUFFER_SIZE
_IN_READ_BYTE__LOOP2:
	LD ACCUM,Z+
	CPI ACCUM,0x00
	BREQ _IN_READ_BYTE__GOT_ZERO
	DEC LOOP_CNTR
	BRNE _IN_READ_BYTE__LOOP2
	RJMP _IN_READ_BYTE__ERROR
_IN_READ_BYTE__GOT_ZERO:
	SBIW ZL,0x01
	ST Z,TEMP
	RJMP _IN_READ_BYTE__DONE
_IN_READ_BYTE__ERROR:
	LDI TEMP,0x00
_IN_READ_BYTE__DONE:
	POP ACCUM
	POP LOOP_CNTR
	RET

;--------------------------------------------------------
_IN_INT_HANDLER:
;--------------------------------------------------------
;Обработчик внешнего прерывания
;--------------------------------------------------------
	PUSH TEMP
	LDS TEMP,SREG
	CLI

	PUSH_Z
	PUSH TEMP
	LDI_Z PORTS_TABLE*2+(((INPUT_PORT>>4)*0x03)+0x02)
	LPM ZL,Z
	CLR ZH
	MCALL _IN_READ_BYTE


.IF	INPUT_PORT == INT0_PORT
	LDS TEMP,EIFR
	ORI TEMP,(1<<INTF0)
	STS EIFR,TEMP
.ENDIF


	POP TEMP
	POP_Z

	STS SREG,TEMP
	POP TEMP
	RET
.ENDIF
