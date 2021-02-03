;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FS_COMMON
.else
.set DEF_C5_FS_COMMON = 1


;---FS-ITEM-STRUCTURE------------------------------------;Структура элемента каталога FS (48 байт)
	.EQU	_C5_FS_ITEM_ID							= 0x00			;2B - ид элемента
	.EQU	_C5_FS_ITEM_PARENT_ID				= 0x02			;2B - ид каталога, в котором лежит данный элемент
	.EQU	_C5_FS_ITEM_OPT						= 0x04			;1B - опции элемента (тип и прочее)
	.EQU	_C5_FS_ITEM_BSTART					= 0x05			;3B - номер первого блока
	.EQU	_C5_FS_ITEM_BLEN						= 0x08			;3B - длина данных в блоках
	.EQU	_C5_FS_ITEM_LEN						= 0x0b			;2B - длина данных в последнем блоке
	.EQU	_C5_FS_ITEM_CTIME						= 0x0d			;4B - время создания
	.EQU	_C5_FS_ITEM_ETIME						= 0x11			;4B - время изменения
	.EQU	_C5_FS_ITEM_RESERVED					= 0x1a			;5B - зарезервировано
	.EQU	_C5_FS_ITEM_NAME						= 0x16			;16B - имя/комментарий


;---FS-ITEM-OPTIONS--------------------------------------;Опции элемента (побитно)
	.EQU	_C5_FS_OPT_FILE						= 0x00			;каталог - 0, файл - 1
	.EQU	_C5_FS_OPT_LOOPFILE					= 0x01			;циклическая запись в файл
	.EQU	_C5_FS_OPT_REMOVED					= 0x02			;удален
	.EQU	_C5_FS_OPT_BADBLOCK					= 0x03			;битый блок
	.EQU	_C5_FS_OPT_HIDDEN						= 0x04			;скрытый
	.EQU	_C5_FS_OPT_CHECKSUM					= 0x05			;используется контрольная сумма
	.EQU	_C5_FS_OPT_LOOP						= 0x06			;выполнен цикл в циклическом файле
	.EQU	_C5_FS_OPT_FREE						= 0x07			;свободное место

;---FS-COMMANDS------------------------------------------;Кманды поддерживаемые FS
	.EQU	C5_FS_CMD_DISK_PGET					= 0x01			;Считать параметр
	.EQU	C5_FS_CMD_DISK_PSET					= 0x02			;Записать параметр
	.EQU	C5_FS_CMD_DISK_BREAD					= 0x03			;Считать данные с блока
	.EQU	C5_FS_CMD_DISK_BWRITE				= 0x04			;Записать данные в блок
	.EQU	C5_FS_CMD_DISK_BVERIFY				= 0x05			;Проверить данные в блоке


;---FS-BLOCK-TILE----------------------------------------;Структура хвоста блока
	.EQU	_C5_FS_BTILE_BNUM						= 0x00			;3B - номер следующего блока
	.EQU	_C5_FS_BTILE_OFFSET					= 0x03			;2B - смещение внутри блока для записи по кругу (0xffff - не текущий)
	.EQU	_C5_FS_BTILE_RESERVED				= 0x05			;1B - не используется (должно быть равно 0xc5)
	.EQU	_C5_FS_BTILE_CHECKSUM				= 0x06			;2B - опции элемента (тип и прочее)


;---FS-RESULT-CODE---------------------------------------;Код результата
	.EQU	C5_FS_RESULT_OK						= 0x00			;
	.EQU	C5_FS_RESULT_WRONG_BLOCK			= 0x81			;


	.EQU	_C5_FS_BLOCK_TILE_LEN				= 0x08			;размер служебного хвоста в блоке

;--------------------------------------------------------
_C5_FS_ITEM_OFFSET:
;--------------------------------------------------------
;Смещение в блоках на элемент каталога
;IN: TEMP_H,TEMP_L - порядковый номер элемента,
;TEMP_EH,TEMP_EL - размер блока
;ACCUM - ид драйвера устройства
;OUT: TEMP_H,TEMP_L,TEMP - номер блока
;TEMP_EH,TEMP_EL - смещение в блоке
;FLAGS - код результата
;--------------------------------------------------------
	PUSH ACCUM
	PUSH_X
	PUSh ZL
	;Умножаю на 48 номер элемента
	SWAP TEMP_L
	MOV TEMP_L,TEMP
	ANDI TEMP,0xf0
	SWAP TEMP_H
	MOV TEMP_H,ACCUM
	ANDI ACCUM,0xf0
	ANDI TEMP_L,0x0f
	OR TEMP_L,ACCUM
	ANDI TEMP_H,0x0f
	MOV XH,TEMP_H
	MOV XL,TEMP_L
	MOV ZL,TEMP
	LSL TEMP
	LSL TEMP_L
	LSL TEMP_H
	ADD ZL,TEMP
	ADC XL,TEMP_L
	ADC XH,TEMP_H

	SUBI TEMP_EH,low(_C5_FS_BLOCK_TILE_LEN)
	SBCI TEMP_EL,high(_C5_FS_BLOCK_TILE_LEN)
	LDI TEMP_H,0x00
	LDI TEMP_L,0x00
	LDI TEMP,0x00
	MCALL _C5_FS_BLOCK_OFFSET

	POP ZL
	POP_X
	POP ACCUM
	RET

