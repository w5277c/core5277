;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2021  w5277c@gmail.com			Начало
;06.02.2021  w5277c@gmail.com			Добавлен макрос загрузки константы
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_PRIM_COMMON
.else
.set DEF_PRIM_COMMON = 1

	;---TYPE-AND-SIZE---
	.EQU	PRIM_BYTE								= 0x11
	.EQU	PRIM_SHORT								= 0x22
	.EQU	PRIM_SDNF								= 0x32	;Не поддерживается
	.EQU	PRIM_INT									= 0x44
	.EQU	PRIM_FLOAT								= 0x54	;Не поддерживается
	.EQU	PRIM_UPTIME								= 0x65
	.EQU	PRIM_LONG								= 0x78	;Не поддерживается
	.EQU	PRIM_DOUBLE								= 0x88	;Не поддерживается

.MACRO PRIM_LDI	;TYPE OFFSET VALUE
	PUSH TEMP
	PUSH ZH
	PUSH ZL
	LDI TEMP,@1
	ADD ZL,TEMP
	CLR TEMP
	ADC ZH,TEMP

	.IF @0 == PRIM_BYTE
		LDI TEMP,(@2 & 0xff)
		STD Z+0x00,TEMP
	.ENDIF
	.IF @0 == PRIM_SHORT
		LDI TEMP,(@2>>0x08) & 0xff
		STD Z+0x00,TEMP
		LDI TEMP,(@2 & 0xff)
		STD Z+0x01,TEMP
	.ENDIF
	.IF @0 == PRIM_INT
		LDI TEMP,(@2>>0x18) & 0xff
		STD Z+0x00,TEMP
		LDI TEMP,(@2>>0x10) & 0xff
		STD Z+0x01,TEMP
		LDI TEMP,(@2>>0x08) & 0xff
		STD Z+0x02,TEMP
		LDI TEMP,(@2 & 0xff)
		STD Z+0x03,TEMP
	.ENDIF
	.IF @0 == PRIM_UPTIME
		LDI TEMP,(@2>>0x20) & 0xff
		STD Z+0x00,TEMP
		LDI TEMP,(@2>>0x18) & 0xff
		STD Z+0x01,TEMP
		LDI TEMP,(@2>>0x10) & 0xff
		STD Z+0x02,TEMP
		LDI TEMP,(@2>>0x08) & 0xff
		STD Z+0x03,TEMP
		LDI TEMP,(@2 & 0xff)
		STD Z+0x04,TEMP
	.ENDIF
	.IF @0 == PRIM_LONG
		LDI TEMP,(@2>>0x38) & 0xff
		STD Z+0x00,TEMP
		LDI TEMP,(@2>>0x30) & 0xff
		STD Z+0x01,TEMP
		LDI TEMP,(@2>>0x28) & 0xff
		STD Z+0x02,TEMP
		LDI TEMP,(@2>>0x20) & 0xff
		STD Z+0x03,TEMP
		LDI TEMP,(@2>>0x18) & 0xff
		STD Z+0x04,TEMP
		LDI TEMP,(@2>>0x10) & 0xff
		STD Z+0x05,TEMP
		LDI TEMP,(@2>>0x08) & 0xff
		STD Z+0x06,TEMP
		LDI TEMP,(@2 & 0xff)
		STD Z+0x07,TEMP
	.ENDIF
	POP ZL
	POP ZH
	POP TEMP
.ENDMACRO
.endif
