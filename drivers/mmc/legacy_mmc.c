/*
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for	list of people who contributed to this
 * project.
 *
 * This	program	is free	software; you can redistribute it and/or
 * modify it under the terms of	the GNU General Public License as
 * published by	the Free Software Foundation; either version 2 of
 * the License,	or (at your option) any later version.
 *
 * This	program	is distributed in the hope that	it will	be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59	Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <asm/errno.h>
#include <part.h>
#include <asm/arch/sdhc.h>
#include <linux/types.h>
#include <linux/list.h>

#define CARD_SUPPORT_BYTE_MODE			(0)
#define CARD_SUPPORT_SECT_MODE			(1)

#define RETRY_TIMEOUT					(10)

static int dbg = 0;
#define DBG(fmt,args...)	if (dbg) printf (fmt"\n" ,##args)

#ifdef CONFIG_CMD_FAT
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
#endif

static block_dev_desc_t	mmc_dev;

block_dev_desc_t *mmc_get_dev(int dev)
{
	return (block_dev_desc_t *)&mmc_dev;
}

static struct list_head mmc_devices;
static int cur_dev_num = -1;

/*
 * FIXME needs to read cid and csd info	to determine block size
 * and other parameters
 */
static int mmc_ready;
static struct mmc_card g_Card;

enum states {
    IDLE,
    READY,
    IDENT,
    STBY,
    TRAN,
    DATA,
    RCV,
    PRG,
    DIS,
    BTST,
    SLP
};

static u32 mmc_cmd(struct mmc_command *cmd, u32 opcode,
		   u32 arg, u32 xfer, u32 fmt, u32 write,
		   u32 crc, u32 cmd_check_en);
static u32 mmc_acmd(struct mmc_command *cmd, u32 opcode,
		u32 arg, u32 xfer, u32 fmt, u32 write,
		u32 crc, u32 cmd_check_en);
static s32 mmc_decode_cid(struct mmc_card *card);
static s32 mmc_decode_csd(struct mmc_card *card);
static s32 sd_voltage_validation(void);
static s32 mmc_voltage_validation(void);
static s32 mmc_send_cid(struct mmc_card *card);
static s32 mmc_send_csd(struct mmc_card *card, u32 u32CardRCA);
static s32 mmc_select_card(u32 card_rca);
static s32 mmcsd_check_status(u32 card_rca, u32 timeout,
		u32 card_state, u32 status_bit);
static s32 mmc_send_relative_addr(u32 *u32CardRCA);
static s32 mmc_decode_scr(struct mmc_card *card);
static s32 mmc_send_scr(struct mmc_card *card);
static s32 mmc_set_relative_addr(u32 u32CardRCA);
static s32 mmc_app_set_bus_width(s32 width);
static s32 mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value);
static s32 mmc_sd_switch(struct mmc_card *card, s32 mode, s32 group,
						u8 value, u8 *resp);

static u32 mmc_cmd(struct mmc_command *cmd, u32 opcode,
			u32 arg, u32 xfer, u32 fmt,
		   u32 write, u32 crc, u32 cmd_check_en)
{
	struct mmc_command *pCmd = cmd;

	pCmd->cmd.command = opcode;
	pCmd->cmd.arg =	arg;
	pCmd->cmd.data_transfer	= xfer;
	pCmd->cmd.response_format = pCmd->resp.format = fmt;
	pCmd->cmd.data_present = write;
	pCmd->cmd.crc_check = crc;
	pCmd->cmd.cmdindex_check = cmd_check_en;

	if (MMC_READ_MULTIPLE_BLOCK == opcode || \
		MMC_WRITE_MULTIPLE_BLOCK == opcode) {
		pCmd->cmd.block_count_enable_check = ENABLE;
		pCmd->cmd.multi_single_block = MULTIPLE;
	} else {
		pCmd->cmd.block_count_enable_check = DISABLE;
		pCmd->cmd.multi_single_block = SINGLE;
	}

	if (interface_send_cmd_wait_resp(&(pCmd->cmd))) {
		DBG("interface_send_cmd_wait_resp Failed!");
		return EPERM;
	}

	interface_read_response(&(pCmd->resp));

	return 0;
}

static u32 mmc_acmd(struct mmc_command *cmd, u32 opcode,
			u32 arg, u32 xfer, u32 fmt, u32 write,
			u32 crc, u32 cmd_check_en)
{
	struct mmc_command *pCmd = cmd;
	struct mmc_command stAPCmd;

	memset(&stAPCmd, 0, sizeof(struct mmc_command));

	/* Send MMC_APP_CMD first to use ACMD */
	stAPCmd.cmd.command = MMC_APP_CMD;
	stAPCmd.cmd.arg = (g_Card.rca << 16);
	stAPCmd.cmd.data_transfer = READ;
	stAPCmd.cmd.response_format = stAPCmd.resp.format = RESPONSE_48;
	stAPCmd.cmd.data_present = DATA_PRESENT_NONE;
	stAPCmd.cmd.crc_check = ENABLE;
	stAPCmd.cmd.cmdindex_check = ENABLE;

	if (interface_send_cmd_wait_resp(&(stAPCmd.cmd))) {
		DBG("Send MMC_APP_CMD Failed! :(");
		return EPERM;
	}

	pCmd->cmd.command = opcode;
	pCmd->cmd.arg =	arg;
	pCmd->cmd.data_transfer	= xfer;
	pCmd->cmd.response_format = pCmd->resp.format = fmt;
	pCmd->cmd.data_present = write;
	pCmd->cmd.crc_check = crc;
	pCmd->cmd.cmdindex_check = cmd_check_en;

	if (interface_send_cmd_wait_resp(&(pCmd->cmd))) {
		DBG("interface_send_cmd_wait_resp Failed!, :(");
		return EPERM;
	}

	interface_read_response(&(pCmd->resp));

	return 0;
}

