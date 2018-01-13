/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
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
#define CONFIG_WARIO
#define CONFIG_WARIO_BASE
#define CONFIG_FLASH_HEADER
#define CONFIG_FLASH_HEADER_OFFSET 0x400
#define CONFIG_MX6_CLK32	   32768

#include <asm/arch/mx6.h>

#define CONFIG_SKIP_RELOCATE_UBOOT

#define CONFIG_LAB126_LPM_TEST		1

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_ARCH_MMU /* disable MMU first */
#define CONFIG_L2_OFF  /* disable L2 cache first*/

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

#define CONFIG_DSN_LEN	16
#define CONFIG_PCBA_LEN	16

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(100 * 1024)
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
#define CONFIG_MMC_SDIO
#define CONFIG_IMX_MMC
#define CONFIG_SYS_FSL_ESDHC_NUM        3
#define CONFIG_SYS_FSL_ESDHC_DMA	1

#define CONFIG_MMC_BOOTFLASH		1
#define CONFIG_MMC_BOOTFLASH_ADDR	0x41000
#define CONFIG_MMC_BOOTFLASH_SIZE	(14*1024*1024) /* 14 MiB */
#define CONFIG_MMC_BOOTDIAGS_ADDR	0xE41000
#define CONFIG_MMC_USERDATA_ADDR	0x5E000
#define CONFIG_MMC_USERDATA_SIZE	(5*1024)
#define CONFIG_MMC_BIST_ADDR		(120*1024)
#define CONFIG_MMC_BIST_SIZE		(256*1024)
#define CONFIG_MMC_MAX_TRANSFER_SIZE	(0xFFFF * 512)

#define CONFIG_BOOT_PARTITION_ACCESS
#define CONFIG_BOOT_FROM_PARTITION	1

/* SD2 is 8 bit */
#define CONFIG_MMC_8BIT_PORTS   0x2

/* crc32 Buffer*/
#define CRC32_BUFFER        0x02000000 + PHYS_SDRAM_1
#define CRC32_BUFFER_SIZE 	0x00400000
#define CRC32_CHECK_UBOOT	0x00000010	/* check crc32 on u-boot.bin */
#define CRC32_CHECK_UIMAGE	0x00000020	/* check crc32 on uImage */
#define CRC32_CHECK_ROOTFS	0x00000040	/* check crc32 on rootfs.img */
#define CRC32_CHECK_MBR		0x00000080	/* check crc32 on mbr-xxx-bin */

/*
 * USB Configs
 */
#define CONFIG_USB_DEVICE		1
/* #define CONFIG_IMX_UDC		1 */
#define CONFIG_DRIVER_FSLUSB		1
#define CONFIG_GADGET_FASTBOOT		1
/* #define CONFIG_GADGET_FILE_STORAGE	1 */
#define USB_BASE_ADDR			OTG_BASE_ADDR

#define CONFIG_USBD_MANUFACTURER "Amazon"
#define CONFIG_USBD_PRODUCT_NAME "Kindle"

#define CONFIG_USBD_VENDORID			0x1949
#define CONFIG_USBD_PRODUCTID_FASTBOOT		0xd0e0
#define CONFIG_USBD_PRODUCTID_FILE_STORAGE	0x0003
#define CONFIG_FASTBOOT_MAX_DOWNLOAD_LEN	((get_dram_size()) - (2*1024*1024) - (CONFIG_FASTBOOT_TEMP_BUFFER - CONFIG_SYS_SDRAM_BASE))
#define CONFIG_FASTBOOT_TEMP_BUFFER		CONFIG_LOADADDR

/*
 * SPI Configs
 */
#define CONFIG_IMX_SPI
#define CONFIG_IMX_ECSPI
#define MAX_SPI_BYTES		(8 * 4)

/*
 * I2C Configs
 */
#define CONFIG_CMD_I2C          1
//#define CONFIG_I2C_MXC			1
#define CONFIG_I2C_MXC_LAB126		1
#define CONFIG_HARD_I2C			1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
/* BEN TODO */
#if 1
#define CONFIG_I2C_MULTI_BUS
#else
#define CONFIG_SYS_I2C_PORT	I2C1_BASE_ADDR
#endif
#define CONFIG_SYS_I2C_SPEED	100000	/* I2C speed and slave address	*/

