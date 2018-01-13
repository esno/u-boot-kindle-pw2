/*
 * mx60_wario_board.h
 *
 * Copyright 2010-2015 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __MX60_WARIO_BOARD_H__
#define __MX60_WARIO_BOARD_H__

#include <boardid.h>

#define MEMORY_TYPE_UNKNOWN	0
#define MEMORY_TYPE_MDDR	1
#define MEMORY_TYPE_LPDDR2	2

#define MEMORY_SIZE_256MB       (256 * 1024 * 1024)
#define MEMORY_SIZE_512MB       (512 * 1024 * 1024)
#define MEMORY_SIZE_1GB         (1024 * 1024 * 1024)

typedef struct board_type {
    const char *id;  /* Tattoo + rev */
    const char *name;
    unsigned char mem_type;
    unsigned int mem_size;
} board_type;

static const struct board_type boards[] = {
    {
	.id = BOARD_ID_WARIO_1,
	.name = "Wario 1.0",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_WARIO_2,
	.name = "Wario 2.0",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_WARIO_2_1,
	.name = "Wario 2.1",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_WARIO_3,
	.name = "Wario 3 256MB",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_WARIO_3_512M,
	.name = "Wario 3 512MB",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_WARIO_3_256M_4P35BAT,
	.name = "Wario 3 256MB 4P35BAT",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_WARIO_3_512M_4P35BAT,
	.name = "Wario 3 512MB 4P35BAT",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {	
	.id = BOARD_ID_WARIO_4_256M_CFG_C,
	.name = "Wario 4 256MB CFG_C BOURBON ZFORCE2",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {	
	.id = BOARD_ID_WARIO_4_512M_CFG_B,
	.name = "Wario 4 512MB CFG_B ICEWINE CYTTSP4",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {	
	.id = BOARD_ID_WARIO_4_512M_CFG_D,
	.name = "Wario 4 512MB CFG_D BRCM Wifi+BT 4343W",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {	
	.id = BOARD_ID_WARIO_4_1G_CFG_A,
	.name = "Wario 4 1GB CFG_A WEIM FPGA",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_1GB,
    },
    {
	.id = BOARD_ID_ICEWINE_WARIO,
	.name = "Icewine",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
    	.id = BOARD_ID_ICEWINE_WFO_WARIO,
	.name = "Icewine WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
        .id = BOARD_ID_ICEWINE_WARIO_512,
        .name = "Icewine 512",
        .mem_type = MEMORY_TYPE_LPDDR2,
        .mem_size = MEMORY_SIZE_512MB,
    },
    {
        .id = BOARD_ID_ICEWINE_WFO_WARIO_512,
        .name = "Icewine WFO 512",
        .mem_type = MEMORY_TYPE_LPDDR2,
        .mem_size = MEMORY_SIZE_512MB,
    },
    {
    	.id = BOARD_ID_PINOT_WFO,
	.name = "Pinot WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
    	.id = BOARD_ID_PINOT,
	.name = "Pinot",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
    	.id = BOARD_ID_PINOT_WFO_2GB,
	.name = "Pinot WFO 2GB",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_PINOT_2GB,
	.name = "Pinot 2GB",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_BOURBON_WFO,
	.name = "Bourbon WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_BOURBON_WFO_PREEVT2,
	.name = "Bourbon PREEVT2 WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_256MB,
    },
    {
	.id = BOARD_ID_MUSCAT_WAN,
	.name = "Muscat WAN",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_MUSCAT_WFO,
	.name = "Muscat WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_MUSCAT_32G_WFO,
	.name = "Muscat WFO 32GB",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_WHISKY_WAN,
	.name = "Whisky WAN",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_WHISKY_WFO,
	.name = "Whisky WFO",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_WOODY,
	.name = "Woody",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
};

#define NUM_KNOWN_BOARDS (sizeof(boards)/sizeof(boards[0]))

#define PARTITION_FILL_SPACE	-1

typedef struct partition_info_t {
    const char *name;
    unsigned int address;
    unsigned int size;
    unsigned int partition;
} partition_info_t;

#ifdef CONFIG_QB_PARTITION
#define CONFIG_NUM_PARTITIONS 12
#else
#define CONFIG_NUM_PARTITIONS 10
#endif

static const struct partition_info_t partition_info_350[CONFIG_NUM_PARTITIONS] = {
    {
	.name = "bootloader",
	.address = 0,
	.size = (376*1024), /* 376 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "prod",
	.address = 0x0, /* overlap with bootloader */
	.size = (120*1024), /* 120 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "bist",
	.address = CONFIG_MMC_BIST_ADDR, /* overlap with bootloader */
	.size = CONFIG_MMC_BIST_SIZE, /* 256 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
#ifdef CONFIG_QB_PARTITION
    {
	.name = "sbios",
	.address = CONFIG_MMC_SBIOS_ADDR,
	.size = CONFIG_MMC_SBIOS_SIZE,
	.partition = CONFIG_QB_PARTITION,
    },
    {
	.name = "fbios",
	.address = CONFIG_MMC_FBIOS_ADDR,
	.size = CONFIG_MMC_FBIOS_SIZE,
	.partition = CONFIG_QB_PARTITION,
    },
#endif
    {
	.name = "userdata",
	.address = CONFIG_MMC_USERDATA_ADDR,
	.size = CONFIG_MMC_USERDATA_SIZE,  /* 5 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "userpartition",
	.address = 0,
	.size = PARTITION_FILL_SPACE,  /* based on MMC size */
	.partition = 0,
    },
    {
	.name = "mbr",
	.address = 0,
	.size = 1024,  /* 1 KiB */
	.partition = 0,
    },
    {
	.name = "kernel",
	.address = CONFIG_MMC_BOOTFLASH_ADDR,
	.size = CONFIG_MMC_BOOTFLASH_SIZE,  /* 14 MiB */
	.partition = 0,
    },
    {
	.name = "diags_kernel",
	.address = CONFIG_MMC_BOOTDIAGS_ADDR,
	.size = CONFIG_MMC_BOOTFLASH_SIZE,  /* 14 MiB */
	.partition = 0,
    },
    {
	.name = "system",
	.address = 0x2000000,
	.size = (350*1024*1024),  /* 350 MiB */
	.partition = 0,
    },
    {
	.name = "diags",
	.address = 0x17E00000,
	.size = (64*1024*1024),  /* 64 MiB */
	.partition = 0,
    }
};

static const struct partition_info_t partition_info_default[CONFIG_NUM_PARTITIONS] = {
    {
	.name = "bootloader",
	.address = 0,
	.size = (376*1024), /* 376 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "prod",
	.address = 0x0, /* overlap with bootloader */
	.size = CONFIG_MMC_BIST_ADDR, /* prod uboot's size is only limited by bist's address*/
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "bist",
	.address = CONFIG_MMC_BIST_ADDR, /* overlap with bootloader */
	.size = CONFIG_MMC_BIST_SIZE, /* 256 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
#ifdef CONFIG_QB_PARTITION
    {
	.name = "sbios",
	.address = CONFIG_MMC_SBIOS_ADDR,
	.size = CONFIG_MMC_SBIOS_SIZE,
	.partition = CONFIG_QB_PARTITION,
    },
    {
	.name = "fbios",
	.address = CONFIG_MMC_FBIOS_ADDR,
	.size = CONFIG_MMC_FBIOS_SIZE,
	.partition = CONFIG_QB_PARTITION,
    },
#endif
    {
	.name = "userdata",
	.address = CONFIG_MMC_USERDATA_ADDR,
	.size = CONFIG_MMC_USERDATA_SIZE,  /* 5 KiB */
	.partition = CONFIG_BOOT_FROM_PARTITION,
    },
    {
	.name = "userpartition",
	.address = 0,
	.size = PARTITION_FILL_SPACE,  /* based on MMC size */
	.partition = 0,
    },
    {
	.name = "mbr",
	.address = 0,
	.size = 1024,  /* 1 KiB */
	.partition = 0,
    },
    {
	.name = "kernel",
	.address = CONFIG_MMC_BOOTFLASH_ADDR,
	.size = CONFIG_MMC_BOOTFLASH_SIZE,  /* 14 MiB */
	.partition = 0,
    },
    {
	.name = "diags_kernel",
	.address = CONFIG_MMC_BOOTDIAGS_ADDR,
	.size = CONFIG_MMC_BOOTFLASH_SIZE,  /* 14 MiB */
	.partition = 0,
    },
    {
	.name = "system",
	.address = 0x2000000,
	.size = (450*1024*1024),  /* 450 MiB */
	.partition = 0,
    },
    {
	.name = "diags",
	.address = 0x1E200000,
	.size = (64*1024*1024),  /* 64 MiB */
	.partition = 0,
    }
};

/*
 * Do not modify these length fields unless you know what you are doing.
 * Changing these can easily cause data corruption 
 */
#define BOARD_DSN_LEN	16
#define BOARD_MAC_LEN 12
#define BOARD_SEC_LEN 20
#define BOARD_PCBA_LEN	16
#define BOARD_BOOTMODE_LEN 16
#define BOARD_POSTMODE_LEN 16
#define BOARD_QBCOUNT_LEN 10


typedef struct nvram_t {
    const char *name;
    unsigned int offset;
    unsigned int size;
} nvram_t;

static const struct nvram_t nvram_info[] = {
    {
	.name = "serial",
	.offset = 0x00,
	.size = BOARD_DSN_LEN,
    },
    {
	.name = "mac",
	.offset = 0x30,
	.size = BOARD_MAC_LEN,
    },
    {
	.name = "sec",
	.offset = 0x40,
	.size = BOARD_SEC_LEN,
    },
    {
	.name = "pcbsn",
	.offset = 0x60,
	.size = BOARD_PCBA_LEN,
    },
    {
	.name = "bootmode",
	.offset = 0x1000,
	.size = BOARD_BOOTMODE_LEN,
    },
    {
	.name = "postmode",
	.offset = 0x1010,
	.size = BOARD_POSTMODE_LEN,
    },
    {
	.name = "btmac",
	.offset = 0x1040,
	.size = 12,
    },
#ifdef CONFIG_QBOOT
    {
	.name = "oldboot",
	.offset = 0x1050,
	.size = BOARD_BOOTMODE_LEN,
    },
    {
	.name = "qbcount",
	.offset = 0x1060,
	.size = BOARD_QBCOUNT_LEN,
    },
#endif
};

#define CONFIG_NUM_NV_VARS (sizeof(nvram_info)/sizeof(nvram_info[0]))

extern int board_mmu_init(void);

#endif /* __MX60_WARIO_CFG_H__ */
