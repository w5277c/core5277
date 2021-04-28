;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF_C5_FS_COMMON
.else
.set DEF_C5_FS_COMMON = 1

	;---OFFSETS---
	.EQU	_DRV_FS_SD_DRV_ID											= 0x00;1B ИД драйвера SD карты
	.EQU	_DRV_FS_RTC_DRV_ID										= 0x01;1B ИД драйвера RTC
	.EQU	_DRV_FS_BUFFER_ADDR										= 0x02;2B адрес на буфер 512 байт
	;---
	.EQU	_DRV_FS_VARS_SIZE											= 0x03;

	;---OPERATIONS---
	.EQU	DRV_FS_OP_FORMAT						= 0x01			;Создать FS
	.EQU	DRV_FS_OP_ITEM_INFO					= 0x02			;Считать блок с информацией об элементе
	.EQU	DRV_FS_OP_MD							= 0x02			;Создать
;	.EQU	DRV_FS_CMD_DISK_PGET					= 0x01			;Считать параметр
;	.EQU	DRV_FS_CMD_DISK_PSET					= 0x02			;Записать параметр
;	.EQU	DRV_FS_CMD_DISK_BREAD				= 0x03			;Считать данные с блока
;	.EQU	DRV_FS_CMD_DISK_BWRITE				= 0x04			;Записать данные в блок
;	.EQU	DRV_FS_CMD_DISK_BVERIFY				= 0x05			;Проверить данные в блоке

	;---FS-ITEM-STRUCTURE------------------------------------;Структура элемента каталога FS (40 байт)
	.EQU	DRV_FS_ITEM_ID							= 0x00			;2B - ид элемента
	.EQU	DRV_FS_ITEM_PARENT_ID				= 0x02			;2B - ид каталога, в котором лежит данный элемент
	.EQU	DRV_FS_ITEM_BSTART					= 0x04			;4B - номер первого блока
	.EQU	DRV_FS_ITEM_BLEN						= 0x08			;4B - длина данных в блоках
	.EQU	DRV_FS_ITEM_LEN						= 0x0c			;2B - длина данных в последнем блоке
	.EQU	DRV_FS_ITEM_CTIME						= 0x0e			;4B - время создания
	.EQU	DRV_FS_ITEM_ETIME						= 0x12			;4B - время изменения
	.EQU	DRV_FS_ITEM_OPT						= 0x16			;1B - опции элемента (тип и прочее)
	.EQU	DRV_FS_ITEM_RESERVED					= 0x17			;1B - зарезервировано
	.EQU	DRV_FS_ITEM_NAME						= 0x18			;16B - имя/комментарий

	;---FS-ITEM-OPTIONS--------------------------------------;Опции элемента (побитно)
	.EQU	DRV_FS_OPT_FILE						= 0x00			;каталог - 0, файл - 1
	.EQU	DRV_FS_OPT_LOOPFILE					= 0x01			;циклическая запись в файл
	.EQU	DRV_FS_OPT_REMOVED					= 0x02			;удален
	.EQU	DRV_FS_OPT_BADBLOCK					= 0x03			;битый блок
	.EQU	DRV_FS_OPT_HIDDEN						= 0x04			;скрытый
	.EQU	DRV_FS_OPT_CHECKSUM					= 0x05			;используется контрольная сумма
	.EQU	DRV_FS_OPT_LOOP						= 0x06			;выполнен цикл в циклическом файле
	.EQU	DRV_FS_OPT_FREE						= 0x07			;свободное место

	;---FS-BLOCK-HEADER--------------------------------------;Структура хвоста блока
	.EQU	_DRV_FS_BHEADER_BNUM					= 0x00			;4B - номер следующего блока
	.EQU	_DRV_FS_BHEADER_OFFSET				= 0x04			;2B - смещение внутри блока для записи по кругу (0xffff - не текущий)
	.EQU	_DRV_FS_BHEADER_CHECKSUM			= 0x06			;2B - контрольная сумма


;---FS-RESULT-CODE---------------------------------------;Код результата
	.EQU	DRV_FS_RESULT_OK						= 0x00			;
	.EQU	DRV_FS_RESULT_OP_FAILED				= 0x81			;
	.EQU	DRV_FS_RESULT_INVALID_NAME			= 0x82			;
	.EQU	DRV_FS_RESULT_WRONG_BLOCK			= 0x83			;
	.EQU	DRV_FS_RESULT_UNSUPPORTED			= 0xFF			;


	.EQU	DRV_FS_BLOCK_SIZE						= 0x0200			;Размер блока - 512B
	.EQU	DRV_FS_ITEM_SIZE						= 0x28			;Размер элемента
	.EQU	DRV_FS_NAME_SIZE						= 0x10			;Размер имени
	.EQU	_DRV_FS_BLOCK_HEADER_LEN			= 0x20			;Размер служебного заголовка



