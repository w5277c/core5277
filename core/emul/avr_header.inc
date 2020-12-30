;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;29.12.2020	w5277c@gmail.com			Очень грубый набросок, вероятно будет сильно переделан или даже удален.
;-----------------------------------------------------------------------------------------------------------------------
	;---SPECIFY-DEVICE---
	.DEVICE ATmega328P

	#pragma AVRPART MEMORY PROG_FLASH 32768
	#pragma AVRPART MEMORY INT_SRAM SIZE 2048
	#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100
	#pragma AVRPART MEMORY EEPROM 1024
.INCLUDE "./devices/_common.inc"


	.SET	C5_EXEC					= 0x8001
	.SET	C5_WAIT_2MS				= 0x8002
	.SET	C5_DISPATCHER_LOCK	= 0x8003
	.SET	C5_DISPATCHER_UNLOCK	= 0x8004
	.SET	C5_SUSPEND				= 0x8005
	.SET	C5_LOG_CHAR				= 0x8006
	.SET	C5_LOG_BYTE				= 0x8007
	.SET	C5_LOG_BYTES			= 0x8008
	.SET	C5_LOG_NUMx8			= 0x8009
	.SET	C5_LOG_ROMSTR			= 0x800a
	.SET	C5_LOG_SDNF				= 0x800b
	.SET	C5_LOG_STR				= 0x800c
	.SET	C5_LOG_WORD				= 0x800d
	.SET	C5_LOG_BYTES			= 0x800e