int
/****************************************************/
mmc_read(int dev_num, u32 src, uchar * dst, u32 size)
/****************************************************/
{
	struct mmc_command stCmd;
	u32 u32Offset = src;
	s32 s32Rslt = EPERM;
	s32 s32ReadRslt	= 0;
	u32 u32BlkLen = BLK_LEN;
	u32 u32MultiBlkNum = 0;

	if (!mmc_ready)	{
		printf("Please initial the Card first\n");
		return EPERM;
	}

	if (size == 0)
		return 0;

	DBG("Entry: mmc_read");

	DBG("src:%08x	dst:%08x size:%d", (unsigned int) src, (unsigned int) dst, size);

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	if (g_Card.state & MMC_STATE_BLOCKADDR) {
		u32BlkLen = 1;
		u32Offset /= BLK_LEN;
	}

	u32MultiBlkNum = (size % BLK_LEN) ? ((size / BLK_LEN) + 1) \
					 : (size / BLK_LEN);

	if (g_Card.cid.manfid == MANFID_SAMSUNG) {

	    if (u32MultiBlkNum > 1) {
		DBG("set block count: %d", u32MultiBlkNum);
		
		s32Rslt = mmc_cmd(&stCmd,
				  MMC_SET_BLOCK_COUNT,
				  u32MultiBlkNum,
				  READ,
				  RESPONSE_48,
				  DATA_PRESENT_NONE,
				  ENABLE,
				  ENABLE);
		if (s32Rslt) {
		    DBG("Send MMC_SET_BLOCK_COUNT Failed! :(\n");
		    return EPERM;
		}

		memset(&stCmd, 0, sizeof(struct mmc_command));
	    }

	    stCmd.cmd.acmd12_enable_check = DISABLE;

	} else {

	    /* enable auto CMD12 if not in predefined mode */
	    stCmd.cmd.acmd12_enable_check = ENABLE;
	}
	
	stCmd.cmd.dma_address = (u32) dst;
	interface_config_block_info(BLK_LEN, u32MultiBlkNum);

	s32Rslt	= mmc_cmd(&stCmd,
			((u32MultiBlkNum > 1) ? MMC_READ_MULTIPLE_BLOCK : MMC_READ_SINGLE_BLOCK),
			u32Offset,
			READ,
			RESPONSE_48,
			DATA_PRESENT,
			ENABLE,
			ENABLE);

	if (s32Rslt) {
		DBG("Send MMC_READ_BLOCK Failed! :(\n");
		return EPERM;
	}

#ifndef CONFIG_FSL_ESDHC_DMA
	DBG("  read begin\n");
	s32Rslt	= interface_data_read((u32 *)dst, BLK_LEN * u32MultiBlkNum);

	if (s32Rslt) {
		DBG("interface_data_read Failed! :(\n");
		return EPERM;
	}

	if (u32MultiBlkNum > 1) {
		s32Rslt	= mmc_cmd(&stCmd,
				MMC_STOP_TRANSMISSION,
				0,
				READ,
				RESPONSE_48,
				DATA_PRESENT_NONE,
				ENABLE,
				ENABLE);

		if (s32Rslt) {
			DBG("Send MMC_STOP_TRANSMISSION Failed! :(\n");
			return EPERM;
		}
	}
#endif

#if 0
	// BEN DEBUG
	{
	    int i;
	    for (i = 0; i < 512; i++) {
		if (!(i%16)) printf("\n0x%04x: ", i);
		printf("%02x ", dst[i]);
	    }
	    printf("\n");
	}
#endif
	DBG("mmc_read	succeed! :)");

	DBG("Exit: mmc_read");

	return s32ReadRslt;
}

int
/****************************************************/
mmc_write(int dev_num, uchar * src, u32 dst, u32 size)
/****************************************************/
{
	struct mmc_command stCmd;
	u32 u32Offset = dst;
	s32 s32Rslt = EPERM;
	s32 s32WriteRslt = 0;
	u32 u32BlkLen = BLK_LEN;
	u32 u32MultiBlkNum = 0;

	DBG("Entry: mmc_write");

	DBG("src:%08x	dst:%08x size:%d", (unsigned int) src, (unsigned int) dst, size);

	if (!mmc_ready)	{
		printf("Please initial the Card first\n");
		return -1;
	}

	if (size == 0)
		return 0;

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	if (g_Card.state & MMC_STATE_BLOCKADDR) {
		u32BlkLen = 1;
		u32Offset /= BLK_LEN;
	}

	u32MultiBlkNum = (size % BLK_LEN) ? ((size / BLK_LEN) + 1) \
					 : (size / BLK_LEN);

	if (g_Card.cid.manfid == MANFID_SAMSUNG) {

	    if (u32MultiBlkNum > 1) {

		s32Rslt = mmc_cmd(&stCmd,
				  MMC_SET_BLOCK_COUNT,
				  u32MultiBlkNum,
				  READ,
				  RESPONSE_48,
				  DATA_PRESENT_NONE,
				  ENABLE,
				  ENABLE);
		if (s32Rslt) {
		    DBG("Send MMC_SET_BLOCK_COUNT Failed! :(\n");
		    return EPERM;
		}

		memset(&stCmd, 0, sizeof(struct mmc_command));
	    }
	
	    stCmd.cmd.acmd12_enable_check = DISABLE;

	} else {

	    /* enable auto CMD12 if not in predefined mode */
	    stCmd.cmd.acmd12_enable_check = ENABLE;
	}

	stCmd.cmd.dma_address = (u32) src;
	interface_config_block_info(BLK_LEN, u32MultiBlkNum);

	s32Rslt	= mmc_cmd(&stCmd,
			((u32MultiBlkNum > 1) ? MMC_WRITE_MULTIPLE_BLOCK : MMC_WRITE_BLOCK),
			u32Offset,
			WRITE,
			RESPONSE_48,
			DATA_PRESENT,
			ENABLE,
			ENABLE);

	if (s32Rslt) {
		DBG("Send MMC_WRITE_BLOCK Failed! :(");
		return EPERM;
	}

#ifndef CONFIG_FSL_ESDHC_DMA
	s32Rslt	= interface_data_write((u32 *)src,
					BLK_LEN * u32MultiBlkNum);

	if (s32Rslt) {
		DBG("interface_data_write Failed! :(");
		return EPERM;
	}

	if (u32MultiBlkNum > 1) {
		s32Rslt	= mmc_cmd(&stCmd,
				MMC_STOP_TRANSMISSION,
				0,
				READ,
				RESPONSE_48,
				DATA_PRESENT_NONE,
				ENABLE,
				ENABLE);

		if (s32Rslt) {
			DBG("Send MMC_STOP_TRANSMISSION Failed! :(");
			return EPERM;
		}
	}
#endif

	/* wait until data is actually written */
	if (mmcsd_check_status(g_Card.rca, 500, TRAN, R1_ERROR)) {
	    DBG("Can't wait for TRAN state! :(\n");
	    return EPERM;
	}

	DBG("mmc_write succeed! :)");

	DBG("Exit: mmc_write");

	return s32WriteRslt;
}

