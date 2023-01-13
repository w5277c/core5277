;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;26.12.2020	w5277c@gmail.com			Начало
;23.10.2022	konstantin@5277.ru			В процессе
;-----------------------------------------------------------------------------------------------------------------------
;TODO
;Обновить процедуры format и обновление root элемента, проверить новый функционал
;
;
;Добавить процедуру позиционирования в файле(относительно и абсолютно),
;записи и чтения блока данных(не более 255 байт), добавления данных.
;Также удаления файла и директории и переименование. Управление флагами. И т.д.
;Идентификация файлов выполняется по их ид.
;-----------------------------------------------------------------------------------------------------------------------

.ifdef DEF_C5_FS_COMMON
.else
.set DEF_C5_FS_COMMON = 1

.include "./core/drivers/_fs.inc"
.include	"./mem/ram_fill.inc"
.include	"./mem/ram_copy.inc"
.include "./core/uptime_delta.inc"
.include "./core/uptime_copy.inc"
.include "./mem/ram_cp.inc"

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_INF
		.include	"./core/io/out_strn.inc"
		.include	"./core/io/out_str_el.inc"
		.include	"./core/io/out_num16.inc"
		.include	"./core/io/out_num32.inc"
_DRV_FS_LOG_STR_INIT:
		.db	"C5FS, disk[",0x00,"] ",0x00, "not formatted ",0x00," of ",0x00," blks(",0x00,"b)",0x0d,0x0a,0x00;
	.endif
.endif

	;---OFFSETS---
	.EQU	_DRV_FS_DISK_DRV_ID										= 0x00;1B ИД драйвера диска
	.EQU	_DRV_FS_RTC_DRV_ID										= 0x01;1B ИД драйвера RTC
	.EQU	_DRV_FS_BUFFER_ADDR										= 0x02;2B адрес на буфер 512 байт
	.EQU	_DRV_FS_DISK_SIZE											= 0x04;4B количество блоков
	.EQU	_DRV_FS_DISK_TIME											= 0x08;5B uptime последней операции
	.EQU	_DRV_FS_DISK_ID											= 0x0d;8B первые 8 байт идентификатора диска
	.EQU	_DRV_FS_ITEM_BUFFER										= 0x15;32B размер буфера для работы с ITEM
	;---
	.EQU	_DRV_FS_RAM_SIZE											= 0x35;53B


	;---FS-ROOT-STRUCTURE---------------------------------;Струкура элемента ROOT
	.EQU	DRV_FS_ROOT_MAGIC						= 0x00			;2B - 5277
	.EQU	DRV_FS_ROOT_VERSION					= 0x02			;1B - версия x/10
	.EQU	DRV_FS_ROOT_CURRENT_BLK				= 0x03			;4B - текущий свободный блок(за ним свободное пространство)
	.EQU	DRV_FS_ROOT_USED_BLKS				= 0x07			;4B - количество занятых блоков
	.EQU	DRV_FS_ROOT_CTIME						= 0x0b			;4B - время форматирования
	.EQU	DRV_FS_ROOT_ETIME						= 0x0f			;4B - время изменения блока ROOT
	.EQU	DRV_FS_ROOT_OPT						= 0x13			;1B - опции элемента (тип и прочее)
	.EQU	DRV_FS_ROOT_NAME						= 0x14			;12B - имя
	;---FS-ITEM-STRUCTURE---------------------------------;Структура элемента каталога FS (32 байт)
	.EQU	DRV_FS_ITEM_PARENT_ID				= 0x00			;2B - ид каталога, в котором лежит данный элемент
	.EQU	DRV_FS_ITEM_BSTART					= 0x02			;4B - номер первого блока
	.EQU	DRV_FS_ITEM_BLEN						= 0x06			;3B - длина данных в блоках(занято на диске для ROOT)
	.EQU	DRV_FS_ITEM_LEN						= 0x09			;2B - длина данных в последнем блоке для FILE или номер текущего блока для LOOPFILE
	.EQU	DRV_FS_ITEM_CTIME						= 0x0b			;4B - время создания
	.EQU	DRV_FS_ITEM_ETIME						= 0x0f			;4B - время изменения
	.EQU	DRV_FS_ITEM_OPT						= 0x13			;1B - опции элемента (тип и прочее)
	.EQU	DRV_FS_ITEM_NAME						= 0x14			;12B - имя

	;---FS-ITEM-OPTIONS-H-NIBBLE--------------------------;Опции элемента
	;---L-NIBBLE------------------------------------------;Тип элемента - младший ниббл
	.EQU	DRV_FS_TYPE_FREE						= 0x00			;свободный элемент
	.EQU	DRV_FS_TYPE_DIR						= 0x01			;каталог
	.EQU	DRV_FS_TYPE_FILE						= 0x02			;файл
	.EQU	DRV_FS_TYPE_LOOPFILE					= 0x03			;файл с циклической записью
	.EQU	DRV_FS_TYPE_RM_DIR					= 0x04			;удаленная директория
	.EQU	DRV_FS_TYPE_RM_FILE					= 0x05			;удаленный файл
