;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.12.2020  w5277c@gmail.com        Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_I2C
.else
.set DEF__DRV_I2C = 1

;---CONSTANTS--------------------------------------------
	;---FREQ---
	.EQU	DRV_I2C_FREQ_50KHZ					= 0x98			;for 50kHz  ---(16000000/50000  - 16)/(2*4^0) = 152
	.EQU	DRV_I2C_FREQ_100KHZ					= 0x48			;for 100kHz ---(16000000/100000 - 16)/(2*4^0) = 72
	.EQU	DRV_I2C_FREQ_200KHZ					= 0x20			;for 200kHz ---(16000000/200000 - 16)/(2*4^0) = 32
	.EQU	DRV_I2C_FREQ_400KHZ					= 0x0c			;for 400kHz ---(16000000/400000 - 16)/(2*4^0) = 12
	;---DRIVER-STATES---
	.EQU	DRV_I2C_ST_DONE						= 0x00
	.EQU	DRV_I2C_ST_TR							= 0x01
	.EQU	DRV_I2C_ST_RC							= 0x02
	;---TWI-STATES---
	.EQU	TWI_MTR_START							= 0x08			;M-master,S-slave,T-transfer,R-receive
	.EQU	TWI_MTR_NEXT_START					= 0x10
	.EQU	TWI_MT_SLA_W_ACK						= 0x18
	.EQU	TWI_MT_SLA_W_NACK						= 0x20
	.EQU	TWI_MT_DATA_ACK						= 0x28
	.EQU	TWI_MT_DATA_NACK						= 0x30
	.EQU	TWI_MTR_ARBIT_LOST					= 0x38
	.EQU	TWI_MR_SLA_R_ACK						= 0x40
	.EQU	TWI_MR_SLA_R_NACK						= 0x48
	.EQU	TWI_MR_ACK								= 0x50
	.EQU	TWI_MR_NACK								= 0x58
	;---STATES---
	.EQU	DRV_I2C_STATE_TR						= 0x01
	.EQU	DRV_I2C_STATE_RC						= 0x02
	.EQU	DRV_I2C_STATE_OK						= 0x80
	.EQU	DRV_I2C_STATE_ERROR					= 0x81
	;---UNIVERSAL-OFFSETS---
	.EQU	DRV_I2C_SDA_PORT						= 0x00			;1B-SDA порт
	.EQU	DRV_I2C_SCL_PORT						= 0x01			;1B-SCL порт

.endif