ulong
/****************************************************/
mmc_bread(int dev, ulong blknr, lbaint_t blkcnt, void *dst)
/****************************************************/
{
	int mmc_block_size = BLK_LEN;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	if (mmc_read(dev, src, (uchar *)dst, blkcnt * mmc_block_size))
		return 0;
	else
		return blkcnt;
}

ulong
/****************************************************/
mmc_bwrite(int dev, ulong blknr, lbaint_t blkcnt, const void *src)
/****************************************************/
{
	int mmc_block_size = BLK_LEN;
	ulong dst = blknr * mmc_block_size + CFG_MMC_BASE;

	if (mmc_write(dev, (uchar *)src, dst, blkcnt * mmc_block_size))
		return 0;
	else
		return blkcnt;
}

#define	UNSTUFF_BITS(resp, start, size)					\
	({								\
		const int __size = size;				\
		const uint32_t __mask =	(__size	< 32 ? 1 << __size : 0) - 1; \
		const int32_t __off = 3 - ((start) / 32); \
		const int32_t __shft = (start) & 31; \
		uint32_t __res;						\
									\
		__res =	resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res &	__mask;						\
	})

static const unsigned int tran_exp[] = {
	10000, 100000, 1000000, 10000000,
	0, 0, 0, 0
};

static const unsigned char tran_mant[] = {
	0,  10, 12, 13, 15, 20, 25, 30,
	35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0,  10, 12, 13, 15, 20, 25, 30,
	35, 40, 45, 50, 55, 60, 70, 80,
};

static s32 mmc_set_blk_len(u32 len)
{
	s32 s32Rslt = 0;
	struct mmc_command stCmd;

	DBG("Entry: mmc_set_blk_len");

	memset(&stCmd, 0, sizeof(struct mmc_command));

	s32Rslt	= mmc_cmd(&stCmd,
			MMC_SET_BLOCKLEN,
			BLK_LEN,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);

	if (s32Rslt) {
		DBG("Send MMC_SET_BLOCKLEN Failed! :(");
		return EPERM;
	}

	DBG("Exit: mmc_set_blk_len");

	return s32Rslt;
}

/*
 * Given the decoded CSD structure, decode the raw CID to our CID structure.
 */
static s32 mmc_decode_cid(struct mmc_card *card)
{
	u32 *resp = card->raw_cid;

	DBG("Entry: mmc_decode_cid");

	if (!card) {
		DBG("NULL card pointer!");
		return EPERM;
	}

	memset(&card->cid, 0, sizeof(struct mmc_cid));

	switch (card->type) {
	case MMC_TYPE_MMC:
		DBG("MMC Card!");
		/*
		* The selection	of the format here is based upon published
		* specs	from sandisk and from what people have reported.
		*/
		switch (card->csd.mmca_vsn) {
		case 0:	/* MMC v1.0 - v1.2 */
		case 1:	/* MMC v1.4	*/
			card->cid.manfid	= \
				UNSTUFF_BITS(resp, 104, 24);
			card->cid.prod_name[0]	= \
				UNSTUFF_BITS(resp, 96, 8);
			card->cid.prod_name[1]	= \
				UNSTUFF_BITS(resp, 88, 8);
			card->cid.prod_name[2]	= \
				UNSTUFF_BITS(resp, 80, 8);
			card->cid.prod_name[3]	= \
				UNSTUFF_BITS(resp, 72, 8);
			card->cid.prod_name[4]	= \
				UNSTUFF_BITS(resp, 64, 8);
			card->cid.prod_name[5]	= \
				UNSTUFF_BITS(resp, 56, 8);
			card->cid.prod_name[6]	= \
				UNSTUFF_BITS(resp, 48, 8);
			card->cid.hwrev	= UNSTUFF_BITS(resp, 44, 4);
			card->cid.fwrev	= UNSTUFF_BITS(resp, 40, 4);
			card->cid.serial = UNSTUFF_BITS(resp, 16, 24);
			card->cid.month	= UNSTUFF_BITS(resp, 12, 4);
			card->cid.year	= \
				UNSTUFF_BITS(resp, 8,	4) + 1997;

			sprintf((char *)mmc_dev.vendor,
				"Man %08x \"%c%c%c%c%c%c%c\" Date %02u/%04u",
				card->cid.manfid,
				card->cid.prod_name[0],
				card->cid.prod_name[1],
				card->cid.prod_name[2],
				card->cid.prod_name[3],
				card->cid.prod_name[4],
				card->cid.prod_name[5],
				card->cid.prod_name[6],
				card->cid.month,
				card->cid.year);
			sprintf((char *)mmc_dev.revision, "%d.%d",
					card->cid.hwrev,
					card->cid.fwrev);
			sprintf((char *)mmc_dev.product, "%u",
					card->cid.serial);
			break;
		case 2:	/* MMC v2.0 - v2.2 */
		case 3:	/* MMC v3.1 - v3.3 */
		case 4:	/* MMC v4 */
			card->cid.manfid = UNSTUFF_BITS(resp, 120, 8);
			card->cid.oemid.mmc_id	= \
				UNSTUFF_BITS(resp, 104, 16);
			card->cid.prod_name[0]	= \
				UNSTUFF_BITS(resp, 96, 8);
			card->cid.prod_name[1]	= \
				UNSTUFF_BITS(resp, 88, 8);
			card->cid.prod_name[2]	= \
				UNSTUFF_BITS(resp, 80, 8);
			card->cid.prod_name[3]	= \
				UNSTUFF_BITS(resp, 72, 8);
			card->cid.prod_name[4]	= \
				UNSTUFF_BITS(resp, 64, 8);
			card->cid.prod_name[5]	= \
				UNSTUFF_BITS(resp, 56, 8);
			card->cid.serial = UNSTUFF_BITS(resp, 16, 32);
			card->cid.month	 = UNSTUFF_BITS(resp, 12, 4);
			card->cid.year	 = \
				UNSTUFF_BITS(resp, 8, 4) + 1997;

			sprintf((char *)mmc_dev.vendor,
			"Man %02x OEM %04x \"%c%c%c%c%c%c\" Date %02u/%04u",
				card->cid.manfid,
				card->cid.oemid.mmc_id,
				card->cid.prod_name[0],
				card->cid.prod_name[1],
				card->cid.prod_name[2],
				card->cid.prod_name[3],
				card->cid.prod_name[4],
				card->cid.prod_name[5],
				card->cid.month,
				card->cid.year);
			sprintf((char *)mmc_dev.product, "%u",
					card->cid.serial);
			sprintf((char *)mmc_dev.revision, "N/A");
			break;
		default:
			printf("MMC card has unknown MMCA version %d\n",
				card->csd.mmca_vsn);
			return EPERM;
		}
		break;

	case MMC_TYPE_SD:
		DBG("SD Card!");
		card->cid.manfid	= UNSTUFF_BITS(resp, 120, 8);
		card->cid.oemid.sd_id[0] = UNSTUFF_BITS(resp, 112, 8);
		card->cid.oemid.sd_id[1] = UNSTUFF_BITS(resp, 104, 8);
		card->cid.prod_name[0]	= UNSTUFF_BITS(resp, 96, 8);
		card->cid.prod_name[1]	= UNSTUFF_BITS(resp, 88, 8);
		card->cid.prod_name[2]	= UNSTUFF_BITS(resp, 80, 8);
		card->cid.prod_name[3]	= UNSTUFF_BITS(resp, 72, 8);
		card->cid.prod_name[4]	= UNSTUFF_BITS(resp, 64, 8);
		card->cid.hwrev		= UNSTUFF_BITS(resp, 60, 4);
		card->cid.fwrev		= UNSTUFF_BITS(resp, 56, 4);
		card->cid.serial	= UNSTUFF_BITS(resp, 24, 32);
		card->cid.year		= UNSTUFF_BITS(resp, 12, 8);
		card->cid.month		= UNSTUFF_BITS(resp, 8,	4);
		card->cid.year += 2000;	/* SD cards year offset */

		sprintf((char *)mmc_dev.vendor,
			"Man %02x OEM %c%c \"%c%c%c%c%c\" Date %02u/%04u",
			card->cid.manfid,
			card->cid.oemid.sd_id[0],
			card->cid.oemid.sd_id[1],
			card->cid.prod_name[0],
			card->cid.prod_name[1],
			card->cid.prod_name[2],
			card->cid.prod_name[3],
			card->cid.prod_name[4],
			card->cid.month,
			card->cid.year);
		sprintf((char *)mmc_dev.revision, "%d.%d",
				card->cid.hwrev, card->cid.fwrev);
		sprintf((char *)mmc_dev.product, "%u",
				card->cid.serial);
		break;

	default:
		printf("unknown card type!");
		return EPERM;
	}

	printf("%s card.\nVendor: %s\nProduct: %s\nRevision: %s\n",
	       (IF_TYPE_SD == mmc_dev.if_type) ? "SD" : "MMC", mmc_dev.vendor,
	       mmc_dev.product, mmc_dev.revision);

	DBG("Exit: mmc_decode_cid");

	return 0;
}