;	.EQU	DRV_FS_TYPE_RESERVED					= 0x06			;зарезервировано
;	.EQU	DRV_FS_TYPE_RESERVED					= 0x07			;зарезервировано
	;---H-NIBBLE------------------------------------------;опции элемента - старший ниббл(биты)
	;---DIR/FILE/RM-DIR/RM-FILE---
	.EQU	DRV_FS_OPT_HIDDEN						= 0x04			;скрытый
	.EQU	DRV_FS_OPT_RO							= 0x05			;только для чтения
	.EQU	DRV_FS_OPT_SYSTEM						= 0x06			;системный
	;---LOOP-FILE---
	.EQU	DRV_FS_OPT_LOOP						= 0x07			;выполнен цикл в циклическом файле

	;---FS-BLOCK-HEADER------------------------------------;Структура хвоста блока
	.EQU	_DRV_FS_BHEADER_BNUM					= 0x00			;4B - номер следующего блока
	.EQU	_DRV_FS_BHEADER_OFFSET				= 0x04			;2B - смещение внутри блока для записи по кругу (0xffff - не текущий)
	.EQU	_DRV_FS_BHEADER_CHECKSUM			= 0x06			;2B - контрольная сумма


	.EQU	DRV_FS_BLOCK_SIZE						= 0x0200			;Размер блока - 512B
	.EQU	DRV_FS_ITEM_SIZE						= 0x20			;Размер элемента
	.EQU	DRV_FS_NAME_SIZE						= 0x10			;Размер имени
	.EQU	_DRV_FS_BLOCK_HEADER_LEN			= 0x20			;Размер служебного заголовка


	.EQU	DRV_FS_OPEN_WRITE						= 0x00			;Открытие файла для записи


;--------------------------------------------------------
DRV_FS_INIT:
;--------------------------------------------------------
;Инициализация драйвера
;IN: X-адрес на буффер в 512 байт,
;TEMP_H-ид драйвера диска, TEMP_L-ид драйвера RTC
;--------------------------------------------------------
	;Выделяем память
	LDI ACCUM,_DRV_FS_RAM_SIZE
	MCALL C5_RAM_REALLOC

	PUSH_X
	MOVW XL,YL
	LDI TEMP,0x00
	MOV LOOP_CNTR,ACCUM
	MCALL RAM_FILL

	POP_X
	STD Y+_DRV_FS_DISK_DRV_ID,TEMP_H
	STD Y+_DRV_FS_RTC_DRV_ID,TEMP_L
	STD Y+_DRV_FS_BUFFER_ADDR+0x00,XH
	STD Y+_DRV_FS_BUFFER_ADDR+0x01,XL

	LDI TEMP,0xff
	STD Y+_DRV_FS_DISK_SIZE+0x00,TEMP
	STD Y+_DRV_FS_DISK_SIZE+0x01,TEMP
	STD Y+_DRV_FS_DISK_SIZE+0x02,TEMP
	STD Y+_DRV_FS_DISK_SIZE+0x03,TEMP

	MCALL C5_READY
