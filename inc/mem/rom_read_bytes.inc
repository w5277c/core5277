;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_ROM_READ_BYTES
.else
.set DEF_C5_ROM_READ_BYTES = 1

;--------------------------------------------------------
C5_ROM_READ_BYTES:
;--------------------------------------------------------
;Копирование блока памяти
;IN: Z-SRC ROM адрес, Y-DST RAM адрес,
;LOOP_CNTR-длина(1-255,0=256)
;-START--------------------------------------------------
	PUSH_Y
	PUSH_Z
	PUSH LOOP_CNTR
	PUSH TEMP
_C5_ROM_READ_BYTES__LOOP:
	LPM TEMP,Z+
	ST Y+,TEMP
	DEC LOOP_CNTR
	BRNE _C5_ROM_READ_BYTES__LOOP
	POP TEMP
	POP LOOP_CNTR
	POP_Z
	POP_Y
	RET
.endif