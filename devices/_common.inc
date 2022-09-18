;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.10.2020	w5277c@gmail.com			Начало
;27.10.2020	w5277c@gmail.com			Обновлена информация об авторских правах
;21.01.2020	w5277c@gmail.com			Вынесен блок определений пинов для процедур PORT_...
;29.05.2021	w5277c@gmail.com			Ввод параметров TS_MODE и MUL_SUPPORT
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_DEVICES_COMMON
.else
.set DEF_DEVICES_COMMON = 1

	.EQU	ATTINY25									= 0x00
	.EQU	ATMEGA8									= 0x01
	.EQU	ATMEGA16									= 0x02
	.EQU	ATMEGA48									= 0x03
	.EQU	ATTINY13A								= 0x04

;---TASK-SWITCHING-MODES---------------------------------;Режимы работы ядра
	.EQU	TS_MODE_NO								= 0xff			;Простое переключение, механизмы ядра для переключения задач отсутствуют
	.EQU	TS_MODE_EVENT							= 0x00			;Кооперативная
	.EQU	TS_MODE_TIME							= 0x01			;Вытесняющая многозадачность
;---LOGGING----------------------------------------------;Параметры логирования
	.EQU	LOGGING_LVL_DIS						= 0x00
	.EQU	LOGGING_LVL_ERR						= 0x01
	.EQU	LOGGING_LVL_WRN						= 0x02
	.EQU	LOGGING_LVL_INF						= 0x03
	.EQU	LOGGING_LVL_DBG						= 0x04
	.EQU	LOGGING_LVL_PNC						= 0x05
	.SET	LOGGING_LEVEL							= LOGGING_LVL_INF

	.SET	BUFFER_SIZE								= 0x0000

.ifdef REPORT_INCLUDES
.else
	.SET	REPORT_INCLUDES						= 0x00
.endif
.ifdef TIMER_C_ENABLE
.else
	.SET	TIMER_C_ENABLE							= 0x00
.endif
	.SET	REALTIME									= 0x00
	.SET	TIMERS									= 0x00
	.SET	TIMERS16									= 0x00
.ifdef AVRA
.else
	.SET	AVRA										= 0x00
.endif
	.SET	MUL_SUPPORT								= 0x01

.ifdef CORE_FREQ
	.EQU	TIMERS_SPEED_50US						= (3125*CORE_FREQ*2)/1000	;2-20МГц(реком. 4,8,12,16,20)
	.EQU	TIMERS_SPEED_25US						= (3125*CORE_FREQ)/1000		;2-20МГц(реком. 8,16)
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50US
	.SET	INPUT_BUFFER_SIZE						= 0x80			;Размер буфера ввода (не менее 2)
;---PROC-ID-OPTIONS--------------------------------------;Не изменяемые опции процесса размещаемые в идентификаторе процесса
	.EQU	C5_PROCID_OPT_NOSUSP					= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	C5_PROCID_OPT_TIMER					= 0x05			;Таймер, повторяет выполнение с указанным периодом
	.EQU	C5_PROCID_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	C5_PROCID_OPT_DRV						= 0x07			;Драйвер
