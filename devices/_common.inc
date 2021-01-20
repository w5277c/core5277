;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.10.2020  w5277c@gmail.com        Начало
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
	.EQU	ATTINY25									= 0x00
	.EQU	ATMEGA8									= 0x01
	.EQU	ATMEGA16									= 0x02
	.EQU	ATMEGA48									= 0x03

	.SET	CORE_FREQ								= 0x10
	.EQU	TIMERS_SPEED_50NS						= (3125*CORE_FREQ*2)/1000	;2-20МГц(реком. 4,8,12,16,20)
	.EQU	TIMERS_SPEED_25NS						= (3125*CORE_FREQ)/1000		;2-20МГц(реком. 8,16)
	.SET	TIMERS_SPEED							= TIMERS_SPEED_50NS

;---PROC-ID-OPTIONS--------------------------------------;Не изменяемые опции процесса размещаемые в идентификаторе процесса
	.EQU	C5_PROCID_OPT_NOSUSP					= 0x04			;Запрещено диспетчеру прерывать выполнение
	.EQU	C5_PROCID_OPT_TIMER					= 0x05			;Таймер, повторяет выполнение с указанным периодом
	.EQU	C5_PROCID_OPT_RESERV2				= 0x06			;Зарезервировано
	.EQU	C5_PROCID_OPT_DRV						= 0x07			;Драйвер

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