;--------------------------------------------------------
;Основной код, коммуникация
;IN: FLAGS-тип операции
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Y

	;Получаем адрес выделенной памяти
	MCALL C5_RAM_OFFSET

	LDD XH,Y+_DRV_FS_BUFFER_ADDR+0x00
	LDD XL,Y+_DRV_FS_BUFFER_ADDR+0x01

	LDD TEMP,Y+_DRV_FS_DISK_SIZE+0x00
	CPI TEMP,0xff
	BRNE PC+0x0c+_MCALL_SIZE
	LDD TEMP,Y+_DRV_FS_DISK_SIZE+0x01
	CPI TEMP,0xff
	BRNE PC+0x09+_MCALL_SIZE
	LDD TEMP,Y+_DRV_FS_DISK_SIZE+0x02
	CPI TEMP,0xff
	BRNE PC+0x06+_MCALL_SIZE
	LDD TEMP,Y+_DRV_FS_DISK_SIZE+0x03
	CPI TEMP,0xff
	BRNE PC+0x03+_MCALL_SIZE
	MCALL _DRV_FS_DISK_INIT
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS__END

	MCALL _DRV_FS_CHECK_DISK
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x05
	CPI TEMP,DRV_RESULT_INVALID_RESOURCE
	BRNE _DRV_FS__END
	CPI FLAGS,DRV_FS_OP_FORMAT
	BRNE _DRV_FS__END

.ifdef DEF_DRIVER_FS_FORMAT
	CPI FLAGS,DRV_FS_OP_FORMAT
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_OP_FORMAT
	RJMP _DRV_FS__END
.endif
.ifdef DEF_DRIVER_FS_MD
	CPI FLAGS,DRV_FS_OP_MD
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_OP_MD
	RJMP _DRV_FS__END
.endif
.ifdef DEF_DRIVER_FS_CD
	CPI FLAGS,DRV_FS_OP_CD
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_OP_CD
	RJMP _DRV_FS__END
.endif
	CPI FLAGS,DRV_FS_OP_INFO
	BRNE PC+0x01+_MCALL_SIZE+0x01
	MCALL _DRV_FS_ITEM_READ
	RJMP _DRV_FS__END
;	CPI FLAGS,DRV_FS_OP_BLOCK_READ
;	BRNE PC+0x01+_MCALL_SIZE+0x01
;	MCALL _DRV_FS_BLOCK_READ
;	RJMP _DRV_FS__END
	;...

	LDI TEMP,DRV_RESULT_UNSUPPORTED
_DRV_FS__END:

	POP_Y
	POP_X
	RET

;--------------------------------------------------------
_DRV_FS_ITEM_READ:
;--------------------------------------------------------
;Считывает блок с элементом и возвращаем его смещение
;IN: Y-адрес на переменные, TEMP_H/L-ITEM_ID
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	CLR TEMP_EH
	CLR TEMP_EL
	MCALL _DRV_FS_ITEM_OFFSETS
	;Считываю блок
	MCALL _DRV_FS_BLOCK_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_ITEM_READ__END

	LDD TEMP,Y+_DRV_FS_BUFFER_ADDR+0x01
	ADD XL,TEMP
	LDD TEMP,Y+_DRV_FS_BUFFER_ADDR+0x00
	ADC XH,TEMP

	MOVW ZL,YL
	ADIW ZL,_DRV_FS_ITEM_BUFFER
	LDI LOOP_CNTR,DRV_FS_ITEM_SIZE
	MCALL RAM_COPY
	LDI TEMP,DRV_RESULT_OK

_DRV_FS_ITEM_READ__END:
	POP LOOP_CNTR
	POP TEMP_L
	POP TEMP_H
	POP TEMP_EL
	POP TEMP_EH
	POP_Z
	POP_X
	RET

;--------------------------------------------------------
_DRV_FS_ITEM_WRITE:
;--------------------------------------------------------
;Записывает блок элемента
;IN: Y-адрес на переменные, TEMP_H/L-ITEM_ID
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH TEMP_EH
	PUSH TEMP_EL
	PUSH TEMP_H
	PUSH TEMP_L
	PUSH LOOP_CNTR

	CLR TEMP_EH
	CLR TEMP_EL
	MCALL _DRV_FS_ITEM_OFFSETS
	;Считываем блок
	MCALL _DRV_FS_BLOCK_READ
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_ITEM_WRITE__END
	;Копируем данные элемента в блок.
	MOVW ZL,XL
	LDD TEMP,Y+_DRV_FS_BUFFER_ADDR+0x01
	ADD ZL,TEMP
	LDD TEMP,Y+_DRV_FS_BUFFER_ADDR+0x00
	ADC ZH,TEMP
	MOVW XL,YL
	ADIW XL,_DRV_FS_ITEM_BUFFER
	LDI LOOP_CNTR,DRV_FS_ITEM_SIZE
	MCALL RAM_COPY
	;Записываем блок
	MCALL _DRV_FS_BLOCK_WRITE