;--------------------------------------------------------
_C5_FS_BLOCK_OFFSET:
;--------------------------------------------------------
;Получаем смещение в блоках
;IN: XH,XL,ZL - смещение в байтах
;TEMP_H,TEMP_L,TEMP - номер первого блока
;TEMP_EH,TEMP_EL - размер блока (без хвоста)
;ACCUM - ид драйвера устройства
;OUT: TEMP_H,TEMP_L,TEMP - номер блока
;TEMP_EH,TEMP_EL - смещение в блоке
;FLAGS - код результата
;--------------------------------------------------------
	PUSH_X
	PUSH ZL
	PUSH_Y
	;Выделяем память под буфер
	PUSH_Z
	PUSH ACCUM
	LDI ACCUM,_C5_FS_BLOCK_TILE_LEN
	MCALL C5_RAM_EXTEND
	POP ACCUM
	POP_Z

__C5_FS_BLOCK_OFFSET_LOOP:
	CPI XH,0x00
	BRNE __C5_FS_BLOCK_OFFSET_NEXT_BLOCK
	CP XL,TEMP_EH
	BRCC __C5_FS_BLOCK_OFFSET_NEXT_BLOCK
	CP ZL,TEMP_EL
	BRCC __C5_FS_BLOCK_OFFSET_NEXT_BLOCK
	;Входим в текущий блок
	MOV TEMP_EH,XL
	MOV TEMP_EL,ZL
	LDI FLAGS,C5_FS_RESULT_OK
	RJMP __C5_FS_BLOCK_OFFSET_NEXT_BLOCK_END

__C5_FS_BLOCK_OFFSET_NEXT_BLOCK:
	SUB ZL,TEMP_EL
	SBC XL,TEMP_EH
	SBCI XH,0x00
	MCALL _C5_FS_BLOCK_GET_NEXT_NUM
	CPI FLAGS,C5_FS_RESULT_OK
	BRNE __C5_FS_BLOCK_OFFSET_NEXT_BLOCK_END
	CPI TEMP_H,0x00
	BRNE __C5_FS_BLOCK_OFFSET_LOOP
	CPI TEMP_L,0x00
	BRNE __C5_FS_BLOCK_OFFSET_LOOP
	CPI TEMP,0x00
	BRNE __C5_FS_BLOCK_OFFSET_LOOP
	LDI FLAGS,C5_FS_RESULT_WRONG_BLOCK
	RJMP __C5_FS_BLOCK_OFFSET_NEXT_BLOCK_END

__C5_FS_BLOCK_OFFSET_NEXT_BLOCK_END:
	;Освобождаем память
	PUSH TEMP
	PUSH_Z
	PUSH ACCUM
	LDI ACCUM,_C5_FS_BLOCK_TILE_LEN
	MCALL C5_RAM_REDUCE
	POP ACCUM
	POP_Z
	POP TEMP
	POP_Y
	POP ZL
	POP_X
	RET
;--------------------------------------------------------
_C5_FS_BLOCK_GET_NEXT_NUM:
;--------------------------------------------------------
;Получаем номер следующего блока
;IN: TEMP_H,TEMP_L,TEMP - номер блока
;TEMP_EH,TEMP_EL - размер блока (без хвоста)
;ACCUM - ид драйвера устройства
;Y - адрес на буфер размерностью
;не менее _C5_FS_BLOCK_TILE_LEN
;OUT: TEMP_H,TEMP_L,TEMP - номер следующего блока
;FLAGS - код результата
;--------------------------------------------------------
	;Читаем хвост
	PUSH_X
	MOV XH,TEMP_EH
	MOV XL,TEMP_EL
	LDI LOOP_CNTR,_C5_FS_BLOCK_TILE_LEN
	MCALL C5_FS_BLOCK_READ
	POP_X
	;Получаем новый номер блока
	LDD TEMP_H,Y+_C5_FS_BTILE_BNUM+0x00
	LDD TEMP_L,Y+_C5_FS_BTILE_BNUM+0x01
	LDD TEMP,Y+_C5_FS_BTILE_BNUM+0x02
	RET

;--------------------------------------------------------
C5_FS_BLOCK_READ:
;--------------------------------------------------------
;Читаем данные с блока
;IN: TEMP_H,TEMP_L,TEMP - номер блока
;X - SRC, смещение в блоке
;Y - DST, адрес на буффер
;LOOP_CNTR - длина данных
;ACCUM - ид драйвера устройства
;OUT: FLAGS - код результата
;--------------------------------------------------------
	...





.endif