#define CONFIG_PMIC		1
#define CONFIG_PMIC_I2C		1  
#define CONFIG_PMIC_MAX77696	1
#define CONFIG_CMD_PMIC		1

/***********************************************************
 * Command definition
 ***********************************************************/

/* Standard commands */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_IMI		/* iminfo			*/
#define CONFIG_CMD_ITEST	/* Integer (and string) test	*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_ENV
#define CONFIG_CMD_MMC
#define CONFIG_CMD_PANIC
#define CONFIG_XYZMODEM
#define CONFIG_SRECORD
#define CONFIG_LOOPW
#define CONFIG_CMD_HALT

/* Lab 126 cmds */
#define CONFIG_CMD_CRC 		1
#define CONFIG_CMD_SPI
#define CONFIG_CMD_GADGET	1
#define CONFIG_CMD_IDME		1
#define CONFIG_CMD_LPM		1
#define CONFIG_CMD_VNI		1
#define CONFIG_CMD_HAPTIC       1
#define CONFIG_CMD_FSR		1

/*
 * OCOTP Configs
 */
#define CONFIG_CMD_IMXOTP		1
#ifdef CONFIG_CMD_IMXOTP
	#define CONFIG_IMX_OTP
	#define IMX_OTP_BASE		OCOTP_BASE_ADDR
	#define IMX_OTP_ADDR_MAX	0x7F
	#define IMX_OTP_DATA_ERROR_VAL	0xBADABADA
#endif

#define CONFIG_CMD_ENV

#define CONFIG_CMD_CLOCK
#define CONFIG_REF_CLK_FREQ CONFIG_MX6_HCLK_FREQ

#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY 0

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */
#define CONFIG_RD_LOADADDR	(CONFIG_LOADADDR + 0x300000)

#define CONFIG_BISTADDR		TEXT_BASE

#define CONFIG_BISTCMD		1
#define CONFIG_BISTCMD_LOCATION (CONFIG_BISTADDR - 0x70000)
#define CONFIG_BISTCMD_MAGIC	0xBC 

#define	CONFIG_EXTRA_ENV_SETTINGS \
    "testmem=mtest " MK_STR(PHYS_SDRAM_1) " 0x803E0000 0 1 2; mtest 0x80500000 0x8FFFFFFF 0 1 2\0" \
    "bootcmd=bootm " MK_STR(CONFIG_MMC_BOOTFLASH_ADDR) "\0" \
    "failbootcmd=panic\0" \
    "post_hotkeys=0\0" \
    "loglevel=5\0" \
    "bootcmd_diags=bootm " MK_STR(CONFIG_MMC_BOOTDIAGS_ADDR) "\0" \
    "bootcmd_fastboot=fastboot\0"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#undef CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_PROMPT		"bist > "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_ORG_MEMTEST      /* Original (not so) quickie memory test */
#define CONFIG_SYS_ALT_MEMTEST      /* Newer data, address, integrity test */
#define CONFIG_SYS_MEMTEST_SCRATCH  IRAM_BASE_ADDR      /* Internal RAM */
#define CONFIG_SYS_MEMTEST_START    PHYS_SDRAM_1	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END      (PHYS_SDRAM_1 + get_dram_size() - 1)
#define CONFIG_SYS_MEMPROTECT_START (CONFIG_SYS_INIT_SP_OFFSET & 0xFFF00000)
#define CONFIG_SYS_MEMPROTECT_END   ((_bss_end & 0xFFF00000) + 0x00100000 - 1)

#define CONFIG_BIST
#define CONFIG_CMD_DIAG

#define CONFIG_POST         (CONFIG_SYS_POST_MMC_CRC32 | \
                             CONFIG_SYS_POST_INVENTORY | \
                             CONFIG_SYS_POST_I2C | \
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
/* BEN TODO */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_DDR_BASE_ADDR
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

#define CONFIG_ENV_SECT_SIZE    (1 * 1024)
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_IS_NOWHERE

#ifndef __ASSEMBLY__
#include <asm/arch/mx60_wario_board.h>
#endif

#endif				/* __CONFIG_H */