/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static s32 mmc_decode_csd(struct mmc_card *card)
{
	struct mmc_csd *csd = &card->csd;
	u32 e, m, csd_struct;
	u32 *resp = card->raw_csd;

	DBG("Entry: mmc_decode_csd");

	if (!card) {
		DBG("NULL card pointer!");
		return EPERM;
	}

	switch (card->type) {
	case MMC_TYPE_MMC:
		/*
		 * We only understand CSD structure v1.1 and v1.2.
		 * v1.2	has extra information in bits 15, 11 and 10.
		 */
		csd_struct = UNSTUFF_BITS(resp,	126, 2);
		if (csd_struct == 0) {
			printf("unrecognised CSD structure version %d\n",
				csd_struct);
			return EPERM;
		}

		csd->mmca_vsn	 = UNSTUFF_BITS(resp, 122, 4);
		m = UNSTUFF_BITS(resp, 115, 4);
		e = UNSTUFF_BITS(resp, 112, 3);
		csd->tacc_ns   = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		csd->tacc_clks = UNSTUFF_BITS(resp, 104, 8) * 100;

		m = UNSTUFF_BITS(resp, 99, 4);
		e = UNSTUFF_BITS(resp, 96, 3);
		csd->max_dtr  = tran_exp[e] * tran_mant[m];
		csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

		e =	UNSTUFF_BITS(resp, 47, 3);
		m =	UNSTUFF_BITS(resp, 62, 12);
		csd->capacity = (1 + m) << (e + 2);

		csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
		csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
		csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
		csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
		csd->r2w_factor	= UNSTUFF_BITS(resp, 26, 3);
		csd->write_blkbits = UNSTUFF_BITS(resp,	22, 4);
		csd->write_partial = UNSTUFF_BITS(resp,	21, 1);

		mmc_dev.if_type = IF_TYPE_MMC;

		mmc_dev.lba = csd->capacity;
		mmc_dev.blksz = 1 << csd->read_blkbits;
		mmc_dev.part_type = PART_TYPE_DOS;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = DEV_TYPE_HARDDISK;
		mmc_dev.removable = 0;
		mmc_dev.block_read  = mmc_bread;
		mmc_dev.block_write = mmc_bwrite;

		break;

	case MMC_TYPE_SD:
		csd_struct = UNSTUFF_BITS(resp,	126, 2);

		switch (csd_struct) {
		case 0:
			m = UNSTUFF_BITS(resp, 115, 4);
			e = UNSTUFF_BITS(resp, 112, 3);
			csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9)	/ 10;
			csd->tacc_clks = UNSTUFF_BITS(resp, 104, 8) * 100;

			m = UNSTUFF_BITS(resp, 99, 4);
			e = UNSTUFF_BITS(resp, 96, 3);
			csd->max_dtr = tran_exp[e] * tran_mant[m];
			csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

			e = UNSTUFF_BITS(resp, 47, 3);
			m = UNSTUFF_BITS(resp, 62, 12);
			csd->capacity = (1 + m) << (e + 2);

			csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
			csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
			csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
			csd->read_misalign = UNSTUFF_BITS(resp,	77, 1);
			csd->r2w_factor	= UNSTUFF_BITS(resp, 26, 3);
			csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
			csd->write_partial = UNSTUFF_BITS(resp,	21, 1);

			mmc_dev.if_type = IF_TYPE_SD;

			mmc_dev.lba = csd->capacity;
			mmc_dev.blksz = 1 << csd->read_blkbits;
			mmc_dev.part_type = PART_TYPE_DOS;
			mmc_dev.dev = 0;
			mmc_dev.lun = 0;
			mmc_dev.type = DEV_TYPE_HARDDISK;
			mmc_dev.removable = 0;
			mmc_dev.block_read  = mmc_bread;
			mmc_dev.block_write = mmc_bwrite;

			break;
		case 1:
			/*
			 * This	is a block-addressed SDHC card.	Most
			 * interesting fields are unused and have fixed
			 * values. To avoid getting tripped by buggy cards,
			 * we assume those fixed values	ourselves.
			 */
			mmc_card_set_blockaddr(card);

			csd->tacc_ns   = 0; /* Unused */
			csd->tacc_clks = 0; /* Unused */

			m = UNSTUFF_BITS(resp, 99, 4);
			e = UNSTUFF_BITS(resp, 96, 3);
			csd->max_dtr  = tran_exp[e] * tran_mant[m];
			csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

			m = UNSTUFF_BITS(resp, 48, 22);
			csd->capacity = (1 + m) << 10;

			csd->read_blkbits = 9;
			csd->read_partial = 0;
			csd->write_misalign = 0;
			csd->read_misalign = 0;
			csd->r2w_factor	= 4; /*	Unused */
			csd->write_blkbits = 9;
			csd->write_partial = 0;

			mmc_dev.if_type = IF_TYPE_SD;

			mmc_dev.lba = csd->capacity;
			mmc_dev.blksz = 512;
			mmc_dev.part_type = PART_TYPE_DOS;
			mmc_dev.dev = 0;
			mmc_dev.lun = 0;
			mmc_dev.type = DEV_TYPE_HARDDISK;
			mmc_dev.removable = 0;
			mmc_dev.block_read = mmc_bread;

			break;
		default:
			printf("unrecognised CSD structure version %d\n",
					csd_struct);
			return EPERM;
		}
		break;

	default:
		printf("unknown	card type!");
		return EPERM;
		}

	DBG("Exit: mmc_decode_csd");

	return 0;
}

