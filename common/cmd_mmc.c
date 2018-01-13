/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <mmc.h>

static int curr_device = -1;

enum mmc_state {
	MMC_INVALID,
	MMC_READ,
	MMC_WRITE,
	MMC_ERASE,
};

static void print_mmcinfo(struct mmc *mmc)
{
	printf("Device: %s\n", mmc->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c%c \n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
	       (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff, (mmc->cid[2] >> 24));
	printf("Revision: 0x%x\n", (mmc->cid[2] >> 16) & 0xff);
	printf("Tran Speed: %d\n", mmc->tran_speed);
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 4) & 0xf, mmc->version & 0xf);

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	printf("Capacity: %lld\n", mmc->capacity);

	printf("Bus Width: %d-bit %s\n", mmc->bus_width,
		(mmc->card_caps & EMMC_MODE_4BIT_DDR ||
		 mmc->card_caps & EMMC_MODE_8BIT_DDR) ? "DDR" : "");

	if (mmc->part_config == MMCPART_NOAVAILABLE) {
		printf("Boot Partition for boot: %s\n",
			"No boot partition available");
	} else {
		printf("Current Partition for boot: ");
		switch (mmc->part_config & EXT_CSD_BOOT_PARTITION_ENABLE_MASK) {
		case EXT_CSD_BOOT_PARTITION_DISABLE:
			printf("Not bootable\n");
			break;
		case EXT_CSD_BOOT_PARTITION_PART1:
			printf("Boot partition 1\n");
			break;
		case EXT_CSD_BOOT_PARTITION_PART2:
			printf("Boot partition 2\n");
			break;
		case EXT_CSD_BOOT_PARTITION_USER:
			printf("User area\n");
			break;
		default:
			printf("Unknown\n");
			break;
		}

		printf("Current boot width: ");
		switch (mmc->boot_bus_width & 0x3) {
		  case EXT_CSD_BOOT_BUS_WIDTH_1BIT:
		    printf("1-bit ");
		    break;
		  case EXT_CSD_BOOT_BUS_WIDTH_4BIT:
		    printf("4-bit ");
		    break;
		  case EXT_CSD_BOOT_BUS_WIDTH_8BIT:
		    printf("8-bit ");
		  default:
		    printf("Unknown ");
		}

		switch ((mmc->boot_bus_width >> 3) & 0x3) {
		  case 0:
		    printf("SDR\n");
		    break;
		  case EXT_CSD_BOOT_BUS_WIDTH_SDR_HS:
		    printf("SDR High-speed\n");
		    break;
		  case EXT_CSD_BOOT_BUS_WIDTH_DDR:
		    printf("DDR\n");
		  default:
		    printf("Unknown\n");
		}
	}
}

int do_mmcinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct mmc *mmc;

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}

	mmc = find_mmc_device(curr_device);

	if (mmc) {
		mmc_init(mmc);

		print_mmcinfo(mmc);
		return 0;
	} else {
		printf("no mmc device at slot %x\n", curr_device);
		return 1;
	}
}

U_BOOT_CMD(
	mmcinfo, 1, 0, do_mmcinfo,
	"display MMC info",
	"    - device number of the device to dislay info of\n"
	""
);