.endif

	.EQU	PA0										= (0x00<<4)|0x00
	.EQU	PA1										= (0x00<<4)|0x01
	.EQU	PA2										= (0x00<<4)|0x02
	.EQU	PA3										= (0x00<<4)|0x03
	.EQU	PA4										= (0x00<<4)|0x04
	.EQU	PA5										= (0x00<<4)|0x05
	.EQU	PA6										= (0x00<<4)|0x06
	.EQU	PA7										= (0x00<<4)|0x07
	.EQU	PB0										= (0x01<<4)|0x00
	.EQU	PB1										= (0x01<<4)|0x01
	.EQU	PB2										= (0x01<<4)|0x02
	.EQU	PB3										= (0x01<<4)|0x03
	.EQU	PB4										= (0x01<<4)|0x04
	.EQU	PB5										= (0x01<<4)|0x05
	.EQU	PB6										= (0x01<<4)|0x06
	.EQU	PB7										= (0x01<<4)|0x07
	.EQU	PC0										= (0x02<<4)|0x00
	.EQU	PC1										= (0x02<<4)|0x01
	.EQU	PC2										= (0x02<<4)|0x02
	.EQU	PC3										= (0x02<<4)|0x03
	.EQU	PC4										= (0x02<<4)|0x04
	.EQU	PC5										= (0x02<<4)|0x05
	.EQU	PC6										= (0x02<<4)|0x06
	.EQU	PC7										= (0x02<<4)|0x07
	.EQU	PD0										= (0x03<<4)|0x00
	.EQU	PD1										= (0x03<<4)|0x01
	.EQU	PD2										= (0x03<<4)|0x02
	.EQU	PD3										= (0x03<<4)|0x03
	.EQU	PD4										= (0x03<<4)|0x04
	.EQU	PD5										= (0x03<<4)|0x05
	.EQU	PD6										= (0x03<<4)|0x06
	.EQU	PD7										= (0x03<<4)|0x07
	.EQU	PE0										= (0x04<<4)|0x00
	.EQU	PE1										= (0x04<<4)|0x01
	.EQU	PE2										= (0x04<<4)|0x02
	.EQU	PE3										= (0x04<<4)|0x03
	.EQU	PE4										= (0x04<<4)|0x04
	.EQU	PE5										= (0x04<<4)|0x05
	.EQU	PE6										= (0x04<<4)|0x06
	.EQU	PE7										= (0x04<<4)|0x07
	.EQU	PF0										= (0x05<<4)|0x00
	.EQU	PF1										= (0x05<<4)|0x01
	.EQU	PF2										= (0x05<<4)|0x02
	.EQU	PF3										= (0x05<<4)|0x03
	.EQU	PF4										= (0x05<<4)|0x04
	.EQU	PF5										= (0x05<<4)|0x05
	.EQU	PF6										= (0x05<<4)|0x06
	.EQU	PF7										= (0x05<<4)|0x07
	.EQU	PG0										= (0x06<<4)|0x00
	.EQU	PG1										= (0x06<<4)|0x01
	.EQU	PG2										= (0x06<<4)|0x02
	.EQU	PG3										= (0x06<<4)|0x03
	.EQU	PG4										= (0x06<<4)|0x04
	.EQU	PG5										= (0x06<<4)|0x05
	.EQU	PG6										= (0x06<<4)|0x06
	.EQU	PG7										= (0x06<<4)|0x07

	.EQU	C5_ISC_LOW_LEVEL						= 0x00
	.EQU	C5_ISC_ANY_CHANGE						= 0x01
	.EQU	C5_ISC_FALLING_EDGE					= 0x02
	.EQU	C5_ISC_RISING_EDGE					= 0x03

	.SET	INT0_PORT								= 0xff
	.SET	INT1_PORT								= 0xff
	.SET	INT2_PORT								= 0xff
	.SET	INT3_PORT								= 0xff
	.SET	INT4_PORT								= 0xff
	.SET	INT5_PORT								= 0xff
	.SET	INT6_PORT								= 0xff
	.SET	INT7_PORT								= 0xff

	.DEF	C0x00										= r7				;Константа 0x00
	.DEF	C0xff										= r8				;Константа 0xff
	.DEF	TEMP_L									= r16				;Младший регистр общего назначения
	.DEF	TEMP_H									= r17				;Старший регистр общего назначения
	.DEF	TEMP										= r18				;Регистр общего назначения
	.DEF	TEMP_EL									= r19				;Младший расширенный регистр общего назначения
	.DEF	TEMP_EH									= r20				;Старший расширенный регистр общего назначения
	.DEF	LOOP_CNTR								= r21				;Регистр счета циклов
	.DEF	FLAGS										= r22				;Регистр флагов
	.DEF	TRY_CNTR									= r23				;Счетчик ошибок
	.DEF	ACCUM										= r24				;Аккумулятор
	.DEF	PID										= r25				;ид процедуры
	.DEF	XL											= r26
	.DEF	XH											= r27
	.DEF	YL											= r28
	.DEF	YH											= r29
	.DEF	ZL											= r30
	.DEF	ZH											= r31

	;---Часто-используемые-константы-для-частот-программного-таймера---
;TODO делить на 2?
.ifdef TIMERS_SPEED
.if TIMERS_SPEED == TIMERS_SPEED_50US							;Необходимо учитывать старший бит(0.002c или 0.000 050с)
	.EQU	TIMER_FREQ_20KHz						= 1
	.EQU	TIMER_FREQ_10KHz						= 2
	.EQU	TIMER_FREQ_5KHz						= 4
	.EQU	TIMER_FREQ_4KHz						= 5
	.EQU	TIMER_FREQ_2KHz						= 10
	.EQU	TIMER_FREQ_1KHz						= 20
	.EQU	TIMER_FREQ_500Hz						= 40
	.EQU	TIMER_FREQ_250Hz						= 80
	.EQU	TIMER_FREQ_100Hz						= 128+5
.else
	.EQU	TIMER_FREQ_40KHz						= 1
	.EQU	TIMER_FREQ_20KHz						= 2
	.EQU	TIMER_FREQ_10KHz						= 4
	.EQU	TIMER_FREQ_5KHz						= 8
	.EQU	TIMER_FREQ_4KHz						= 10
	.EQU	TIMER_FREQ_2KHz						= 20
	.EQU	TIMER_FREQ_1KHz						= 40
	.EQU	TIMER_FREQ_500Hz						= 80
	.EQU	TIMER_FREQ_250Hz						= 128+2
	.EQU	TIMER_FREQ_100Hz						= 128+5