/*
 * Do SD voltage validation.
 */
static s32 sd_voltage_validation(void)
{
	struct mmc_command stCmd;
	u32 u32OcrVal =	0;
	u32 u32VoltageValidation = EPERM;
	s32 s32Rslt	= EPERM;
	s32 s32Retries = 0;
	/* Supported arguments for CMD8 */
	const u32 sd_if_cmd_arg[SD_IF_CMD_ARG_COUNT] = {
		SD_IF_HV_COND_ARG,
		SD_IF_LV_COND_ARG };
	const u32 sd_ocr_value[SD_OCR_VALUE_COUNT] = {
		SD_OCR_VALUE_HV_HC,
		SD_OCR_VALUE_LV_HC,
		SD_OCR_VALUE_HV_LC };

	DBG("Entry: sd_voltage_validation\n");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	for (s32Retries = 0; s32Retries < SD_IF_CMD_ARG_COUNT; ++s32Retries) {
		/* Configure CMD55 for SD card */
		/* This	command	expects	defualt	RCA 0x0000 as argument.*/
		s32Rslt	= mmc_cmd(&stCmd,
				SD_SEND_IF_COND,
				sd_if_cmd_arg[s32Retries],
				READ,
				RESPONSE_48,
				DATA_PRESENT_NONE,
				ENABLE,
				ENABLE);

		if (!s32Rslt) {
			if (sd_if_cmd_arg[s32Retries] == \
			(stCmd.resp.cmd_rsp0 & sd_if_cmd_arg[s32Retries])) {
				u32OcrVal = sd_ocr_value[s32Retries];
			} else {
				u32OcrVal = 0;
			}
			break;
		}
	}

	if (s32Rslt) {
		DBG("Card is of SD-1.x spec with LC");
		u32OcrVal = SD_OCR_VALUE_HV_LC;
	}

	for (s32Retries = RETRY_TIMEOUT; s32Retries; --s32Retries) {
		/* Configure ACMD41	for	SD card	*/
		/* This	command	expects	operating voltage range	as argument.*/
		s32Rslt	= mmc_acmd(&stCmd,
				SD_APP_OP_COND,
				u32OcrVal,
				READ,
				RESPONSE_48,
				DATA_PRESENT_NONE,
				DISABLE,
				DISABLE);

		/* Issue ACMD41	to SD Memory card to determine OCR value */
		if (s32Rslt == EPERM) {
			DBG("Send SD_APP_OP_COND Failed! :(");
			break;
		}

		/* Obtain OCR value	from the response buffer
		*/
		u32OcrVal = stCmd.resp.cmd_rsp0;

		/* Check if	card busy bit is cleared or	not	*/
		if (!(u32OcrVal	& MMC_CARD_BUSY))
			continue;

		u32VoltageValidation = 0;

		/* TODO: check if volatge lies in range or not*/

		/* read the byte/sector address mode bit */
		if (u32OcrVal & 0x40000000)
		    g_Card.state |= MMC_STATE_BLOCKADDR;
		else
		    g_Card.state &= ~MMC_STATE_BLOCKADDR;
			
		break;
	}

	DBG("Exit: sd_voltage_validation");

	return u32VoltageValidation;
}

/*
 * Do SD voltage validation.
 */
static s32 mmc_voltage_validation(void)
{
	struct mmc_command stCmd;
	u32 u32Response	= 0;
	u32 u32VoltageValidation = EPERM;
	s32 s32Rslt = EPERM;
	s32 s32Retries = 0;

	DBG("Entry: mmc_voltage_validation");

	for (s32Retries = RETRY_TIMEOUT; s32Retries; --s32Retries) {
		s32Rslt	= mmc_cmd(&stCmd,
				MMC_SEND_OP_COND,
				(u32)0x40FF8000,
				READ,
				RESPONSE_48,
				DATA_PRESENT_NONE,
				DISABLE,
				DISABLE);

		if (s32Rslt == EPERM) {
		       DBG("Send MMC_SEND_OP_COND Failed! :(");
		       break;
		}

		/* Obtain OCR value from the response buffer
		*/
		u32Response = stCmd.resp.cmd_rsp0;

		/* Check if card busy bit is cleared or not */
		if (!(u32Response & MMC_CARD_BUSY)) {
		    DBG("Card Busy!");

		    /* some cards need to wait before retry */
		    udelay(10000);

		    continue;
		}

		u32VoltageValidation = 0;

		/* Check if volatge lies in range or not*/
		if (0x40000000 == (u32Response & 0x60000000)) {
			DBG("Address_mode: SECT_MODE");
			g_Card.state |= MMC_STATE_BLOCKADDR;
		} else {
			DBG("Address_mode: BYTE_MODE");
			g_Card.state &= ~MMC_STATE_BLOCKADDR;
		}
	}

	DBG("mmc_voltage_validation succeed! :)");

	DBG("Exit: mmc_voltage_validation");

	return u32VoltageValidation;
}

