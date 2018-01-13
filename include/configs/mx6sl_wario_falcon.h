/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX6Q Armadillo2 Freescale board.
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

#ifndef __CONFIG_FALCON_H
#define __CONFIG_FALCON_H

/*
 * #define CONFIG_SECURE_BOOT
 *	Enable Secure Boot. DO NOT TURN ON IT until you know what you are doing
 */

#include <asm/arch/mx6.h>

/***********************************************************
 * Command definition
 ***********************************************************/

#define CONFIG_GZIP
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SPLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MISC

/* Enable below configure when supporting nand */
#define CONFIG_CMD_MMC

#define CONFIG_CMD_CLOCK
#define CONFIG_REF_CLK_FREQ CONFIG_MX6_HCLK_FREQ

#undef CONFIG_CMD_IMLS

#undef CONFIG_CMD_IMX_DOWNLOAD_MODE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_CMD_FB

/* 
 * If you move these around, make sure that sbios doesn't collide
 * with reserved EPDC working buffer 
 */
#define CONFIG_BIOS_LOADADDR		0x9f5f0000
#define CONFIG_BIOS_SIZE		0x00020000
#define CONFIG_SBIOS_LOADADDR		0x9f5e0000
#define CONFIG_SBIOS_SIZE		0x00010000

#define CONFIG_EXTRA_FB_ENV_SETTINGS	\
	"bios_addr="MK_STR(CONFIG_BIOS_LOADADDR)"\0" \
	"sbios_addr="MK_STR(CONFIG_SBIOS_LOADADDR)"\0" \
	"bank=0\0" \
	"boot=run bootcmd_FB; " \
		"run bootargs_base; setenv bootargs ${bootargs} " \
		"no_console_suspend ignore_loglevel root=/dev/fsd1 ro rootwait; " \
		"bootm ${loadaddr}\0" \
	"fb=run bootcmd_FB; fb\0" \
	"bootcmd_FB=mmc dev 1; mmc read ${sbios_addr} 0x0 0x80 2; " \
		"mmc read ${bios_addr} 0x10000 0xe0 2; " \
		"biosinit ${bios_addr} ${sbios_addr} ${bank}\0" \
	"bootargs=no_console_suspend rootwait console=ttymxc0,115200 root=/dev/mmcblk0p1 rw ip=off video=mxcepdcfb:E60,bpp=8,x_mem=3M\0"

/*
 * OCOTP Configs
 */
#ifdef CONFIG_CMD_IMXOTP
	#define CONFIG_IMX_OTP
	#define IMX_OTP_BASE			OCOTP_BASE_ADDR
	#define IMX_OTP_ADDR_MAX		0x7F
	#define IMX_OTP_DATA_ERROR_VAL	0xBADABADA
#endif

/*
 * SPI Configs
 */
#ifdef CONFIG_CMD_SF
	#define CONFIG_FSL_SF		1
	#define CONFIG_SPI_FLASH_IMX_M25PXX	1
	#define CONFIG_SPI_FLASH_CS	0
	#define CONFIG_IMX_ECSPI
	#define IMX_CSPI_VER_2_3	1
	#define MAX_SPI_BYTES		(64 * 4)
#endif

/*
 * MMC Configs
 */
#ifdef CONFIG_CMD_MMC
	#define CONFIG_SYS_MMC_ENV_DEV  1
	#define CONFIG_DOS_PARTITION	1
	#define CONFIG_CMD_FAT		1
	#define CONFIG_CMD_EXT2		1

	/* Setup target delay in DDR mode for each SD port */
	#define CONFIG_GET_DDR_TARGET_DELAY
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/* Monitor at beginning of flash */
#define CONFIG_FSL_ENV_IN_MMC
/* #define CONFIG_FSL_ENV_IN_NAND */
/* #define CONFIG_FSL_ENV_IN_SATA */