;--------------------------------------------------------
DRV_FS_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;IN: Y-адрес на буффер в 512 байт, TEMP_H-ид SD драйвера,
;TEMP_L-ид драйвера RTC
;--------------------------------------------------------
	PUSH ACCUM
	;Выделяем память
	LDI ACCUM,_DRV_FS_VARS_SIZE
	MCALL C5_RAM_REALLOC

	POP ACCUM
	STD Z+_DRV_FS_SD_DRV_ID+0x00,ACCUM
	STD Z+_DRV_FS_BUFFER_ADDR+0x00,YH
	STD Z+_DRV_FS_BUFFER_ADDR+0x01,YL

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: ACCUM-тип операции
;OUT: FLAGS-результат DRV_FS_RESULT_...
;--------------------------------------------------------
	PUSH_Z

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

.ifdef DEF_DRIVER_FS_FORMAT
	CPI ACCUM,DRV_FS_OP_FORMAT
	BRNE PC+0x04
	MCALL _DRV_FS_FORMAT
	RJMP _DRV_FS__END
.endif
	CPI ACCUM,DRV_FS_OP_ITEM_INFO
	BRNE PC+0x04
	MCALL _DRV_FS_ITEM_INFO
	RJMP _DRV_FS__END
	CPI ACCUM,DRV_FS_OP_BLOCK_READ
	BRNE PC+0x04
	MCALL _DRV_FS_BLOCK_READ
	RJMP _DRV_FS__END
	;...

	LDI FLAGS,DRV_FS_RESULT_UNSUPPORTED
_DRV_FS__END:

	POP_Z
	RET

;--------------------------------------------------------
_DRV_FS_ITEM_INFO:
;--------------------------------------------------------
;Считывает блок с элементом и возвращаем его смещение
;IN: TEMP_H/L-ITEM_ID,
;OUT: X-смещение в блоке
;FLAGS-код результата
;--------------------------------------------------------
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH_Y

	;Умножаю на 40 номер элемента
	PUSH TEMP_L
	PUSH TEMP_H
	LDI TEMP,0x0c
	MCALL DIV16X8

	MOV YH,TEMP_H
	MOV YL,TEMP_L
	CLR TEMP_EH
	CLR TEMP_EL
	CLR TEMP_H
	CLR TEMP_L
C5_FS_ITEM_INFO__LOOP:
	MCALL C5_FS_READ_NEXT_BLOCK
	CPI FLAGS,C5_FS_RESULT_OK
	BRNE C5_FS_ITEM_INFO__END
	SBIW YL,0x01
	BRCC C5_FS_ITEM_INFO__LOOP

	;Нахожу номер записи в блоке
	MCALL MUL16X8
	POP TEMP
	POP TEMP
	SUB TEMP,TEMP_L
	;Умножаю на размер записи
	CLR TEMP_H
	MOV TEMP_L,TEMP
	LDI TEMP,40
	MCALL MUL16X8
	MOV XH,TEMP_H
	MOV XL,TEMP_L

C5_FS_ITEM_INFO__END:
	POP TEMP_EL
	POP TEMP_EH
	POP Y
	RET

;--------------------------------------------------------
_C5_FS_READ_NEXT_BLOCK:
;--------------------------------------------------------
;Считывает следующий блок
;IN: TEMP_EH/EL/H/L-номер блока
;OUT: TEMP_EH/EL/H/L-номер блока,
;FLAGS-код результата
;--------------------------------------------------------
	MCALL _C5_FS_BLOCK_READ

	LDD TEMP_EH,Y+_C5_FS_BHEADER_BNUM+0x00
	LDD TEMP_EL,Y+_C5_FS_BHEADER_BNUM+0x01
	LDD TEMP_H,Y+_C5_FS_BHEADER_BNUM+0x02
	LDD TEMP_L,Y+_C5_FS_BHEADER_BNUM+0x03
	RET


;--------------------------------------------------------
_C5_FS_BLOCK_READ:
;--------------------------------------------------------
;IN: TEMP_EH/EL/H/L-номер блока
;OUT: FLAGS-код результата
;--------------------------------------------------------
	PUSH TEMP
	PUSH ACCUM

	LDD TEMP,Z+_C5_FS_DRV_ID
	LDI FLAGS,DRV_SD_OP_READ_BLOCK
	MCALL C5_EXEC
	LDI FLAGS,DRV_FS_RESULT_OP_FAILED
	CPI ACCUM,0x00
	BRNE PC+0x02
	LDI FLAGS,DRV_FS_RESULT_OK

	POP ACCUM
	POP TEMP
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
