;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;11.06.2021	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_VIDEO
.else
.set DEF__DRV_VIDEO = 1

	.SET	DRV_VIDEO_TXT_MODES							= 0x00	;Поддерживаемые текстовые режимы
	.SET	DRV_VIDEO_GRX_MODES							= 0x00	;Поддерживаемые графические режимы
	.SET	DRV_VIDEO_BUFFER_SIZE						= 0x00	;Размер видео буфера
	;---TXT-MODES(BITS)---
	.EQU	DRV_VIDEO_MODE_TXT1B							= 0x00	;Текст, монохромный
;	.EQU	DRV_VIDEO_MODE_TXT3B							= 0x01	;Текст, 8 цветов
;	.EQU	DRV_VIDEO_MODE_TXT4B							= 0x02	;Текст, 16 цветов (4б, 1б на каждый цвет, 1 бит яркости)
	;---GRX-MODES(BITS)---
	.EQU	DRV_VIDEO_MODE_GRX1B							= 0x00	;Графический, монохромный (1б)
	.EQU	DRV_VIDEO_MODE_GRX3B							= 0x01	;Графический, 8цветов (3б)
	.EQU	DRV_VIDEO_MODE_GRX4B							= 0x02	;Графический, 16 цветов (4б, 1б на каждый цвет, 1 бит яркости)
	.EQU	DRV_VIDEO_MODE_GRX9B							= 0x03	;Графический, 512 цветов (9б, 3б на каждый цвет)
	.EQU	DRV_VIDEO_MODE_GRX12B						= 0x04	;Графический, 4К цветов (12б, 4б на каждый цвет)
	.EQU	DRV_VIDEO_MODE_GRX15B						= 0x05	;Графический, 32К цветов (15б, 5б на каждый цвет)
	.EQU	DRV_VIDEO_MODE_GRX24B						= 0x06	;Графический, 16М цветов (24б, 8б на каждый цвет)
	.EQU	DRV_VIDEO_MODE_GRX30B						= 0x07	;Графический, 1Г цветов (30б, 10б на каждый цвет)

	;---OPERATIONS---
	.EQU	DRV_VIDEO_OP_INIT								= 0x00	;Подготовить экран к работе
	.EQU	DRV_VIDEO_OP_SET_PARAM						= 0x01	;Записываем параметр
	.EQU	DRV_VIDEO_OP_GET_PARAM						= 0x02	;Считываем параметр
	.EQU	DRV_VIDEO_OP_GET_MODES						= 0x03	;Получить поддерживаемые текстовые и графические режимы
	.EQU	DRV_VIDEO_OP_GET_MODE						= 0x04	;Получить текущий графический или текстовый режим
	.EQU	DRV_VIDEO_OP_SET_MODE						= 0x05	;Задать текущий графический или текстовый режим
	.EQU	DRV_VIDEO_OP_GET_SIZE						= 0x06	;Получить размер экрана
	.EQU	DRV_VIDEO_OP_SET_XY							= 0x07	;Задать текущую координату
	.EQU	DRV_VIDEO_OP_SET_SYMBOL						= 0x08	;Вывести символ
	.EQU	DRV_VIDEO_OP_SET_TEXT						= 0x09	;Вывести текст с текущей позиции

	;---PARAMETERS---
	;TODO

.endif