.endif
.endif
.ifdef CORE_FREQ
	;---Часто-используемые-константы-для-частот-таймера-С---
	.EQU	TIMER_C_BAUDRATE_115200				= CORE_FREQ*125000/115200-1
	.EQU	TIMER_C_BAUDRATE_76800				= CORE_FREQ*125000/76800-1
	.EQU	TIMER_C_BAUDRATE_74880 				= CORE_FREQ*125000/74880-1
	.EQU	TIMER_C_BAUDRATE_57600				= CORE_FREQ*125000/57600-1
	.EQU	TIMER_C_BAUDRATE_38400				= CORE_FREQ*125000/38400-1
	.EQU	TIMER_C_BAUDRATE_28800				= CORE_FREQ*125000/28800-1
	.EQU	TIMER_C_BAUDRATE_19200				= CORE_FREQ*125000/19200-1
	.EQU	TIMER_C_BAUDRATE_14400				= CORE_FREQ*125000/14400-1
	.EQU	TIMER_C_BAUDRATE_9600				= CORE_FREQ*125000/9600-1
	;---
	.EQU	TIMER_C_FREQ_40KHz					= (CORE_FREQ*125000/(40000*2))-1
	.EQU	TIMER_C_FREQ_38KHz					= (CORE_FREQ*125000/(38000*2))-1
	.EQU	TIMER_C_FREQ_36KHz					= (CORE_FREQ*125000/(36000*2))-1
	.EQU	TIMER_C_FREQ_20KHz					= (CORE_FREQ*125000/(20000*2))-1
	.EQU	TIMER_C_FREQ_10KHz					= (CORE_FREQ*125000/(10000*2))-1
	.EQU	TIMER_C_FREQ_5KHz						= (CORE_FREQ*125000/(5000*2))-1
.IF CORE_FREQ < 20
	.EQU	TIMER_C_FREQ_4800Hz					= (CORE_FREQ*125000/(4800*2))-1
.ENDIF
	;---
.endif
	.MACRO _C5_MACRO__PUSH_RDS
		PUSH r16
		PUSH r17
		PUSH r18
		PUSH r19
		PUSH r20
		PUSH r21
		PUSH r22
		PUSH r23
		PUSH r24
		PUSH r25
		PUSH r26
		PUSH r27
		PUSH r28
		PUSH r29
		PUSH r30
		PUSH r31
	.ENDMACRO

	.MACRO _C5_MACRO__POP_RDS
		POP r31
		POP r30
		POP r29
		POP r28
		POP r27
		POP r26
		POP r25
		POP r24
		POP r23
		POP r22
		POP r21
		POP r20
		POP r19
		POP r18
		POP r17
		POP r16
	.ENDMACRO

.MACRO PUSH_X
	PUSH XH
	PUSH XL
.ENDMACRO
.MACRO POP_X
	POP XL
	POP XH
.ENDMACRO
.MACRO PUSH_Y
	PUSH YH
	PUSH YL
.ENDMACRO
.MACRO POP_Y
	POP YL
	POP YH
.ENDMACRO
.MACRO PUSH_Z
	PUSH ZH
	PUSH ZL
.ENDMACRO
.MACRO POP_Z
	POP ZL
	POP ZH
.ENDMACRO

.ifdef CORE_FREQ
.MACRO C5_WAIT_500NS
	PUSH TEMP
	LDI TEMP,low((@0-36)/(16/CORE_FREQ))
	.if ((@0-36)/(16/CORE_FREQ)) > 0
		MCALL _C5_WAIT_500NS
	.endif
	POP TEMP
.ENDMACRO
.endif

.MACRO LDI_X
	LDI XH,high(@0)
	LDI XL,low(@0)
.ENDMACRO
.MACRO LDI_Y
	LDI YH,high(@0)
	LDI YL,low(@0)
.ENDMACRO
.MACRO LDI_Z
	LDI ZH,high(@0)
	LDI ZL,low(@0)
.ENDMACRO
.MACRO PUSH_THL
	PUSH TEMP_H
	PUSH TEMP_L
.ENDMACRO
.MACRO POP_THL
	POP TEMP_L
	POP TEMP_H
.ENDMACRO
.MACRO PUSH_TEHL
	PUSH TEMP_EH
	PUSH TEMP_EL
.ENDMACRO
.MACRO POP_TEHL
	POP TEMP_EL
	POP TEMP_EH
.ENDMACRO
.MACRO PUSH_FT
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
.ENDMACRO
.MACRO POP_FT
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
.ENDMACRO

.endif
