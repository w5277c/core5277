;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
;TODO C5_RAM_OFFSET Z->Y
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_C5_FS_COMMON
.else
.set DEF_C5_FS_COMMON = 1

.include	"./mem/ram_fill.inc"

	;---OFFSETS---
	.EQU	_DRV_FS_SD_DRV_ID											= 0x00;1B ИД драйвера SD карты
	.EQU	_DRV_FS_RTC_DRV_ID										= 0x01;1B ИД драйвера RTC
	.EQU	_DRV_FS_BUFFER_ADDR										= 0x02;2B адрес на буфер 512 байт
	.EQU	_DRV_FS_CUR_DIR											= 0x04;2B ID текущей директории
	.EQU	_DRV_FS_CUR_FILE											= 0x06;2B ID текущего файла
	.EQU	_DRV_FS_TEMP_ITEM											= 0x08;2B ID текущего файла (исп. для внутренних циклов и прочего)
	.EQU	_DRV_FS_ITEM_BUFFER										= 0x0a;32B размер буфера для работы с ITEM
	;---
	.EQU	_DRV_FS_VARS_SIZE											= 0x2a;

	;---OPERATIONS---
	.EQU	DRV_FS_OP_FORMAT						= 0x01			;Создать FS
	.EQU	DRV_FS_OP_ITEM_INFO					= 0x02			;Считать блок с информацией об элементе
	.EQU	DRV_FS_OP_MD							= 0x02			;Создать
;	.EQU	DRV_FS_CMD_DISK_PGET					= 0x01			;Считать параметр
;	.EQU	DRV_FS_CMD_DISK_PSET					= 0x02			;Записать параметр
;	.EQU	DRV_FS_CMD_DISK_BREAD				= 0x03			;Считать данные с блока
;	.EQU	DRV_FS_CMD_DISK_BWRITE				= 0x04			;Записать данные в блок
;	.EQU	DRV_FS_CMD_DISK_BVERIFY				= 0x05			;Проверить данные в блоке

	;---FS-ITEM-STRUCTURE------------------------------------;Структура элемента каталога FS (32 байт)
	.EQU	DRV_FS_ITEM_PARENT_ID				= 0x02			;2B - ид каталога, в котором лежит данный элемент
	.EQU	DRV_FS_ITEM_BSTART					= 0x04			;4B - номер первого блока
	.EQU	DRV_FS_ITEM_BLEN						= 0x08			;3B - длина данных в блоках
	.EQU	DRV_FS_ITEM_LEN						= 0x0c			;2B - длина данных в последнем блоке для FILE
																			;или номер последнего занятого элемента для ROOT
																			;или номер текущего блока для LOOPFILE
	.EQU	DRV_FS_ITEM_CTIME						= 0x0e			;4B - время создания
	.EQU	DRV_FS_ITEM_ETIME						= 0x12			;4B - время изменения
	.EQU	DRV_FS_ITEM_OPT						= 0x16			;1B - опции элемента (тип и прочее)
	.EQU	DRV_FS_ITEM_NAME						= 0x18			;12B - имя/комментарий

	;---FS-ITEM-OPTIONS-H-NIBBLE--------------------------;Опции элемента
	;---H-NIBBLE------------------------------------------;Тип элемента - старший ниббл
	.EQU	DRV_FS_TYPE_FREE						= 0x00			;свободный элемент
	.EQU	DRV_FS_TYPE_DIR						= 0x01			;каталог
	.EQU	DRV_FS_TYPE_FILE						= 0x02			;файл
	.EQU	DRV_FS_TYPE_LOOPFILE					= 0x03			;файл с циклической записью
	.EQU	DRV_FS_TYPE_RM_DIR					= 0x04			;удаленная директория
	.EQU	DRV_FS_TYPE_RM_FILE					= 0x05			;удаленный файл