_DRV_FS_ITEM_WRITE__END:
	POP LOOP_CNTR
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
_DRV_FS_READ_NEXT_BLOCK:
;--------------------------------------------------------
;Считывает следующий блок
;IN: TEMP_EH/EL/H/L-номер блока
;OUT: TEMP_EH/EL/H/L-номер блока, TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	LDS XH,_DRV_FS_BUFFER_ADDR+0x00
	LDS XL,_DRV_FS_BUFFER_ADDR+0x01

	MCALL _DRV_FS_BLOCK_READ

	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_READ_NEXT_BLOCK__END
	PUSH_Y
	MOVW YL,XL
	LDD TEMP_EH,Y+_DRV_FS_BHEADER_BNUM+0x00
	LDD TEMP_EL,Y+_DRV_FS_BHEADER_BNUM+0x01
	LDD TEMP_H,Y+_DRV_FS_BHEADER_BNUM+0x02
	LDD TEMP_L,Y+_DRV_FS_BHEADER_BNUM+0x03
	POP_Y
_DRV_FS_READ_NEXT_BLOCK__END:
	POP_X
	RET

;--------------------------------------------------------
_DRV_FS_BLOCK_READ:
;--------------------------------------------------------
;IN: TEMP_EH/EL/H/L-номер блока,X-адрес буфера
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH FLAGS

	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_READ
	MCALL C5_EXEC

	POP FLAGS
	RET

;--------------------------------------------------------
_DRV_FS_BLOCK_WRITE:
;--------------------------------------------------------
;IN: TEMP_EH/EL/H/L-номер блока,X-адрес буфера
;OUT: TEMP-DRV_RESULT_...
;--------------------------------------------------------
	PUSH FLAGS

	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_WRITE
	MCALL C5_EXEC

	POP FLAGS
	RET


;--------------------------------------------------------
_DRV_FS_DISK_INIT:
;--------------------------------------------------------
;IN: Y-адрес на переменные, X-адрес буфера
;OUT: TEMP-DEV_RESULT_...
;--------------------------------------------------------
	PUSH_X
	PUSH_Z
	PUSH FLAGS
	PUSH ACCUM
	PUSH LOOP_CNTR

	LDI ACCUM,0x00
	
	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_INIT
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_FS_DISK_INIT__END

	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_GET_ID
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_FS_DISK_INIT__END
	MOVW ZL,YL
	ADIW ZL,_DRV_FS_DISK_ID
	MCALL RAM_COPY

	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_GET_SIZE
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_FS_DISK_INIT__END
	STD Y+_DRV_FS_DISK_SIZE+0x00,TEMP_EH
	STD Y+_DRV_FS_DISK_SIZE+0x01,TEMP_EL
	STD Y+_DRV_FS_DISK_SIZE+0x02,TEMP_H
	STD Y+_DRV_FS_DISK_SIZE+0x03,TEMP_L

	LDI_T32 0x00000000
	MCALL _DRV_FS_BLOCK_READ
	CPI TEMP,DRV_RESULT_OK
	BREQ PC+0x02
	RJMP _DRV_FS_DISK_INIT__END
	LD ZL,X+
	CPI ZL,0x52
	BREQ PC+0x02
	LDI ACCUM,0x01
	LD ZL,X+
	CPI ZL,0x77
	BREQ PC+0x02
	LDI ACCUM,0x01
	LD ZL,X+
	CPI ZL,0x01
	BREQ PC+0x02
	LDI ACCUM,0x01
	SBIW XL,0x03

	ADIW YL,_DRV_FS_DISK_TIME
	MCALL C5_UPTIME_COPY
	SBIW YL,_DRV_FS_DISK_TIME

