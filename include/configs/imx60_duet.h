/*
 * Copyright (C) 2014-2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX6SL Wario board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

 /* High Level Configuration Options */
#define CONFIG_ARMV7	/* This is armv7 Cortex-A9 CPU core */
#define CONFIG_MXC
#define CONFIG_MX6SL
#define CONFIG_MX6SL_ARM2
#define CONFIG_QBOOT
#define CONFIG_WARIO
#define CONFIG_WARIO_WOODY
#define CONFIG_FLASH_HEADER
#define CONFIG_FLASH_HEADER_OFFSET 0x400
#define CONFIG_MX6_CLK32	   32768

#include <asm/arch/mx6.h>

#define CONFIG_SKIP_RELOCATE_UBOOT

#define CONFIG_ARCH_CPU_INIT
#ifdef CONFIG_QBOOT
#define CONFIG_ARCH_MMU /* enable MMU for QuickBoot */
#else
#undef CONFIG_ARCH_MMU /* disable MMU first */
#endif
#define CONFIG_L2_OFF  /* disable L2 cache first*/
#ifdef CONFIG_QBOOT
#define CONFIG_BOOT_RETRY_TIME (-1) /* enable but do not use by default */
#define CONFIG_RESET_TO_RETRY /* completely re-initialize */
#define CONFIG_ZERO_BOOTDELAY_CHECK /* allow stop autoboot by holding key at boot */
#endif /* CONFIG_QBOOT */

#define CONFIG_IRAM_BOOT
#define CONFIG_MX6_HCLK_FREQ	24000000

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_64BIT_VSPRINTF

#define BOARD_LATE_INIT

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_SERIAL16_TAG		1
#define CONFIG_REVISION16_TAG		1
#define CONFIG_POST_TAG			1
#define CONFIG_MACADDR_TAG		1
#define CONFIG_BTMACADDR_TAG		1
#define CONFIG_BOOTMODE_TAG		1
#define CONFIG_DDRMFGID_TAG		1


/*
 * Size of malloc() pool
 */
/* BEN TODO */
#define CONFIG_SYS_MALLOC_LEN		(8 * 1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_SYS_GBL_DATA_OFFSET	(TEXT_BASE - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_POST_WORD_ADDR

/*
 * Hardware drivers
 */
#define CONFIG_MXC_UART
#define CONFIG_UART_BASE_ADDR		UART1_IPS_BASE_ADDR

#define CONFIG_MXC_GPIO    1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/* MMC drivers */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_IMX_MMC
#define CONFIG_SYS_FSL_ESDHC_NUM        3
#define CONFIG_SYS_FSL_ESDHC_DMA	1

#define CONFIG_MMC_BOOTFLASH		1
#define CONFIG_MMC_BOOTFLASH_ADDR	0x41000
#define CONFIG_MMC_BOOTFLASH_SIZE	(14*1024*1024) /* 14 MiB */
#define CONFIG_MMC_BOOTDIAGS_ADDR	0xE41000
#define CONFIG_MMC_USERDATA_ADDR	0x5E000
#define CONFIG_MMC_USERDATA_SIZE	(5*1024)
#define CONFIG_MMC_BIST_ADDR		(192*1024)
#define CONFIG_MMC_BIST_SIZE		(256*1024)
#define CONFIG_MMC_MAX_TRANSFER_SIZE	(0xFFFF * 512)

#define CONFIG_BOOT_PARTITION_ACCESS
#define CONFIG_BOOT_FROM_PARTITION	1

#ifdef CONFIG_QBOOT
/*
 * QuickBoot related configs.
 */
#define CONFIG_QB_PARTITION             2
#define CONFIG_MMC_SBIOS_ADDR           (0)
#define CONFIG_MMC_SBIOS_SIZE           (0x10000)
#define CONFIG_MMC_FBIOS_ADDR           (CONFIG_MMC_SBIOS_ADDR + CONFIG_MMC_SBIOS_SIZE)
#define CONFIG_MMC_FBIOS_SIZE           (0x20000)
#endif /* CONFIG_QBOOT */

/* SD2 is 8 bit */
#define CONFIG_MMC_8BIT_PORTS   0x2


/*
 * I2C Configs
 */
//#define CONFIG_I2C_MXC          1
#define CONFIG_I2C_MXC_LAB126		1
#define CONFIG_HARD_I2C         1   /* I2C with hardware support    */
#define CONFIG_SOFT_I2C         1   /* I2C bit-banged       */
/* BEN TODO */
#if 0
#define CONFIG_I2C_MULTI_BUS
#else
#define CONFIG_SYS_I2C_PORT I2C1_BASE_ADDR
#endif
#define CONFIG_SYS_I2C_SPEED    100000  /* I2C speed and slave address  */