#define CONFIG_ENV_SECT_SIZE    (1 * 1024)
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_PARTITION    CONFIG_QB_PARTITION

#if defined(CONFIG_FSL_ENV_IN_MMC)
	#define CONFIG_ENV_IS_IN_MMC	1
        /* Config is in Boot Partition 2 */
	#define CONFIG_ENV_OFFSET	(CONFIG_MMC_FBIOS_ADDR + CONFIG_MMC_FBIOS_SIZE)
#elif defined(CONFIG_FSL_ENV_IN_SF)
	#define CONFIG_ENV_IS_IN_SPI_FLASH	1
	#define CONFIG_ENV_SPI_CS		1
	#define CONFIG_ENV_OFFSET       (768 * 1024)
#else
	#define CONFIG_ENV_IS_NOWHERE	1
#endif

#define CONFIG_SPLASH_SCREEN

/*
 * SPLASH SCREEN Configs
 */
#ifdef CONFIG_SPLASH_SCREEN



/* TODO: All of these locations and sizes should change.
 * The eventual goal is to have the working buffer and waveform buffer unified
 * with Linux's by putting them in a memory deadzone. For now, we will
 * initialize all of our buffers to the base of memory.
 */
#define GRAPHICS_MEMORY_BASE (CONFIG_LOADADDR)
#define CONFIG_MXC_EPDC
#define CONFIG_LCD
#define CONFIG_FB_BASE ( ALIGN( GRAPHICS_MEMORY_BASE, 0x200) ) 

// The framebuffer for our high res displays is currently about 1.5 MiB, make
// the memory region bigger to (hopefully) prevent problems when changing
// resolutions. Note: This is also used for CONFIG_SPLASH_(DE)COMPRESSED below
#define CONFIG_FB_SIZE (1024*1024*2)

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN (32 * 1024)
#define CONFIG_SYS_MALLOC_BASE (0x900000)

#define CONFIG_QUIET_ZLIB

// 24MB allocated at start sector 1773568.  DO NOT exceed 24MB or you will overwrite User Store.
#define SPLASH_MMC_OFFSET             (0x36200000)
#define SPLASH_KEYVALUE_OFFSET        (0)
#define SPLASH_WAVEFORM_OFFSET        (0x100000) /* 1 MiB */
#define SPLASH_WORKING_BUFFER_OFFSET  (0x200000) /* 2 MiB */
#define SPLASH_BMPGZ_OFFSET           (0xa00000) /* 10 MiB */
#define SPLASH_CRITGZ_OFFSET          (0xc00000) /* 12 MiB */