.ifdef LOGGING_PORT
	.if LOGGING_LEVEL >= LOGGING_LVL_INF
		CPI ACCUM,0x01
		BREQ _DRV_FS_DISK_INIT__LOG_SKIP							;GONE
		LDI_Z _DRV_FS_LOG_STR_INIT|0x8000
		LDI TEMP,0x00
		MCALL C5_OUT_STR_EL
		MOVW ZL,YL
		ADIW ZL,_DRV_FS_DISK_ID
		MCALL C5_OUT_STR
		LDI_Z _DRV_FS_LOG_STR_INIT|0x8000
		LDI TEMP,0x01
		MCALL C5_OUT_STR_EL

		CPI ACCUM,0x02
		BREQ _DRV_FS_DISK_INIT__LOG_UNFORMATTED
		MOVW ZL,XL
		ADIW ZL,DRV_FS_ROOT_NAME
		LDI TEMP,0x0c
		MCALL C5_OUT_STRN
		LDI TEMP,' '
		MCALL C5_OUT_CHAR
		MOVW ZL,XL
		LDD TEMP_EH,Z+DRV_FS_ROOT_USED_BLKS+0x00
		LDD TEMP_EL,Z+DRV_FS_ROOT_USED_BLKS+0x01
		LDD TEMP_H,Z+DRV_FS_ROOT_USED_BLKS+0x02
		LDD TEMP_L,Z+DRV_FS_ROOT_USED_BLKS+0x03
		MCALL C5_OUT_NUM32
		LDI_Z _DRV_FS_LOG_STR_INIT|0x8000
		LDI TEMP,0x03
		MCALL C5_OUT_STR_EL
		RJMP _DRV_FS_DISK_INIT__LOG_LAST_PART
_DRV_FS_DISK_INIT__LOG_UNFORMATTED:
		LDI_Z _DRV_FS_LOG_STR_INIT|0x8000
		LDI TEMP,0x02
		MCALL C5_OUT_STR_EL
_DRV_FS_DISK_INIT__LOG_LAST_PART:
		LDD TEMP_EH,Y+_DRV_FS_DISK_SIZE+0x00
		LDD TEMP_EL,Y+_DRV_FS_DISK_SIZE+0x01
		LDD TEMP_H,Y+_DRV_FS_DISK_SIZE+0x02
		LDD TEMP_L,Y+_DRV_FS_DISK_SIZE+0x03
		MCALL C5_OUT_NUM32
		LDI TEMP,0x04
		MCALL C5_OUT_STR_EL
		LDI TEMP_H,high(DRV_DISK_BLOCK_SIZE)
		LDI TEMP_L,low(DRV_DISK_BLOCK_SIZE)
		MCALL C5_OUT_NUM16
		LDI TEMP,0x05
		MCALL C5_OUT_STR_EL
_DRV_FS_DISK_INIT__LOG_SKIP:
	.endif
.endif

	LDI TEMP,DRV_RESULT_OK
	CPI ACCUM,0x01
	BRNE PC+0x02
	LDI TEMP,DRV_RESULT_INVALID_RESOURCE
_DRV_FS_DISK_INIT__END:

	POP LOOP_CNTR
	POP ACCUM
	POP FLAGS
	POP_Z
	POP_X
	RET

;--------------------------------------------------------
_DRV_FS_CHECK_DISK:
;--------------------------------------------------------
;IN: Y-адрес на переменные, X-адрес буфера
;OUT: TEMP-DEV_RESULT_...
;--------------------------------------------------------
	PUSH_T32
	PUSH FLAGS

	PUSH_Y
	ADIW YL,_DRV_FS_DISK_TIME
	MCALL C5_UPTIME_DELTA
	POP_Y
	OR TEMP_EH,TEMP_EL
	OR TEMP_EH,TEMP_H
	OR TEMP_EH,TEMP_L
	BRNE _DRV_FS_CHECK_OTIME__EXPIRED
	CPI TEMP,250														;0.5ms
	LDI TEMP,DRV_RESULT_OK
	BRCS _DRV_FS_CHECK_OTIME__END
_DRV_FS_CHECK_OTIME__EXPIRED:
	LDD TEMP,Y+_DRV_FS_DISK_DRV_ID
	LDI FLAGS,DRV_DISK_OP_GET_ID
	MCALL C5_EXEC
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_CHECK_OTIME__END
	MOVW ZL,YL
	ADIW ZL,_DRV_FS_DISK_ID
	MCALL RAM_CP
	BREQ _DRV_FS_CHECK_OTIME__END
	MCALL _DRV_FS_DISK_INIT
	CPI TEMP,DRV_RESULT_OK
	BRNE _DRV_FS_CHECK_OTIME__END
	LDI TEMP,DRV_RESULT_GONE
_DRV_FS_CHECK_OTIME__END:

	POP FLAGS
	POP_T32
	RET
.endif