int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	enum mmc_state state;
	u32 part = 0;
	int err = 0;
	ulong st, end, tm;
	u32 numblks = CONFIG_MMC_MAX_TRANSFER_SIZE;

	if (argc < 2)
		return cmd_usage(cmdtp);

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}

	if (strcmp(argv[1], "rescan") == 0) {
		struct mmc *mmc = find_mmc_device(curr_device);

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		mmc->has_init = 0;

		if (mmc_init(mmc))
			return 1;
		else
			return 0; 
	} else if (strcmp(argv[1], "list") == 0) {
		print_mmc_devices('\n');
		return 0;
	} else if (strcmp(argv[1], "dev") == 0) {
		int dev;
		struct mmc *mmc;

		if (argc == 2)
			dev = curr_device;
		else if (argc == 3)
			dev = simple_strtoul(argv[2], NULL, 10);
		else
			return cmd_usage(cmdtp);

		mmc = find_mmc_device(dev);
		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return 1;
		}

		mmc_init(mmc);

		curr_device = dev;
		if (mmc->part_config == MMCPART_NOAVAILABLE)
			printf("mmc%d is current device\n", curr_device);
		else
			printf("mmc%d(part %d) is current device\n",
				curr_device, mmc->part_num);

		return 0;
	}

	if (strcmp(argv[1], "read") == 0) {
		state = MMC_READ;
		if (argc < 5)
		    return cmd_usage(cmdtp);
	} else if (strcmp(argv[1], "write") == 0) {
		state = MMC_WRITE;
		if (argc < 5)
		    return cmd_usage(cmdtp);
	} else if (strcmp(argv[1], "erase") == 0) {
		state = MMC_ERASE;
		if (argc < 4)
		    return cmd_usage(cmdtp);
	} else {
		state = MMC_INVALID;
	}

	if (state != MMC_INVALID) {
		struct mmc *mmc = find_mmc_device(curr_device);
		int idx = 2;
		u32 blk, cnt, len, trlen, n = 0;
		void *addr;

		if (state != MMC_ERASE) {
			addr = (void *)simple_strtoul(argv[idx], NULL, 16);
			++idx;
		} else
			addr = 0;
		blk = simple_strtoul(argv[idx++], NULL, 16);
		cnt = simple_strtoul(argv[idx++], NULL, 16);

		/* Check for optional partition argument */
		if (idx < argc) {
		    part = simple_strtoul(argv[idx++], NULL, 10);		    
		}

		/* Check for optional transfer size argument */
		if (idx < argc) {
		    numblks = simple_strtoul(argv[idx++], NULL, 10);
		    numblks *= mmc->read_bl_len;
		}

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		mmc_init(mmc);

		printf("\nMMC %s: dev # %d, %s %d, "
		       "blk count %d, blks/transfer %d, partition # %d ... \n",
		       argv[1], curr_device, mmc->high_capacity ? "offset" : "block #",
                       blk, cnt, (numblks/mmc->read_bl_len), part);

		/* Switch partition */
		if (mmc->part_config != MMCPART_NOAVAILABLE && 
		    part != mmc->part_num)
		{
		    if (IS_SD(mmc))
			err = sd_switch_part(curr_device, part);
		    else
			err = mmc_switch_partition(mmc, part, 0);

		    if (err) {
			printf("Error switching to partition %d\n", part);
			return -1;
		    }
		    
		    mmc->part_num = part;
		}
		
		st = get_timer_masked();

		len = cnt * mmc->read_bl_len;

		while (len && !err) {

		    trlen = MIN(numblks, len);

		    switch (state) {
		      case MMC_READ:
			err = mmc_read(curr_device, blk + n, addr + n, trlen);
			break;
		      case MMC_WRITE:
			err = mmc_write(curr_device, addr + n, blk + n, trlen);
			break;
		      case MMC_ERASE:
			/* BEN TODO */
		      default:
			printf("Invalid MMC operation!\n");
			return -1;
		    }

		    n += trlen;
		    len -= trlen;
		}

		end = get_timer_masked();
		
		tm = end - st;
#if defined(CONFIG_MX50)
		tm /= 1000;
#endif
		printf("%s %d blocks in %ld msec: %s\n", argv[1], cnt, tm, (n == (cnt * mmc->read_bl_len)) ? "OK" : "ERROR");
		
		return (n == (cnt * mmc->read_bl_len) && !err) ? 0 : 1;
	}

	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"MMC sub system",
	"read addr blk# cnt\n"
	"mmc write addr blk# cnt\n"
	"mmc erase blk# cnt\n"
	"mmc rescan\n"
	"mmc part - lists available partition on current mmc device\n"
	"mmc dev [dev] [part] - show or set current mmc device [partition]\n"
	"mmc bootpart [dev] [part] - show or set boot partition\n"
	"mmc list - lists available devices");