#ifdef CONFIG_MXC_EPDC
	#undef LCD_TEST_PATTERN
	#define LCD_BPP					LCD_MONOCHROME


	#define CONFIG_WAVEFORM_STATIC_MMC_DEVNO 1

	#define CONFIG_WAVEFORM_BUF_ADDR ( ALIGN( (CONFIG_FB_BASE + CONFIG_FB_SIZE), 0x200) )
	#define CONFIG_WAVEFORM_BUF_SIZE (0x800000) //8MB

	#define CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR ( ALIGN( (CONFIG_WAVEFORM_BUF_ADDR + CONFIG_WAVEFORM_BUF_SIZE), 0x200) )
	#define CONFIG_WAVEFORM_COMPRESSED_BUF_SIZE (0x100000) //1MB

	/* TODO: We need to figure out how to combine this waveform with the
	 * waveform data loaded by the kernel.
	 *
	 * This waveform data needs to be the data after the header, which
	 * consists of a struct waveform_data_header (see the linux kernel's
	 * mxc_epdc_fb.c for definition) and the variable length temperature
	 * table. On my panel this totaled to 63 bytes.
	 */
	#define CONFIG_WAVEFORM_FILE_OFFSET (SPLASH_MMC_OFFSET + SPLASH_WAVEFORM_OFFSET)
	#define CONFIG_WAVEFORM_FILE_SIZE (0x100000)
	#define CONFIG_WAVEFORM_FILE_IN_MMC

	#define CONFIG_HIBERNATE_HEADER_OFFSET (SPLASH_MMC_OFFSET + SPLASH_KEYVALUE_OFFSET)
	#define CONFIG_HIBERNATE_HEADER_LEN    (4096)
	#define CONFIG_HIBERNATE_HEADER_DEVNO  1

	#define CONFIG_SAVED_WB_OFFSET (SPLASH_MMC_OFFSET + SPLASH_WORKING_BUFFER_OFFSET)
	#define CONFIG_SAVED_WB_LEN    (0x300000) /* There is 8 MiB set aside for this, but only 3 needs to be loaded. */
	#define CONFIG_SAVED_WB_DEVNO  1
	#define CONFIG_SAVED_WB

	#define CONFIG_SPLASH_COMPRESSED
	#define CONFIG_SPLASH_COMPRESSED_DEVNO  1
	#define CONFIG_SPLASH_COMPRESSED_OFFSET (SPLASH_MMC_OFFSET + SPLASH_BMPGZ_OFFSET)

	#define CONFIG_SPLASH_COMPRESSED_ADDR ( ALIGN( (CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR + CONFIG_WAVEFORM_COMPRESSED_BUF_SIZE), 0x200) )
	#define CONFIG_SPLASH_COMPRESSED_SIZE (CONFIG_FB_SIZE) // assume entire FB full of white noise

	#define CONFIG_SPLASH_DECOMPRESSED_ADDR ( ALIGN( (CONFIG_SPLASH_COMPRESSED_ADDR + CONFIG_SPLASH_COMPRESSED_SIZE), 0x200) )
	#define CONFIG_SPLASH_DECOMPRESSED_SIZE (CONFIG_FB_SIZE)

#define CONFIG_SPLASH_CRIT_BATT_IMG_OFFSET (SPLASH_MMC_OFFSET + SPLASH_CRITGZ_OFFSET)
#define CONFIG_CRIT_IMG_SIZE (1024*1024)  /* Maximum size of the partition */

#define CONFIG_FB_MXC_EINK_WORK_BUFFER_RESERVED 1

#ifdef CONFIG_FB_MXC_EINK_WORK_BUFFER_RESERVED
	#define CONFIG_FB_MXC_EINK_WORK_BUFFER_ADDR 0x9F000000 //must match kernel config
	#define CONFIG_FB_MXC_EINK_WORK_BUFFER_SIZE 0x00400000 //must match kernel config
	#define CONFIG_WORKING_BUF_ADDR CONFIG_FB_MXC_EINK_WORK_BUFFER_ADDR
	#define CONFIG_WORKING_BUF_SIZE CONFIG_FB_MXC_EINK_WORK_BUFFER_SIZE
#else
	/*
	 * Make sure CONFIG_WORKING_BUF_ADDR is at the top of these allocations.
	 * It can be near the top of DDR and we don't want other allocations to grow past the end of memory.
	 */
	#define CONFIG_WORKING_BUF_ADDR ( ALIGN( (CONFIG_SPLASH_DECOMPRESSED_ADDR + CONFIG_SPLASH_DECOMPRESSED_SIZE), 0x200) )
	// The working buffer is 2x the frame buffer size because it needs
	// 16 bits per pixel, where the frame buffer is monochrome 8bpp.
	#define CONFIG_WORKING_BUF_SIZE (CONFIG_FB_SIZE * 2)
#endif // CONFIG_FB_MXC_EINK_WORK_BUFFER_RESERVED




#ifdef CONFIG_SPLASH_IS_IN_MMC
	#define CONFIG_SPLASH_IMG_OFFSET		0x4c000
	#define CONFIG_SPLASH_IMG_SIZE			0x19000
#endif
#endif
#endif /* CONFIG_SPLASH_SCREEN */

#endif				/* __CONFIG_FALCON_H */
