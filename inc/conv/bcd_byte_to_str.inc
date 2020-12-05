;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;07.10.2019  w5277c@gmail.com        Начало
;01.08.2020  w5277c@gmail.com        Разбиение на файлы
;27.10.2020  w5277c@gmail.com        Обновлена информация об авторских правах
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_BCD_BYTE_TO_STR
.else
.set DEF_C5_BCD_BYTE_TO_STR = 1

;--------------------------------------------------------
C5_BCD_BYTE_TO_STR:
;--------------------------------------------------------
;Конвертирование байта в формате BCD в STR(два ASCII символа)
;IN: TEMP - байт
;OUT: TEMP_H,TEMP_L - ASCII
;-START--------------------------------------------------
	MOV TEMP_H,TEMP
	SWAP TEMP_H
	ANDI TEMP_H,0x0f
	SUBI TEMP_H,(0x100-0x30)
	MOV TEMP_L,TEMP
	ANDI TEMP_L,0x0f
	SUBI TEMP_L,(0x100-0x30)
	RET
.endif