static s32 mmc_send_cid(struct mmc_card *card)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;

	DBG("Entry: mmc_send_cid");

	if (!card) {
		DBG("NULL card pointer!");
		return EPERM;
	}

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_cmd(&stCmd,
			MMC_ALL_SEND_CID,
			0,
			READ,
			RESPONSE_136,
			DATA_PRESENT_NONE,
			ENABLE,
			DISABLE);
	if (s32Rslt) {
		DBG("Send MMC_ALL_SEND_CID Failed! :(");
		return EPERM;
	}

	/*
	card->raw_cid[0] = stCmd.resp.cmd_rsp0;
	card->raw_cid[1] = stCmd.resp.cmd_rsp1;
	card->raw_cid[2] = stCmd.resp.cmd_rsp2;
	card->raw_cid[3] = stCmd.resp.cmd_rsp3;
	*/

	card->raw_cid[0] = (stCmd.resp.cmd_rsp3 << 8) | \
				(stCmd.resp.cmd_rsp2 >> 24);
	card->raw_cid[1] = (stCmd.resp.cmd_rsp2 << 8) | \
				(stCmd.resp.cmd_rsp1 >> 24);
	card->raw_cid[2] = (stCmd.resp.cmd_rsp1 << 8) | \
				(stCmd.resp.cmd_rsp0 >> 24);
	card->raw_cid[3] = stCmd.resp.cmd_rsp0 << 8;

	DBG("mmc_send_cid succeed! :)");

	DBG("Exit: mmc_send_cid");

	return 0;
}

static s32 mmc_send_csd(struct mmc_card *card, u32 u32CardRCA)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;

	DBG("Entry: mmc_send_csd");

	if (!card) {
		DBG("NULL card pointer!");
		return s32Rslt;
	}

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_cmd(&stCmd,
			MMC_SEND_CSD,
			(u32CardRCA << 16),
			READ,
			RESPONSE_136,
			DATA_PRESENT_NONE,
			ENABLE,
			DISABLE);
	 if (s32Rslt) {
		DBG("Send MMC_SEND_CSD Failed! :(");
		return EPERM;
	 }

	 /*
	card->raw_csd[0] = stCmd.resp.cmd_rsp0;
	card->raw_csd[1] = stCmd.resp.cmd_rsp1;
	card->raw_csd[2] = stCmd.resp.cmd_rsp2;
	card->raw_csd[3] = stCmd.resp.cmd_rsp3;
	*/

	card->raw_csd[0] = (stCmd.resp.cmd_rsp3 << 8) | \
				(stCmd.resp.cmd_rsp2 >> 24);
	card->raw_csd[1] = (stCmd.resp.cmd_rsp2 << 8) | \
				(stCmd.resp.cmd_rsp1 >> 24);
	card->raw_csd[2] = (stCmd.resp.cmd_rsp1 << 8) | \
				(stCmd.resp.cmd_rsp0 >> 24);
	card->raw_csd[3] = stCmd.resp.cmd_rsp0 << 8;

	DBG("mmc_send_csd succeed! :)");

	DBG("Exit: mmc_send_csd");

	return 0;
}

static s32 mmc_select_card(u32 card_rca)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;
	u32 u32CardAddr = card_rca << 16;

	DBG("Entry: mmcsd_set_data_transfer_mode");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_cmd(&stCmd,
			MMC_SELECT_CARD,
			u32CardAddr,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);
	if (s32Rslt) {
		DBG("Send MMC_SELECT_CARD Failed! :(");
		return EPERM;
	}

	DBG("Exit mmcsd_set_data_transfer_mode");

	return mmcsd_check_status(card_rca, 96, TRAN, R1_ERROR);
}

static s32 mmcsd_check_status(u32 card_rca, u32 timeout, \
				u32 card_state, u32 status_bit)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;
	s32 s32Retries = 0;
	u32 u32CardAddr	= card_rca << 16;
	s32 s32Status =	1;

	DBG("Entry: mmcsd_check_status");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	for (s32Retries = 500; s32Retries; --s32Retries)	{

		udelay(timeout);

		s32Rslt	= mmc_cmd(&stCmd,
			MMC_SEND_STATUS,
			u32CardAddr,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);
		if (s32Rslt) {
			DBG("Send MMC_SEND_STATUS Failed!	:(");
			break;
		}

		if (stCmd.resp.cmd_rsp0 & status_bit) {
			DBG("R1 Error! :(");
			break;
		}

		if (R1_CURRENT_STATE(stCmd.resp.cmd_rsp0) == card_state) {
			DBG("Get state! :)");
			s32Status = 0;
			break;
		}
	}

	DBG("Exit: mmcsd_check_status");

	return s32Status;
}

static s32 mmc_send_relative_addr(u32 *u32CardRCA)
{
	struct mmc_command stCmd;
	s32 s32Status = 1;
	s32	s32Rslt	= EPERM;

	DBG("Entry: mmc_send_relative_addr");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_cmd(&stCmd,
			SD_SEND_RELATIVE_ADDR,
			0,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);
	if (s32Rslt) {
		DBG("Send SD_SEND_RELATIVE_ADDR Failed! :(");
		return s32Status;
	}

	*u32CardRCA = (u32)stCmd.resp.cmd_rsp0 >> 16;

	if (R1_CURRENT_STATE(stCmd.resp.cmd_rsp0) != IDENT) {
		DBG("Invalid R1 State! :(");
		return s32Status;
	}

	DBG("Exit: mmc_send_relative_addr");

	return 0;
}

