;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;05.01.2023	konstantin@5277.ru			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_DISK
.else
.set DEF__DRV_DISK = 1
	.SET	DRV_DISK_BLOCK_SIZE							= 0x200	;Размер блока по умолчанию
	
	;---OPERATIONS---
	.EQU	DRV_DISK_OP_INIT								= 0x00	;Подготовить диск к работе
																			;IN: X-адрес на буфер

	.EQU	DRV_DISK_OP_GET_ID							= 0x01	;Считать идентификатор диска
																			;OUT:???
	.EQU	DRV_DISK_OP_GET_SIZE							= 0x02	;Считать размер в блоках
																			;OUT:TEMP_EH/EL/H/L-кол-во блоков
	;---
	.EQU	DRV_DISK_OP_READ								= 0x08	;Считать блок данных
																			;IN:X-адрес на буфер,
																			;TEMP_EH/EL/H/L-номер блока

	.EQU	DRV_DISK_OP_WRITE								= 0x09	;Записать блок данных
																			;IN:X-адрес на буфер,
																			;TEMP_EH/EL/H/L-номер блока

.endif