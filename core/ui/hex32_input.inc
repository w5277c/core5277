;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.04.2021  w5277c@gmail.com			Начало
;08.04.2021  w5277c@gmail.com			Добавлена поддержка кнопок курсора Esc[C, Esc[D
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_HEX32_INPUT
.else
.set DEF_C5_HEX32_INPUT = 1

.include "./core/io/out_char.inc"
.include "./core/io/out_cr.inc"
.include "./core/wait.inc"
.include "./conv/bitnum_to_num.inc"

;TODO необходима оптимизация кода

;--------------------------------------------------------
C5_HEX32_INPUT:
;--------------------------------------------------------
;Считывет с UART ASCII HEX символы
;IN/OUT: TEMP_EH/EL/H/L-число
;--------------------------------------------------------
	PUSH TEMP
	PUSH FLAGS
	PUSH TRY_CNTR
	PUSH LOOP_CNTR
	PUSH_Y

	;Флаги для мигания
	LDI TRY_CNTR,0x00
	;Текущая цифра(начиная с старшей)
	LDI LOOP_CNTR,0x00
	;Признак ESC последовательности
;	LDI FLAGS,0x00
	;Счтечтик паузы для перерисовки
	LDI_Y 0x02

C5_HEX32_INPUT__MAIN_LOOP:
	SBIW YL,0x01
	BRNE PC+0x04
	LDI_Y 0x02
	RCALL _C5_HEX32_INPUT_DRAW

	MCALL C5_INPUT_GET
	CPI TEMP,0x00
	BREQ C5_HEX32_INPUT__KEY_CHECKING_END
	CPI TEMP,0x1b
	BRNE C5_HEX32_INPUT__KEY_CHECKING_NOT_ESC
	MCALL C5_INPUT_GET
	CPI TEMP,'['
	BRNE C5_HEX32_INPUT__KEY_CHECKING_END
	MCALL C5_INPUT_GET
	CPI TEMP,'C'
	BRNE C5_HEX32_INPUT__KEY_CHECKING_ESC_NOT_RIGHT
	CPI LOOP_CNTR,0x07
	BREQ PC+0x02
	INC LOOP_CNTR
	RJMP C5_HEX32_INPUT__MAIN_LOOP
C5_HEX32_INPUT__KEY_CHECKING_ESC_NOT_RIGHT:
	CPI TEMP,'D'
	BRNE C5_HEX32_INPUT__KEY_CHECKING_END
	CPI LOOP_CNTR,0x00
	BREQ PC+0x02
	DEC LOOP_CNTR
	RJMP C5_HEX32_INPUT__MAIN_LOOP

C5_HEX32_INPUT__KEY_CHECKING_NOT_ESC:
	CPI TEMP,0x0d
	BRNE C5_HEX32_INPUT__KEY_CHECKING_NOT_ENTER
	CLR TRY_CNTR
	MCALL _C5_HEX32_INPUT_DRAW
	RJMP C5_HEX32_INPUT__DONE
C5_HEX32_INPUT__KEY_CHECKING_NOT_ENTER:
	CPI TEMP,0x08
	BRNE C5_HEX32_INPUT__KEY_CHECKING_NOT_BACKSPACE
	CPI LOOP_CNTR,0x00
	BREQ C5_HEX32_INPUT__KEY_CHECKING_END
	DEC LOOP_CNTR
	RJMP C5_HEX32_INPUT__KEY_CHECKING_END
C5_HEX32_INPUT__KEY_CHECKING_NOT_BACKSPACE:
	CPI TEMP,0x30
	BRCS C5_HEX32_INPUT__KEY_CHECKING_END
	CPI TEMP,0x40
	BRCC C5_HEX32_INPUT__KEY_CHECKING_NOT_DIG
	SUBI TEMP,0x30
	MCALL _C5_HEX32_INPUT_SET_NUM
	RJMP C5_HEX32_INPUT__KEY_CHECKING_END
C5_HEX32_INPUT__KEY_CHECKING_NOT_DIG:
	CPI TEMP,0x41
	BRCS C5_HEX32_INPUT__KEY_CHECKING_END
	ANDI TEMP,0xdf
	CPI TEMP,0x47
	BRCC C5_HEX32_INPUT__KEY_CHECKING_END
	SUBI TEMP,0x37
	MCALL _C5_HEX32_INPUT_SET_NUM
C5_HEX32_INPUT__KEY_CHECKING_END:
	MCALL C5_WAIT
	RJMP C5_HEX32_INPUT__MAIN_LOOP

C5_HEX32_INPUT__DONE:
	MCALL C5_OUT_CR

	POP_Y
	POP LOOP_CNTR
	POP TRY_CNTR
	POP FLAGS
	POP TEMP
	RET


_C5_HEX32_INPUT_SET_NUM:
	CPI LOOP_CNTR,0x00
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT8
	SWAP TEMP
	ANDI TEMP_EH,0x0f
	OR TEMP_EH,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT8:
	CPI LOOP_CNTR,0x01
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT7
	ANDI TEMP_EH,0xf0
	OR TEMP_EH,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT7:
	CPI LOOP_CNTR,0x02
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT6
	SWAP TEMP
	ANDI TEMP_EL,0x0f
	OR TEMP_EL,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT6:
	CPI LOOP_CNTR,0x03
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT5
	ANDI TEMP_EL,0xf0
	OR TEMP_EL,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT5:
	CPI LOOP_CNTR,0x04
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT4
	SWAP TEMP
	ANDI TEMP_H,0x0f
	OR TEMP_H,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT4:
	CPI LOOP_CNTR,0x05
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT3
	ANDI TEMP_H,0xf0
	OR TEMP_H,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT3:
	CPI LOOP_CNTR,0x06
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT2
	SWAP TEMP
	ANDI TEMP_L,0x0f
	OR TEMP_L,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT2:
	CPI LOOP_CNTR,0x07
	BRNE _C5_HEX32_INPUT_SET__NUM_NOT1
	ANDI TEMP_L,0xf0
	OR TEMP_L,TEMP
	RJMP _C5_HEX32_INPUT_SET__END
_C5_HEX32_INPUT_SET__NUM_NOT1:
_C5_HEX32_INPUT_SET__END:

	CPI LOOP_CNTR,0x07
	BREQ PC+0x02
	INC LOOP_CNTR

	RET

_C5_HEX32_INPUT_DRAW:
	LDI TEMP,0x0d
	MCALL C5_OUT_CHAR
	LDI TEMP,'0'
	MCALL C5_OUT_CHAR
	LDI TEMP,'x'
	MCALL C5_OUT_CHAR

	PUSH TEMP_L
	PUSH TEMP_H
	MOV TEMP,TEMP_EH
	MCALL BYTE_TO_HEX
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x00
	MOV TEMP,TEMP_H
	MCALL C5_OUT_CHAR
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x01
	MOV TEMP,TEMP_L
	MCALL C5_OUT_CHAR
	MOV TEMP,TEMP_EL
	MCALL BYTE_TO_HEX
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x02
	MOV TEMP,TEMP_H
	MCALL C5_OUT_CHAR
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x03
	MOV TEMP,TEMP_L
	MCALL C5_OUT_CHAR
	POP TEMP_H
	PUSH TEMP_H
	MOV TEMP,TEMP_H
	MCALL BYTE_TO_HEX
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x04
	MOV TEMP,TEMP_H
	MCALL C5_OUT_CHAR
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x05
	MOV TEMP,TEMP_L
	MCALL C5_OUT_CHAR
	POP TEMP_H
	POP TEMP_L
	PUSH TEMP_L
	PUSH TEMP_H
	MOV TEMP,TEMP_L
	MCALL BYTE_TO_HEX
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x06
	MOV TEMP,TEMP_H
	MCALL C5_OUT_CHAR
	LDI TEMP,'_'
	SBRS TRY_CNTR,0x07
	MOV TEMP,TEMP_L
	MCALL C5_OUT_CHAR
	POP TEMP_H
	POP TEMP_L

	MOV TEMP,LOOP_CNTR
	MCALL BITNUM_TO_NUM
	COM TRY_CNTR
	AND TRY_CNTR,TEMP

	RET
.endif
