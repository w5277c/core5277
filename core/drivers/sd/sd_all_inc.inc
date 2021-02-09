;-----------------------------------------------------------------------------------------------------------------------
;Файл распространяется под лицензией GPL-3.0-or-later, https://www.gnu.org/licenses/gpl-3.0.txt
;-----------------------------------------------------------------------------------------------------------------------
;09.02.2021  w5277c@gmail.com			Начало
;-----------------------------------------------------------------------------------------------------------------------

.include	"./core/drivers/sd/sd_csd_block_info.inc"
.include	"./core/drivers/sd/sd_erase_blocks.inc"
.include	"./core/drivers/sd/sd_gen_cmd.inc"
.include	"./core/drivers/sd/sd_get_block.inc"
.include	"./core/drivers/sd/sd_get_cid.inc"
.include	"./core/drivers/sd/sd_get_csd.inc"
.include	"./core/drivers/sd/sd_get_dataresp_token.inc"
.include	"./core/drivers/sd/sd_get_ocr.inc"
.include	"./core/drivers/sd/sd_get_r1.inc"
.include	"./core/drivers/sd/sd_get_r1b5.inc"
.include	"./core/drivers/sd/sd_get_r2.inc"
.include	"./core/drivers/sd/sd_get_r3r7.inc"
.include	"./core/drivers/sd/sd_get_status.inc"
.include	"./core/drivers/sd/sd_log_cid.inc"
.include	"./core/drivers/sd/sd_log_csd.inc"
.include	"./core/drivers/sd/sd_log_ocr.inc"
.include	"./core/drivers/sd/sd_read_block.inc"
.include	"./core/drivers/sd/sd_write_block.inc"