#define CONFIG_PMIC     1  
#define CONFIG_PMIC_I2C     1  
/* Enable Gas gauge settings */
#define CONFIG_MAX77696_FG_INIT   1 
#define CONFIG_PMIC_MAX77696    1

/***********************************************************
 * Command definition
 ***********************************************************/

#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */

/* Lab 126 cmds */
#define CONFIG_CMD_BIST		1
#define CONFIG_CMD_IDME		1

#define CONFIG_IDME_UPDATE		1
#define CONFIG_IDME_UPDATE_ADDR		0x3f000
#define CONFIG_IDME_UPDATE_MAGIC	"abcdefghhgfedcba"
#define CONFIG_IDME_UPDATE_MAGIC_SIZE	16

#define CONFIG_REF_CLK_FREQ CONFIG_MX6_HCLK_FREQ

#undef CONFIG_CMD_IMLS

#if defined(DEVELOPMENT_MODE)
#define CONFIG_BOOTDELAY	3
#else
#define CONFIG_BOOTDELAY	1
#endif

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */
#define CONFIG_RD_LOADADDR	(CONFIG_LOADADDR + 0x300000)
#define CONFIG_BISTADDR		0x80400000

#define CONFIG_BISTCMD_LOCATION (CONFIG_BISTADDR - 0x70000)
#define CONFIG_BISTCMD_MAGIC	0xBC 

#ifdef CONFIG_QBOOT
#define	CONFIG_EXTRA_ENV_SETTINGS \
    "failbootcmd=panic\0" \
    "post_hotkeys=0\0" \
    "loglevel=5\0" \
    "bootcmd_diags=run bootcmd_FB;bootm " MK_STR(CONFIG_MMC_BOOTDIAGS_ADDR) "\0" \
    "bootcmd_factory=bist halt\0" \
    "bootcmd_fastboot=bist fastboot\0" \
    "bootcmd=run bootcmd_FB; bootm " MK_STR(CONFIG_MMC_BOOTFLASH_ADDR) "\0"
#else /* !CONFIG_QBOOT */
#define	CONFIG_EXTRA_ENV_SETTINGS \
    "bootcmd=bootm " MK_STR(CONFIG_MMC_BOOTFLASH_ADDR) "\0" \
    "failbootcmd=panic\0" \
    "post_hotkeys=0\0" \
    "loglevel=5\0" \
    "bootcmd_diags=bootm " MK_STR(CONFIG_MMC_BOOTDIAGS_ADDR) "\0" \
    "bootcmd_factory=bist halt\0" \
    "bootcmd_fastboot=bist fastboot\0"
#endif /* CONFIG_QBOOT */                                                 

/*
 * Miscellaneous configurable options
 */
#undef CONFIG_SYS_LONGHELP		/* undef to save memory */
#undef CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_PROMPT		"uboot > "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_ORG_MEMTEST      /* Original (not so) quickie memory test */
#define CONFIG_SYS_ALT_MEMTEST      /* Newer data, address, integrity test */
#define CONFIG_SYS_MEMTEST_SCRATCH  IRAM_BASE_ADDR      /* Internal RAM */
#define CONFIG_SYS_MEMTEST_START    CONFIG_SYS_SDRAM_BASE	/* memtest works on */ 
#define CONFIG_WDOG_PRINTK_SIZE	    (4096 * 16) //Printk is only 32k, but could not be placed in the last pages.
#define CONFIG_SYS_MEMTEST_END      (PHYS_SDRAM_1 + get_dram_size() - 1 - CONFIG_WDOG_PRINTK_SIZE)

#define CONFIG_POST         (CONFIG_SYS_POST_MEMORY | \
                             CONFIG_SYS_POST_FAIL)

#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ			1000

#define CONFIG_MX6_INTER_LDO_BYPASS	1

#define CONFIG_CMDLINE_EDITING  1

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_DDR_BASE_ADDR

#ifdef CONFIG_QBOOT
#define PHYS_SDRAM_1_SIZE	(512 * 1024 * 1024)
#endif /* CONFIG_QBOOT */

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define iomem_valid_addr(addr, size) \
	(addr >= PHYS_SDRAM_1 && addr <= (PHYS_SDRAM_1 + get_dram_size()))

/*-----------------------------------------------------------------------
 * IRAM Memory Map
 */
#define IRAM_FREE_START		0x00907000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/* Monitor at beginning of flash */
/* #define CONFIG_FSL_ENV_IN_SF
*/
/* #define CONFIG_FSL_ENV_IN_MMC */

#ifndef CONFIG_QBOOT
#define CONFIG_ENV_SECT_SIZE    (1 * 1024)
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_IS_NOWHERE
#else /* CONFIG_QBOOT */
#include "mx6sl_wario_falcon.h"
#endif /* CONFIG_QBOOT */

#ifndef __ASSEMBLY__
#include <asm/arch/mx60_wario_board.h>
#endif

#endif				/* __CONFIG_H */