static s32 mmc_set_relative_addr(u32 u32CardRCA)
{
	struct mmc_command stCmd;
	s32	s32Rslt	= EPERM;

	DBG("Entry: mmc_set_relative_addr");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	/* Set RCA */
	s32Rslt	= mmc_cmd(&stCmd,
			MMC_SET_RELATIVE_ADDR,
			(u32CardRCA	<< 16),
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);
	if (s32Rslt) {
		DBG("Send MMC_SET_RELATIVE_ADDR Failed! :(");
		return 1;
	}

	if (R1_CURRENT_STATE(stCmd.resp.cmd_rsp0) != IDENT) {
		DBG("Invalid R1 State! :(");
		return 1;
	}

	DBG("Exit: mmc_set_relative_addr");

	return 0;
}

static s32 mmc_send_scr(struct mmc_card *card)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;

	DBG("Entry: mmc_app_send_scr");

	if (!card) {
		DBG("NULL card pointer!");
		return s32Rslt;
	}

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_acmd(&stCmd,
			SD_APP_SEND_SCR,
			0,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);

	 /* Issue CMD55 to SD Memory card*/
	 if (s32Rslt) {
		DBG("Send SD_APP_SEND_SCR Failed! :(");
		return EPERM;
	 }

	card->raw_scr[0] = stCmd.resp.cmd_rsp0;
	card->raw_scr[1] = stCmd.resp.cmd_rsp1;

	mmc_decode_scr(card);

	DBG("mmc_send_scr succeed! :)");

	DBG("Exit: mmc_app_send_scr");

	return 0;
}

static s32 mmc_decode_scr(struct mmc_card *card)
{
	struct sd_scr *scr = &card->scr;
	unsigned int scr_struct;
	u32 resp[4];

	resp[3] = card->raw_scr[1];
	resp[2] = card->raw_scr[0];

	scr_struct = UNSTUFF_BITS(resp, 60, 4);
	if (scr_struct != 0) {
		printf("Unrecognised SCR structure version %d\n", scr_struct);
		return 1;
	}

	scr->sda_vsn = UNSTUFF_BITS(resp, 56, 4);
	scr->bus_widths = UNSTUFF_BITS(resp, 48, 4);

	return 0;
}

static s32 mmc_read_switch(struct mmc_card *card)
{
	u8 status[64] = { 0 };

	if (card->scr.sda_vsn < SCR_SPEC_VER_1)
		return 0;

	if (!(card->csd.cmdclass & CCC_SWITCH)) {
		printf("card lacks mandatory switch "
			"function, performance might suffer.\n");
		return 0;
	}

	if (mmc_sd_switch(card, 0, 0, 1, status)) {
		/*
		 * We all hosts that cannot perform the command
		 * to fail more gracefully
		 */
		printf("problem reading switch "
			"capabilities, performance might suffer.\n");

		return 1;
	}

	if (status[13] & 0x02)
		card->sw_caps.hs_max_dtr = 50000000;

	return 0;
}

static s32 mmc_sd_switch(struct mmc_card *card, s32 mode, s32 group,
						u8 value, u8 *resp)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;
	u32 u32Args = 0;

	DBG("Entry: mmc_sd_switch");

	if (!card) {
		DBG("NULL card pointer!");
		return s32Rslt;
	}

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	u32Args = mode << 31 | 0x00FFFFFF;
	u32Args &= ~(0xF << (group * 4));
	u32Args |= value << (group * 4);

	s32Rslt	= mmc_acmd(&stCmd,
			SD_SWITCH,
			u32Args,
			READ,
			RESPONSE_48,
			DATA_PRESENT,
			ENABLE,
			ENABLE);

	 /* Issue CMD55 to SD Memory card*/
	 if (s32Rslt) {
		DBG("Send SD_SWITCH Failed! :(");
		return EPERM;
	 }

	return 0;
}

static s32 mmc_app_set_bus_width(s32 width)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;

	DBG("Entry: mmc_app_set_bus_width");

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	s32Rslt	= mmc_acmd(&stCmd,
			SD_APP_SET_BUS_WIDTH,
			width,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);

	if (s32Rslt) {
		DBG("Send SD_APP_SET_BUS_WIDTH Failed! :(");
		return EPERM;
	 }

	DBG("Exit: mmc_app_set_bus_width");

	return 0;
}

static s32 mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value)
{
	struct mmc_command stCmd;
	s32 s32Rslt = EPERM;
	u32 u32Args = 0;

	DBG("Entry: mmc_switch");

	if (!card) {
		DBG("NULL card pointer!");
		return s32Rslt;
	}

	memset(&stCmd, 0, sizeof(struct	mmc_command));

	u32Args = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				(index << 16) | (value << 8) | set;
	DBG("switch args: 0x%x\n", u32Args);
	s32Rslt	= mmc_cmd(&stCmd,
			MMC_SWITCH,
			u32Args,
			READ,
			RESPONSE_48,
			DATA_PRESENT_NONE,
			ENABLE,
			ENABLE);

	if (s32Rslt) {
		DBG("Send MMC_SWITCH Failed! :(");
		return EPERM;
	}

	/* wait until busy is cleared */
	if (mmcsd_check_status(g_Card.rca, 500, TRAN, R1_ERROR)) {
	    printf("Can't wait for TRAN state! :(\n");
	    return EPERM;
	}

	DBG("Exit: mmc_switch");

	return 0;
}


static s32 mmc_init_sd(struct mmc_card *card)
{
	u32 u32CardRCA = 0;

	if (mmc_send_cid(card)) {
		DBG("mmcsd_get_cid Failed! :(");
		return 1;
	}

	if (mmc_send_relative_addr(&u32CardRCA)) {
		DBG("sd_send_relative_addr Failed! :(");
		return 1;
	}

	if (mmc_send_csd(card, u32CardRCA)) {
		DBG("mmcsd_get_csd Failed! :(");
		return 1;
	}

	g_Card.rca = u32CardRCA;

	mmc_decode_csd(card);
	mmc_decode_cid(card);

	/* Enable operating	frequency */
	interface_configure_clock(OPERATING_FREQ);

	if (mmc_select_card(u32CardRCA)) {
		DBG("mmc_select_card Failed! :(");
		return 1;
	}

	if (mmcsd_check_status(u32CardRCA, 96, TRAN, R1_ERROR)) {
		DBG("Can't wait for TRAN state! :(\n");
		return EPERM;
	}

	if (mmc_set_blk_len(BLK_LEN)) {
		DBG("mmc_set_blk_len Failed! :(");
		return EPERM;
	}

	/*
	if (mmc_send_scr(card)) {
		DBG("mmc_send_scr Failed! :(");
		return 1;
	}
	*/

	if (mmc_app_set_bus_width(SD_BUS_WIDTH_4)) {
		/* Try to set 1 bit mode */
		if (mmc_app_set_bus_width(SD_BUS_WIDTH_1)) {
			DBG("mmc_app_set_bus_width Failed");
			return EPERM;
		}
		interface_set_bus_width(SD_BUS_WIDTH_1);
	} else {
		interface_set_bus_width(SD_BUS_WIDTH_4);
	}

	return 0;
}

