;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.10.2020  w5277c@gmail.com			Начало
;27.10.2020  w5277c@gmail.com			Обновлена информация об авторских правах
;21.01.2020  w5277c@gmail.com			Вынесен блок определений пинов для процедур PORT_...
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	ATTINY25									= 0x00
	.EQU	ATMEGA8									= 0x01
	.EQU	ATMEGA16									= 0x02
	.EQU	ATMEGA48									= 0x03

	.SET	AVRA										= 0x00
.ifdef CORE_FREQ
.else
	.SET	CORE_FREQ								= 0x10
.endif
	.EQU	TIMERS_SPEED_50NS						= (3125*CORE_FREQ*2)/1000	;2-20МГц(реком. 4,8,12,16,20)
	.EQU	TIMERS_SPEED_25NS						= (3125*CORE_FREQ)/1000		;2-20МГц(реком. 8,16)
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS

;---PROC-ID-OPTIONS--------------------------------------;Не изменяемые опции процесса размещаемые в идентификаторе процесса
	.EQU	C5_PROCID_OPT_NOSUSP					= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	C5_PROCID_OPT_TIMER					= 0x05			;Таймер, повторяет выполнение с указанным периодом
	.EQU	C5_PROCID_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	C5_PROCID_OPT_DRV						= 0x07			;Драйвер

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

	.EQU	C5_ISC_LOW_LEVEL						= 0x00
	.EQU	C5_ISC_ANY_CHANGE						= 0x01
	.EQU	C5_ISC_FALLING_EDGE					= 0x02
	.EQU	C5_ISC_RISING_EDGE					= 0x03

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

.MACRO C5_WAIT_500NS
	PUSH TEMP
	LDI TEMP,low((@0-36)/(16/CORE_FREQ))
	.if ((@0-36)/(16/CORE_FREQ)) > 0
		MCALL _C5_WAIT_500NS
	.endif
	POP TEMP
.ENDMACRO

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
