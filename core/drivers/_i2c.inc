;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;12.12.2020	w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------
.ifdef DEF__DRV_I2C
.else
.set DEF__DRV_I2C = 1

;---CONSTANTS--------------------------------------------
	;---FREQ---
	.EQU	DRV_I2C_FREQ_10KHZ					= (CORE_FREQ*1000/10-16)/2
	.EQU	DRV_I2C_FREQ_20KHZ					= (CORE_FREQ*1000/20-16)/2
	.EQU	DRV_I2C_FREQ_50KHZ					= (CORE_FREQ*1000/50-16)/2
.if CORE_FREQ>1
	.EQU	DRV_I2C_FREQ_100KHZ					= (CORE_FREQ*1000/100-16)/2
.else
	.message "Unsupported I2C freq. 100KHz, low CPU freq."
.endif
.if CORE_FREQ>3
	.EQU	DRV_I2C_FREQ_200KHZ					= (CORE_FREQ*1000/200-16)/2
.else
	.message "Unsupported I2C freq. 200KHz, low CPU freq."
.endif
.if CORE_FREQ>7
	.EQU	DRV_I2C_FREQ_400KHZ					= (CORE_FREQ*1000/400-16)/2
.else
	.message "Unsupported I2C freq. 400KHz, low CPU freq."
.endif
	;---DRIVER-STATES---
	.EQU	DRV_I2C_ST_TR							= 0x01
	.EQU	DRV_I2C_ST_RC							= 0x02
	.EQU	DRV_I2C_ST_OK							= DRV_RESULT_OK
	.EQU	DRV_I2C_ST_ERROR						= DRV_RESULT_ERROR
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
	;---
	.EQU	TWI_SR_SLA_W							= 0x60
	.EQU	TWI_SR_SLA_W_AL						= 0x68
	.EQU	TWI_SR_SLA_R							= 0xA8
	.EQU	TWI_SR_SLA_R_AL						= 0xB0
	.EQU	TWI_SR_GC								= 0x70
	.EQU	TWI_SR_GC_AL							= 0x78
	.EQU	TWI_SR_RC_ACK							= 0x80
	.EQU	TWI_SR_RC_NACK							= 0x88
	.EQU	TWI_SR_GC_RC_ACK						= 0x90
	.EQU	TWI_SR_GC_RC_NACK						= 0x98
	.EQU	TWI_SR_TR_ACK							= 0xB8

	.EQU	TWI_SR_RC_STOP							= 0xA0
	.EQU	TWI_SR_TR_STOP							= 0xC0
	;---STATES---
	.EQU	DRV_I2C_STATE_TR						= 0x01
	.EQU	DRV_I2C_STATE_RC						= 0x02
	;---UNIVERSAL-OFFSETS---
	.EQU	DRV_I2C_SDA_PORT						= 0x00			;1B-SDA порт
	.EQU	DRV_I2C_SCL_PORT						= 0x01			;1B-SCL порт

.endif
