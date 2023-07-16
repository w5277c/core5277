;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;04.08.2022	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_EM
.else
.set DEF__DRV_EM = 1

	;---OPERATIONS---
	.EQU	MEM_READ_BYTE							= 0x01			;TEMP
	.EQU	MEM_READ_SHORT							= 0x02			;TEMP_H,TEML_L
	.EQU	MEM_READ_INT							= 0x03			;TEMP_EH,TEMP_EL,TEMP_H,TEMP_L
	.EQU	MEM_READ_LONG							= 0x04			;XH,XL,YH,YL,TEMP_EH,TEMP_EL,TEMP_H,TEMP_L
	.EQU	MEM_READ_BLOCK							= 0x0f			;X-addr,LOOP_NCTR-block size
	.EQU	MEM_WRITE_BYTE							= 0x11
	.EQU	MEM_WRITE_SHORT						= 0x12
	.EQU	MEM_WRITE_INT							= 0x13
	.EQU	MEM_WRITE_LONG							= 0x14
	.EQU	MEM_WRITE_BLOCK						= 0x1f			;X-addr,LOOP_NCTR-block size
;	.EQU	MEM_GET_SIZE							= 0x41			;TEMP_EH,TEMP_EL,TEMP_H,TEMP_L,TEMP


	.EQU	DRV_MEM_SET_PAGE						= 0x01			;IE WRITE CURRENT PAGE AND READ NEW PAGE IN BUFFER PAGE
	.EQU	DRV_MEM_GET_PAGESIZE					= 0x02
	.EQU	DRV_MEM_GET_PAGEQNT					= 0x03
	.EQU	DRV_MEM_FLUSH_PAGE					= 0x04			;IE WRITE CURRENT PAGE
.endif