;	.EQU	DRV_FS_TYPE_RESERVED					= 0x06			;зарезервировано
;	.EQU	DRV_FS_TYPE_RESERVED					= 0x07			;зарезервировано
	;---L-NIBBLE------------------------------------------;опции элемента - младший ниббл
	;---DIR/FILE/RM-DIR/RM-FILE---
	.EQU	DRV_FS_OPT_HIDDEN						= 0x04			;скрытый
	;---LOOP-FILE---
	.EQU	DRV_FS_OPT_HIDDEN						= 0x04			;скрытый
	.EQU	DRV_FS_OPT_LOOP						= 0x05			;выполнен цикл в циклическом файле

	;---FS-BLOCK-HEADER------------------------------------;Структура хвоста блока
	.EQU	_DRV_FS_BHEADER_BNUM					= 0x00			;4B - номер следующего блока
	.EQU	_DRV_FS_BHEADER_OFFSET				= 0x04			;2B - смещение внутри блока для записи по кругу (0xffff - не текущий)
	.EQU	_DRV_FS_BHEADER_CHECKSUM			= 0x06			;2B - контрольная сумма


;---FS-RESULT-CODE---------------------------------------;Код результата
	.EQU	DRV_FS_RESULT_OK						= 0x00			;
	.EQU	DRV_FS_RESULT_OP_FAILED				= 0x81			;
	.EQU	DRV_FS_RESULT_INVALID_NAME			= 0x82			;
	.EQU	DRV_FS_RESULT_INVALID_ITEM			= 0x83			;
	.EQU	DRV_FS_RESULT_WRONG_BLOCK			= 0x84			;
	.EQU	DRV_FS_RESULT_UNSUPPORTED			= 0xFF			;


	.EQU	DRV_FS_BLOCK_SIZE						= 0x0200			;Размер блока - 512B
	.EQU	DRV_FS_ITEM_SIZE						= 0x20			;Размер элемента
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

	MOV XH,ZH
	MOV XL,ZL
	LDI TEMP,0x00
	MOV LOOP_CNTR,ACCUM
	MCALL RAM_FILL

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
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_FORMAT
	RJMP _DRV_FS__END
.endif
	CPI ACCUM,DRV_FS_OP_ITEM_INFO
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_ITEM_INFO
	RJMP _DRV_FS__END
	CPI ACCUM,DRV_FS_OP_BLOCK_READ
	BRNE PC+0x01+_MCALL_SIZE+0x01
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
;IN: Z-адрес на переменные, TEMP_H/L-ITEM_ID
;OUT: FLAGS-код результата
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH TEMP
	PUSH LOOP_CNTR

	CLR TEMP_EH
	CLR TEMP_EL
	MCALL _DRV_FS_ITEM_OFFSETS
	;Считываю блок
	MCALL _C5_FS_BLOCK_READ
	CPI FLAGS,C5_FS_RESULT_OK
	BRNE C5_FS_ITEM_INFO__END

	LDD TEMP,Z+_DRV_FS_BUFFER_ADDR+0x01
	ADD XL,TEMP
	LDD TEMP_L,Y+_DRV_FS_BUFFER_ADDR+0x00
	ADC XH,TEMP

	LDD TEMP,Z+_DRV_FS_ITEM_BUFFER+0x00
	LDD ZL,Z+_DRV_FS_ITEM_BUFFER+0x01
	MOV ZH,TEMP
	LDI LOOP_CNTR,DRV_FS_ITEM_SIZE
	MCALL RAM_COPY
	LDI FLAGS,C5_FS_RESULT_OK

C5_FS_ITEM_INFO__END:
	POP LOOP_CNTR
	POP TEMP
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_X
	RET


;--------------------------------------------------------
_DRV_FS_ITEM_OFFSETS:
;--------------------------------------------------------
;Получаем номер блока и адрес в буфере
;IN: TEMP_H/L-ITEM_ID
;OUT: TEMP_H,L-номер блока,X-адрес в буфере
;--------------------------------------------------------
	PUSH TEMP

	PUSH TEMP_L
	;Вычисляю номер блока
	;Делю на 16
	SWAP TEMP_L
	ANDI TEMP_L,0x0f
	SWAP TEMP_H
	MOV TEMP,TEMP_H
	ANDI TEMP,0xf0
	OR TEMP_L,TEMP
	ANDI TEMP_H,0x0f

	POP XL
	;Вычисляю адрес элемента в буфере
	ANDI XL,0x0f
	;Умножаю на 32
	CLR XH
	SWAP XL
	LSL XL
	ROL XH

	POP TEMP
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

.endif