static s32 mmc_init_mmc(struct mmc_card *card)
{
	u32 u32CardRCA = 1;

	/* mmc init */
	if (mmc_send_cid(card)) {
		DBG("mmcsd_get_cid Failed! :(");
		return 1;
	}

	/* Set RCA */
	if (mmc_set_relative_addr(u32CardRCA)) {
		DBG("mmc_set_relative_addr Failed! :(");
		return 1;
	}

	if (mmc_send_csd(card, u32CardRCA)) {
		DBG("mmcsd_get_csd Failed! :(");
		return 1;
	}

	g_Card.rca = u32CardRCA;

	mmc_decode_csd(card);
	mmc_decode_cid(card);

	/* Enable operating frequency */
	interface_configure_clock(OPERATING_FREQ);

	if (mmc_select_card(u32CardRCA)) {
		DBG("mmc_select_card Failed! :(");
		return 1;
	}

	if (mmcsd_check_status(g_Card.rca, 96, TRAN, R1_ERROR)) {
		DBG("Can't wait for TRAN state! :(\n");
		return EPERM;
	}

	if (mmc_set_blk_len(BLK_LEN)) {
		DBG("mmc_set_blk_len Failed! :(");
		return 1;
	}
	// BEN TODO: check if card supports
	if (!mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
		       EXT_CSD_HS_TIMING, 1)) {
	    printf("high speed\n");
	    /* Enable high speed frequency */
	    interface_configure_clock(HIGHSPEED_FREQ);
	} else {
	    printf("normal speed\n");
	}

	if (card->csd.mmca_vsn >= CSD_SPEC_VER_4) {
#if 0 //(CONFIG_SYS_FSL_ESDHC_ADDR == MMC_SDHC1_BASE_ADDR)
	    if (mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			   EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_8)) {
		DBG("Switch card to 8 bits failed! :(\n");
		return 1;
	    }
	    interface_set_bus_width(MMC_BUS_WIDTH_8);
#else
	    if (mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			   EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_4)) {
		DBG("Switch card to 4 bits failed! :(\n");
		return 1;
	    }
	    interface_set_bus_width(MMC_BUS_WIDTH_4);
#endif
	}

	return 0;
}

extern int sdhc_init(int verbose);

int
/****************************************************/
mmc_legacy_init(int mmc_device)
/****************************************************/
{
	struct mmc_command stCmd;
	s32	s32InitStatus =	-1;
	s32	s32Rslt	= EPERM;

	DBG("Entry: mmc_init\n");

	memset(&stCmd, 0, sizeof(struct	mmc_command));
	memset(&g_Card, 0, sizeof(struct mmc_card));

	/* Reset device	interface type */
	mmc_dev.if_type	= IF_TYPE_UNKNOWN;

	/* initialize Interface	Controller */
	sdhc_init(mmc_device);

	/* Software reset to Interface Controller */
	if (interface_reset()) {
		DBG("interface_reset failed! :(");
		return s32InitStatus;
	}

	/* Enable Identification Frequency */
	interface_configure_clock(IDENTIFICATION_FREQ);

	/* Software reset */
	s32Rslt	= mmc_cmd(&stCmd,
			MMC_GO_IDLE_STATE,
			0,
			READ,
			RESPONSE_NONE,
			DATA_PRESENT_NONE,
			DISABLE,
			DISABLE);

	if (!sd_voltage_validation()) {
		DBG("SD Card Detected!");
		g_Card.type = MMC_TYPE_SD;

		/* SD init */
		if (mmc_init_sd(&g_Card)) {
		    printf("SD init failed!\n");
		    return s32InitStatus;
		}

		s32InitStatus = 0;
		mmc_ready = 1;
	} else if (!mmc_voltage_validation()) {
		DBG("MMC Card	Detected!");
		g_Card.type = MMC_TYPE_MMC;

		/* mmc init	*/
		if (mmc_init_mmc(&g_Card)) {
		    printf("MMC init failed!\n");
		    return s32InitStatus;
		}

		s32InitStatus = 0;
		mmc_ready = 1;
	} else {
	    printf("MMC init: no card found!\n");
	    mmc_ready = 0;
	    return s32InitStatus;
	}

#ifdef CONFIG_CMD_FAT
	fat_register_device(&mmc_dev, 1); /* partitions start counting with 1 */
#endif
	DBG("Exit: mmc_init\n");

	return s32InitStatus;
}

int mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int mmc2info(ulong addr)
{
	/* Not avaiable for cp command now. */
	return 0;

	if (addr >= CFG_MMC_BASE
	    && addr < CFG_MMC_BASE + (mmc_dev.lba * mmc_dev.blksz)) {
		return 1;
	}
	return 0;

}

#ifdef BEN_TODO
/*
 * CPU and board-specific MMC initializations.  Aliased function
 * signals caller to move on
 */
static int __def_mmc_init(bd_t *bis)
{
	return -1;
}

int cpu_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));
int board_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));

struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->block_dev.dev == dev_num)
			return m;
	}

	printf("MMC Device %d not found\n", dev_num);

	return NULL;
}

void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		printf("%s: %d", m->name, m->block_dev.dev);

		if (entry->next != &mmc_devices)
			printf("%c ", separator);
	}

	printf("\n");
}

int mmc_initialize(bd_t *bis)
{
	INIT_LIST_HEAD (&mmc_devices);
	cur_dev_num = 0;

	if (board_mmc_init(bis) < 0)
		cpu_mmc_init(bis);

	print_mmc_devices(',');

	return 0;
}
#endif
