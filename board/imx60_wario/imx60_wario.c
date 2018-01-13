/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
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
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/mx6_pins.h>
#include <asm/arch/mx6sl_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/gpio.h>
#include <asm/errno.h>
#include <asm/arch/board-mx6sl_wario.h>

#include "wario_iomux/include/iomux_config.h"
#include "wario_iomux/include/iomux_define.h"
#include "wario_iomux/include/iomux_register.h"

#if defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc.h>
#endif

#ifdef CONFIG_IMX_ECSPI
#include <imx_spi.h>
#endif

#if CONFIG_I2C_MXC_LAB126
#include <i2c.h>
#endif

#ifdef CONFIG_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

#if defined (CONFIG_CMD_IDME)
#include <idme.h>
#endif

#if defined(CONFIG_PMIC)
#include <pmic.h>
#ifdef CONFIG_PMIC_MAX77696
#include <max77696_registers.h>
#include <pmic_max77696.h>
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

static u32 system_rev;
static enum boot_device boot_dev;
enum {
    BOOT_MODE_UNKNOWN = -1,
    BOOT_MODE_QBOOT = 1
};

/* board id and serial number. */
static u8 serial_number[BOARD_DSN_LEN+1];
static u8 board_id[BOARD_PCBA_LEN+1];

#ifdef CONFIG_VIDEO_MX5
extern unsigned char fsl_bmp_600x400[];
extern int fsl_bmp_600x400_size;
extern int g_ipu_hw_rev;

#if defined(CONFIG_BMP_8BPP)
unsigned short colormap[256];
#elif defined(CONFIG_BMP_16BPP)
unsigned short colormap[65536];
#else
unsigned short colormap[16777216];
#endif

static int di = 1;

extern int ipuv3_fb_init(struct fb_videomode *mode, int di,
			int interface_pix_fmt,
			ipu_di_clk_parent_t di_clk_parent,
			int di_clk_val);

static struct fb_videomode lvds_xga = {
	 "XGA", 60, 1024, 768, 15385, 220, 40, 21, 7, 60, 10,
	 FB_SYNC_EXT,
	 FB_VMODE_NONINTERLACED,
	 0,
};

vidinfo_t panel_info;

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);
#endif

#define HAPTIC_IQC_TEST_STR             "haptic_iqc"

/*
 * Disable power down counter event before the memory test
 * to avoid the wachdog reset itself after 16 secs.
 */
static void disable_watchdog_power_count_down_event(void);

void reset_touch_zforce2_pins(void);
static void set_unused_pins(void);
static void setup_touch_pins(void);
static void setup_bt_brcm_pins(void);

int board_info_valid (u8 *info, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	if ((info[i] < '0') &&
	    (info[i] > '9') &&
	    (info[i] < 'A') &&
	    (info[i] > 'Z'))
	    return 0;
    }

    return 1;
}

extern int check_haptic_and_recovery(void); 
void config_board_100k_pullup(void)
{
    __raw_writel((LVE_ENABLED & 0x1) << 22 | (HYS_ENABLED & 0x1) << 16 | (PUS_100KOHM_PU & 0x3) << 14 |
                 (PUE_PULL & 0x1) << 13 | (PKE_ENABLED & 0x1) << 12 | (ODE_DISABLED & 0x1) << 11 |
                 (SPD_100MHZ & 0x3) << 6 | (DSE_40OHM & 0x7) << 3 | (SRE_SLOW & 0x1), IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE1);
    printf(" imx6 register 0x20E03F4 = 0x%8x\n", __raw_readl(IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE1));
}

/*************************************************************************
 * get_board_serial() - setup to pass kernel serial number information
 *      return: alphanumeric containing the serial number.
 *************************************************************************/
const u8 *get_board_serial(void)
{
    if (!board_info_valid(serial_number, BOARD_DSN_LEN))
	return (u8 *) "0000000000000000";
    else
	return serial_number;
}

/*************************************************************************
 * get_board_id16() - setup to pass kernel board revision information
 *      16-byte alphanumeric containing the board revision.
 *************************************************************************/
const u8 *get_board_id16(void)
{
    if (!board_info_valid(board_id, BOARD_PCBA_LEN))
	return (u8 *) "0000000000000000";
    else
	return board_id;
}

/*************************************************************************
 * Get the partition info for this device
 *************************************************************************/
const struct partition_info_t *get_partition_info_for_device(void)
{
	const char *bid = (char*)get_board_id16();
	if (BOARD_IS_PINOT(bid) || BOARD_IS_PINOT_WFO(bid))
		return partition_info_350;
	return partition_info_default;
}

void setup_board_gpios(void)
{
	const char *bid = (char*)get_board_id16();

	/* drive reset gpio low to keep touch @ reset */
	gpio_direction_output( IMX_GPIO_NR(4, 5) , 0); //KEY_ROW6

	gpio_direction_output( IMX_GPIO_NR(1, 15) , 1); //SDDO8
	gpio_direction_output( IMX_GPIO_NR(1, 16) , 1); //SDDO9
	
	iomux_config();
	set_unused_pins();
	//Keeper circuit soln.
	gpio_direction_output(IMX_GPIO_NR(1,22), 1);

	//GPIO(pin EPDC_SDCE1) for PMIC LPM - disabled by default
	gpio_direction_output(IMX_GPIO_NR(1,28), 0);

	//some delay to stabilise
	udelay(2000);

	/* do the settings here so that board checks take into effect */
	if ( BOARD_IS_BOURBON(bid) || 
		BOARD_IS_WARIO_4_256M_CFG_C(bid) ) {
		reset_touch_zforce2_pins();
	} else {
		setup_touch_pins();
	}

	if (BOARD_IS_WHISKY_WAN(bid) || BOARD_IS_WHISKY_WFO(bid)	||
		BOARD_IS_WOODY(bid) ) {

		setup_bt_brcm_pins();

#if defined(CONFIG_WARIO_WOODY)
		gpio_direction_output( MX6SL_WARIO_WL_GPIO_1 , 0);      //SD1_DAT1
		gpio_direction_output( MX6SL_WARIO_WL_REG_ON , 0);      //LCD_CLK
		gpio_direction_output( MX6SL_WARIO_BT_REG_ON , 0);      //LCD_RESET
		gpio_direction_output( MX6SL_WARIO_3GM_POWER_ON, 0);    //EDPC_PWRCTRL3
                gpio_direction_input( MX6SL_WARIO_3GM_FW_READY);        //KEY_COL3
                gpio_direction_output( MX6SL_WARIO_3GM_DL_KEY, 0);      //KEY_COL5
                gpio_direction_output( MX6SL_WARIO_3GM_WAKEUP_MDM, 1);  //FEC_REF_CLK
                gpio_direction_input( MX6SL_WARIO_3GM_WAKEUP_AP);       //KEY_ROW4
                gpio_direction_output( MX6SL_WARIO_3GM_SAR_DET, 1);     //KEY_COL2

		/* init soda gpios */
		gpio_direction_input(MX6_SODA_CHG_DET);
		gpio_direction_output(MX6_SODA_I2C_SCL, 1);
		gpio_direction_output(MX6_SODA_I2C_SDA, 1);
		gpio_direction_output(MX6_SODA_I2C_SDA_PU, 1);
		gpio_direction_output(MX6_SODA_BOOST, 0);
		gpio_direction_output(MX6_SODA_OTG_SW, 0);
		gpio_direction_output(MX6_SODA_VBUS_ENABLE, 0);

		soft_i2c_init();
#endif
	}

}

int setup_board_info(void)
{
#if defined(CONFIG_CMD_IDME)
	int do_clear_old_idme = idme_check_update();

#ifdef CONFIG_QBOOT
	{
		char bootmode[BOARD_BOOTMODE_LEN + 1];
		if (!idme_get_var("bootmode", bootmode, sizeof(bootmode)) &&
			!strncmp(bootmode, "qboot", 5)) {
                        puts("QBOOT\n");
			gd->flags |= GD_FLG_QUICKBOOT;

			// userland will continue using the updated area, don't clear it.
			do_clear_old_idme = 0;
		} else {
			gd->flags &= ~GD_FLG_QUICKBOOT;
		}
	}
#endif
	if (do_clear_old_idme) idme_clear_update();

	if (idme_get_var("pcbsn", (char *) board_id, sizeof(board_id)))
#endif
	{
		/* not found: clean up garbage characters. */
		memset(board_id, 0, sizeof(board_id));
    	}

#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("serial", (char *) serial_number, sizeof(serial_number)))
#endif
	{
		/* not found: clean up garbage characters. */
		memset(serial_number, 0, sizeof(serial_number));
	}

    setup_board_gpios();
    return 0;
}

static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = ONE_NAND_BOOT;
		else
			boot_dev = WEIM_NOR_BOOT;
		break;
	case 0x2:
			boot_dev = SATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = I2C_BOOT;
		else
			boot_dev = SPI_NOR_BOOT;
		break;
	case 0x4:
	case 0x5:
		boot_dev = SD_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = NAND_BOOT;
		break;
	default:
		boot_dev = UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void)
{
	return boot_dev;
}

u32 get_board_rev(void)
{

#if defined CONFIG_MX6Q
	system_rev = 0x63000;
#elif defined CONFIG_MX6DL
	system_rev = 0x61000;
#else
	system_rev = 0x60000;
#endif
	return system_rev;
}


static const struct board_type *get_board_type(void)
{
    int i;

    if (!board_info_valid(board_id, BOARD_PCBA_LEN)) {
	printf("board inf invalid: %s\n", board_id);
	return NULL;
    }

    for (i = 0; i < NUM_KNOWN_BOARDS; i++) {
	if (strncmp((const char *) board_id, boards[i].id, strlen(boards[i].id)) == 0) {
	    return &(boards[i]);
	}
    }

    return NULL;
}

unsigned int get_dram_size(void)
{
    int i;
    unsigned int size = 0;

    for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
	size += gd->bd->bi_dram[i].size;
    }

    return size;
}

#ifdef CONFIG_ARCH_MMU
int board_mmu_init(void)
{
        unsigned long ttb_base = PHYS_SDRAM_1           /* DDR base */
                                 + PHYS_SDRAM_1_SIZE    /* DDR size */
                                 - 0x9F0000             /* Reserved area */
                                 - 0x00004000;          /* TTB size */
	unsigned long i;

	/*
	 * Set the TTB register
	 */
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r" (ttb_base) /*: */);

	/*
	 * Set the Domain Access Control Register
	 */
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r" (i) /*: */);

	/*
	 * First clear all TT entries - ie Set them to Faulting
	 */
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes          Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
//	X_ARM_MMU_SECTION(0x000, 0x000, 0x001, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);	/* ROM, 1M */
	X_ARM_MMU_SECTION(0x009, 0x009, 0x001, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);	/* IRAM */
	X_ARM_MMU_SECTION(0x00A, 0x00A, 0x0F6, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);	/* 246M */
	/* 512MB memory starting at 0x80000000 */
	X_ARM_MMU_SECTION(0x800, 0x800, 0x200,
			  ARM_CACHEABLE, ARM_BUFFERABLE, ARM_ACCESS_PERM_RW_RW);

        // This is needed if we allow interrupts and exceptions in U-boot.
	extern void * _start;
	memset((void *)PHYS_SDRAM_1, 0, 0x80);
	memcpy((void *)PHYS_SDRAM_1, &_start, 16*4);

	X_ARM_MMU_SECTION(0x800, 0x0, 0x001, ARM_CACHEABLE, ARM_BUFFERABLE, ARM_ACCESS_PERM_RW_RW);	/* RAM(vector table), 1M */

//	X_ARM_MMU_SECTION(0x340, 0xE40, 0x0c0,
//			  ARM_CACHEABLE, ARM_BUFFERABLE,
//			  ARM_ACCESS_PERM_RW_RW);

	/* Enable MMU */
	cache_flush();
	MMU_ON();
        return 0;
}
#endif

#ifdef CONFIG_FOR_FACTORY
static char boardid_input_buf[CONFIG_SYS_CBSIZE];
extern int readline_into_buffer (const char *const prompt, char * buffer);
#endif

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;

	while(1) {
		const struct board_type *board;

		board = get_board_type();

	    	if (board) {

#ifdef CONFIG_FOR_FACTORY
			gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#else
			gd->bd->bi_dram[0].size = board->mem_size;
#endif

#ifdef CONFIG_IRAM_BOOT
	      		switch (board->mem_type) {
	        		case MEMORY_TYPE_LPDDR2:
          	    		if (board->mem_size == MEMORY_SIZE_256MB || board->mem_size == MEMORY_SIZE_512MB) {
                        		lpddr2_init(board);
		    		} else {
		        		printf("Error! invalid memory config!\n");
		    		}
		    		break;
	        		default:
		    			printf("Error! unsupported memory type!\n");
	    		}
#endif
			return 0;
	   	}

#ifdef CONFIG_FOR_FACTORY
		/* Clear out buffer */
		memset(boardid_input_buf, 0, sizeof(boardid_input_buf));

		printf("Board ID is invalid!  Please enter a valid board id:\n");
		readline_into_buffer(">", boardid_input_buf);

		if (strlen(boardid_input_buf) != BOARD_PCBA_LEN) {
			printf("\nError! Board ID must be %d chars long.\n\n", BOARD_PCBA_LEN);
			continue;
		}

		idme_update_var("pcbsn", boardid_input_buf);

		/* Set bootmode to diags if this is the first boot */
		idme_update_var("bootmode", "uboot");
#else
		printf("Invalid board id!  Can't determine system type for RAM init.. bailing!\n");
		return 0;
#endif /* CONFIG_FOR_FACTORY */

	}

	return 0;
}

static void setup_uart(void)
{
	uart1_iomux_config();
}

#if defined (CONFIG_I2C_MXC_LAB126)
void setup_i2c(unsigned int module_base)
{
	unsigned int reg;

	switch (module_base) {
	case I2C1_BASE_ADDR:
		i2c1_iomux_config();

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC0;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);
		break;
	case I2C2_BASE_ADDR:

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0x300;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);
		break;
	case I2C3_BASE_ADDR:
		/* GPIO_3 for I2C3_SCL */
		mxc_iomux_v3_setup_pad(MX6SL_PAD_EPDC_SDCE2__I2C3_SCL);
		/* GPIO_6 for I2C3_SDA */
		mxc_iomux_v3_setup_pad(MX6SL_PAD_EPDC_SDCE3__I2C3_SDA);
		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC00;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);
		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}
}
#endif

#ifdef CONFIG_NET_MULTI
int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

	return rc;
}
#endif

#ifdef CONFIG_MMC

/* On this board, only SD3 can support 1.8V signalling
 * that is required for UHS-I mode of operation.
 * Last element in struct is used to indicate 1.8V support.
 */
struct fsl_esdhc_cfg usdhc_cfg[3] = {
    {USDHC1_BASE_ADDR, 1, 1, 52000000, 0, 0},
    {USDHC2_BASE_ADDR, 1, 1, 52000000, 0, 0},
    {USDHC3_BASE_ADDR, 1, 1, 52000000, 0, 0},
};

#ifdef CONFIG_DYNAMIC_MMC_DEVNO
int get_mmc_env_devno(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);

	if (SD_BOOT == boot_dev || MMC_BOOT == boot_dev) {
		/* BOOT_CFG2[3] and BOOT_CFG2[4] */
		return (soc_sbmr & 0x00001800) >> 11;
	} else
		return -1;

}
#endif

#define MX6_WARIO_WIFI_PWD      IMX_GPIO_NR(3, 29)      /* KEY_ROW2 */

#ifdef CONFIG_SAVED_WB
int get_hibernate_property(const char *propname, char *buffer, size_t buflen) {
#ifdef CONFIG_HIBERNATE_HEADER_OFFSET
	static char hibernate_header[CONFIG_HIBERNATE_HEADER_LEN] __attribute__ ((aligned (4096)));
	static int header_loaded = 0;
	uint blk_start, blk_cnt, n;
	const char *itr;
	unsigned int copied;
	struct mmc *mmc = find_mmc_device(CONFIG_HIBERNATE_HEADER_DEVNO);


	if (!header_loaded) {
		header_loaded = 1;

		if (mmc_init(mmc)) {
			*buffer = '\0';
			puts("MMC init failed\n");
		} else {
			blk_start = ALIGN(CONFIG_HIBERNATE_HEADER_OFFSET, mmc->read_bl_len) / mmc->read_bl_len;
			blk_cnt   = ALIGN(CONFIG_HIBERNATE_HEADER_LEN, mmc->read_bl_len) / mmc->read_bl_len;
			n = mmc->block_dev.block_read(CONFIG_HIBERNATE_HEADER_DEVNO, blk_start, blk_cnt, (u_char *)hibernate_header);

			if (n != blk_cnt) {
				hibernate_header[0] = '\0';
				printf("Failed to load hibernate header.\n");
			} else {
				int nl_count = 0;
				int i;

				hibernate_header[CONFIG_HIBERNATE_HEADER_LEN - 1] - '\0';

				for (i=0; i<CONFIG_HIBERNATE_HEADER_LEN; i++) {
					if (hibernate_header[i] == '\n') {
						nl_count += 1;
					} else {
						nl_count = 0;
					}

					if (nl_count == 2) {
						hibernate_header[i] = '\0';
						break;
					}
				}
			}
		}
	}

	itr = hibernate_header;
	while(itr && *itr) {
		itr = strstr(hibernate_header, propname);

		if (itr && (itr == hibernate_header || itr[-1]=='\n') && itr[strlen(propname)]=='=') {
			itr += strlen(propname) + 1;
			copied = 0;

			while (*itr != '\0' && *itr != '\n') {
				if (copied < buflen-1) {
					buffer[copied] = *itr;
					copied += 1;
				}
				itr += 1;
			}
			buffer[copied] = '\0';

			return 1;
		}
	}
#endif /* CONFIG_HIBERNATE_HEADER_OFFSET */
	*buffer = '\0';
	return 0;
}

/* Read a hibernate property and parse it into an unsigned long. The value from the property is returned in *value. The
 * function will return 0 on success, and -1 on failure. In the event of a failure *value will be set to default_val.
 */
int get_hibernate_property_ul(const char *propname, unsigned long *value, unsigned long default_val) {
	char buffer[20];
	char *endp;

	get_hibernate_property(propname, buffer, ARRAY_SIZE(buffer));

	*value = simple_strtol(buffer, &endp, 10);

	if (buffer[0] != '\0' && *endp == '\0') {
		return 0;
	} else {
		*value = default_val;
		return -1;
	}
}
#endif /* CONFIG_SAVED_WB */

int usdhc_gpio_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;
	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
		++index) {
		switch (index) {
		case 0:
			usdhc1_iomux_config();
			break;
		case 1:
			usdhc2_iomux_config();
			break;
		case 2:
		{
			u32 reg;

			usdhc3_iomux_config();

			/* Set up for 1.8 V operation */
			reg = readl(USDHC3_BASE_ADDR + VENDORSPEC);
			reg |= VENDORSPEC_VSELECT;
			writel(reg, USDHC3_BASE_ADDR + VENDORSPEC);

#if defined(CONFIG_CMD_MMC)
			/* enable wifi chip power */
#if defined(CONFIG_WARIO_BASE)
			gpio_direction_output(MX6_WARIO_WIFI_PWD, 1);
#elif defined(CONFIG_WARIO_WOODY)
	/* TODO: check power in lpm mode after enabling WIFI/BT brcm chip */
#endif
#endif
			break;
		}
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index+1, CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}

	return status;
}

int board_mmc_init(bd_t *bis)
{
	if (usdhc_gpio_init(bis)) {
		return -1;
	}

	return 0;
}

#ifdef CONFIG_SPLASH_COMPRESSED
static int setup_bmp_image(void) {
	uint blk_start, blk_cnt, n;
	ulong addr, size;
	char img_len[50];

	struct mmc *mmc = find_mmc_device(CONFIG_SPLASH_COMPRESSED_DEVNO);
	if (!mmc) {
		printf("MMC Device %d not found\n",
			CONFIG_SPLASH_COMPRESSED_DEVNO);
		return -1;
	}
	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}
#ifdef CONFIG_SAVED_WB
	get_hibernate_property("img_len", img_len, sizeof(img_len));
	size = simple_strtoul(img_len, NULL, 10);
#else
	size = 0;
#endif /* CONFIG_SAVED_WB */

	if (size <= 0) {
		int is_qb = gd->flags & GD_FLG_QUICKBOOT;
		gd->flags &= ~GD_FLG_QUICKBOOT;
		printf("Splash image invalid.  Size: %d\n", size);
		if (is_qb) gd->flags |= GD_FLG_QUICKBOOT;
		return -1;
	}

	if (size > CONFIG_SPLASH_COMPRESSED_SIZE) {
		int is_qb = gd->flags & GD_FLG_QUICKBOOT;
		gd->flags &= ~GD_FLG_QUICKBOOT;
		printf("Splash image too large (will be corrupted).  Size: %d\n", size);
		if (is_qb) gd->flags |= GD_FLG_QUICKBOOT;
		// continue on and display corrupted splash image
	}

	addr = CONFIG_SPLASH_COMPRESSED_ADDR;
	blk_start = ALIGN(CONFIG_SPLASH_COMPRESSED_OFFSET, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	mmc_read(CONFIG_SPLASH_COMPRESSED_DEVNO,CONFIG_SPLASH_COMPRESSED_OFFSET,addr,blk_cnt * mmc->read_bl_len);

	gunzip((void *)CONFIG_SPLASH_DECOMPRESSED_ADDR, CONFIG_SPLASH_DECOMPRESSED_SIZE, (unsigned char *)CONFIG_SPLASH_COMPRESSED_ADDR, &size);

	lcd_display_bitmap(CONFIG_SPLASH_DECOMPRESSED_ADDR, 0, 0);

	return 0;
}
#endif

#ifdef CONFIG_SPLASH_SCREEN
/*
 * Blit the spash image to the framebuffer
 * The framebuffer is fb_widt x fb_height. Blit your image to 0,0 in the frame
 * buffer, and report in *x0 and *y0 where it should be rendered.
 */
void blit_splash_img(void *fb, int fb_width, int fb_height, int *x0, int *y0, int *width, int *height) {
	int r, c;
	unsigned long x = 0, y = 0, w = 0, h = 0;
	unsigned int reg;
#ifdef CONFIG_SAVED_WB
	get_hibernate_property_ul("img_x", &x, 0);
	get_hibernate_property_ul("img_y", &y, 0);

	get_hibernate_property_ul("img_w", &w, 0);
	get_hibernate_property_ul("img_h", &h, 0);
#endif /* CONFIG_SAVED_WB */
	
#ifdef CONFIG_SPLASH_COMPRESSED
	setup_bmp_image();
#else
	for (r=0; r<h; r++) {
		for (c=0; c<w; c++) {
			((char *)fb)[r*fb_width + c] = (r+c) % 256 < 16 ? 0 : 0xf0;
		}
	}
#endif

	*x0 = x;
	*y0 = y;
	*width = w;
	*height = h;
}
#endif

#ifdef CONFIG_SPLASH_COMPRESSED
static int setup_crit_batt_image(void) {
	uint blk_start, blk_cnt, n;
	ulong addr, size;
	char img_len[50];

	struct mmc *mmc = find_mmc_device(CONFIG_SPLASH_COMPRESSED_DEVNO);
	if (!mmc) {
		printf("MMC Device %d not found\n",
			CONFIG_SPLASH_COMPRESSED_DEVNO);
		return -1;
	}
	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}
#ifdef CONFIG_SAVED_WB
	get_hibernate_property("crit_len", img_len, sizeof(img_len));
	size = simple_strtoul(img_len, NULL, 10);
#else
	size = 0;
#endif /* CONFIG_SAVED_WB */

	if (size <= 0) {
		int is_qb = gd->flags & GD_FLG_QUICKBOOT;
		gd->flags &= ~GD_FLG_QUICKBOOT;
		printf("Crit Batt image invalid.  Size: %d\n", size);
		if (is_qb) gd->flags |= GD_FLG_QUICKBOOT;
		return -1;
	}

	if (size > CONFIG_CRIT_IMG_SIZE) {
		int is_qb = gd->flags & GD_FLG_QUICKBOOT;
		gd->flags &= ~GD_FLG_QUICKBOOT;
		printf("Crit Batt image too large (will be corrupted).  Size: %d\n", size);
		if (is_qb) gd->flags |= GD_FLG_QUICKBOOT;
		// continue on and display corrupted splash image
	}

	addr = CONFIG_SPLASH_COMPRESSED_ADDR;
	blk_cnt   = (size + mmc->read_bl_len - 1) / mmc->read_bl_len;

        printf("%s: Read from eMMC addr 0x%08x, %d blocks, size %d\n", __func__, CONFIG_SPLASH_CRIT_BATT_IMG_OFFSET, blk_cnt, size);
	mmc_read(CONFIG_SPLASH_COMPRESSED_DEVNO,CONFIG_SPLASH_CRIT_BATT_IMG_OFFSET,addr,blk_cnt * mmc->read_bl_len);

	gunzip((void *)CONFIG_SPLASH_DECOMPRESSED_ADDR, CONFIG_SPLASH_DECOMPRESSED_SIZE, (unsigned char *)CONFIG_SPLASH_COMPRESSED_ADDR, &size);

	lcd_display_bitmap(CONFIG_SPLASH_DECOMPRESSED_ADDR, 0, 0);

	return 0;
}
#endif

/*
 * Blit the critical battery image to the framebuffer
 * The framebuffer is fb_width x fb_height. Blit the image to 0,0 in the frame
 * buffer, and report in *x0 and *y0 where it should be rendered.
 */
void blit_crit_batt_img(void *fb, int fb_width, int fb_height, int *x0, int *y0, int *width, int *height) {
	int r, c;
	unsigned long x = 0, y = 0, w = 0, h = 0;
	unsigned int reg;
#ifdef CONFIG_SAVED_WB
	get_hibernate_property_ul("crit_x", &x, 0);
	get_hibernate_property_ul("crit_y", &y, 0);

	get_hibernate_property_ul("crit_w", &w, 0);
	get_hibernate_property_ul("crit_h", &h, 0);
#endif /* CONFIG_SAVED_WB */
	
#ifdef CONFIG_SPLASH_COMPRESSED
	setup_crit_batt_image();
#else
	for (r=0; r<h; r++) {
		for (c=0; c<w; c++) {
			((char *)fb)[r*fb_width + c] = (r+c) % 256 < 16 ? 0 : 0xf0;
		}
	}
#endif

	*x0 = x;
	*y0 = y;
	*width = w;
	*height = h;
}

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
int setup_splash_img()
{
#ifdef CONFIG_SPLASH_IS_IN_MMC
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_SPLASH_IMG_OFFSET;
	ulong size = CONFIG_SPLASH_IMG_SIZE;
	ulong addr = 0;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	s = getenv("splashimage");

	if (NULL == s) {
		puts("env splashimage not found!\n");
		return -1;
        }
	addr = simple_strtoul(s, NULL, 16);

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return  -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
					blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#endif
	return -1;
}
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 1072,
	.vl_row = 1448,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 92,
	.vl_upper_margin = 4,
	.vl_lower_margin = 7,
	.vl_hsync = 8,
	.vl_vsync = 2,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	cmap:0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 464,
	.gdsp_offs = 451,
	.gdoe_offs = 0,
	.gdclk_offs = 127,
	.num_ce = 1,
};

int board_is_hall_closed (void)
{
	int rval,hall_val;
	gpio_direction_input(MX6_WARIO_HALL_SNS);
	hall_val = gpio_get_value(MX6_WARIO_HALL_SNS);
#ifdef CONFIG_WARIO_WOODY
	return hall_val? 1:0;
#else
	return !hall_val? 1:0;
#endif
}

static void setup_epdc_power()
{
	unsigned int reg;

	/* Set as output */
	reg = readl(GPIO2_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 3);
	writel(reg, GPIO2_BASE_ADDR + GPIO_GDIR);

	/* Set as output */
	reg = readl(GPIO2_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 14);
	writel(reg, GPIO2_BASE_ADDR + GPIO_GDIR);

	/* Set as output */
	reg = readl(GPIO2_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 7);
	writel(reg, GPIO2_BASE_ADDR + GPIO_GDIR);
}

void epdc_power_on()
{
	unsigned int reg;
	char vcom_buffer[50];
	char *endp;

#ifdef CONFIG_PMIC_MAX77696
	pmic_init();
#ifdef CONFIG_SAVED_WB
	get_hibernate_property("vcom", vcom_buffer, sizeof(vcom_buffer));
#else
	vcom_buffer[0] = '\0';
#endif /* CONFIG_SAVED_WB */
	if (vcom_buffer[0] != '\0') {
		pmic_vcom_set(vcom_buffer); 
	} else {
		pmic_vcom_set("-2500"); 
	}

	pmic_vddh_set_int(panel_info.vl_vddh);

#else
	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg |= (1 << 7);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	/* Set PMIC Wakeup to high - enable Display power */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg |= (1 << 14);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
		if (!(reg & (1 << 13)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg |= (1 << 3);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);

	udelay(500);
#endif // CONFIG_PMIC_MAX77696
}

void  epdc_power_off()
{
	unsigned int reg;

#ifdef CONFIG_PMIC_MAX77696
        printf("Calling pmic_enable_epdc\n");
        pmic_enable_epdc(0);
#else
	/* Set PMIC Wakeup to low - disable Display power */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 14);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	/* Disable VCOM */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 3);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 7);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);
#endif // CONFIG_PMIC_MAX77696
}

#ifdef CONFIG_SPLASH_SCREEN
struct wfm_header {
	unsigned int checksum:32;
	unsigned int file_length:32;
	unsigned int serial_number:32;
	unsigned int run_type:8;
	unsigned int fpl_platform:8;
	unsigned int fpl_lot:16;
	unsigned int mode_version:8;
	unsigned int wf_version:8;
	unsigned int wf_subversion:8;
	unsigned int wf_type:8;
	unsigned int panel_size:8;
	unsigned int amepd_part_number:8;
	unsigned int wf_revision:8;
	unsigned int frame_rate:8;
	unsigned int reserved1_0:8;
	unsigned int vcom_shifted:8;
	unsigned int reserved1_1:16;

	unsigned int xwia:24;
	unsigned int cs1:8;

	unsigned int wmta:24;
	unsigned int fvsn:8;
	unsigned int luts:8;
	unsigned int mc:8;
	unsigned int trc:8;
	unsigned int advanced_wfm_flags:8;
	unsigned int eb:8;
	unsigned int sb:8;
	unsigned int reserved0_1:8;
	unsigned int reserved0_2:8;
	unsigned int reserved0_3:8;
	unsigned int reserved0_4:8;
	unsigned int reserved0_5:8;
	unsigned int cs2:8;
};
#endif

int setup_waveform_file()
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
#if defined(CONFIG_WAVEFORM_STATIC_MMC_DEVNO)
	int mmc_dev = CONFIG_WAVEFORM_STATIC_MMC_DEVNO;
#else
	int mmc_dev = get_mmc_env_devno();
#endif
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong wfm_addr = CONFIG_WAVEFORM_BUF_ADDR;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;
	char wfm_format[50], wfm_len[50], wfm_addr_string[20];
	struct wfm_header *hdr = CONFIG_WAVEFORM_BUF_ADDR;

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}
#ifdef CONFIG_SAVED_WB
	if (!get_hibernate_property("wfm_format", wfm_format, sizeof(wfm_format)) ||
			!get_hibernate_property("wfm_len", wfm_len, sizeof(wfm_len)) ||
			!get_hibernate_property("wfm_addr", wfm_addr_string, sizeof(wfm_addr_string))) {
		printf("Unable to set up waveform! (%s, %s, %s)\n", wfm_format, wfm_len, wfm_addr);
		return -1;
	}
	wfm_addr = simple_strtoul(wfm_addr_string, NULL, 16);
	size = simple_strtoul(wfm_len, NULL, 10);
	if (size <= 0 || size > CONFIG_WAVEFORM_BUF_SIZE || !iomem_valid_addr(wfm_addr, size)) {
		printf("Invalid waveform spec: 0x%x, %d\n", wfm_addr, size);
	}
#else
	wfm_format[0] = '\0';
	wfm_len[0] = '\0';
#endif /* CONFIG_SAVED_WB */

#ifdef CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR
	if (strcmp(wfm_format,"compressed") == 0) {
		blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
		blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

		n = mmc_read(mmc_dev, offset, CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR, blk_cnt * mmc->read_bl_len);

		if (n == 0) {
			ssize_t offset, start;

			gunzip((void *)(CONFIG_WAVEFORM_BUF_ADDR), sizeof(struct wfm_header),
			       (unsigned char *)(CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR), &size);

			/* The waveform data is followed by a list of temperature entries that is trc+1 bytes long.
			 * After that list, there is 1 extra byte before the actual waveform data starts.
			 */
			offset = -(sizeof(struct wfm_header) + hdr->trc + 2);

			start = 0;
			while (offset < 0){
				offset += 512;
				start += 512;
			}

			size = simple_strtoul(wfm_len, NULL, 10);
			gunzip((void *)(CONFIG_WAVEFORM_BUF_ADDR + offset), CONFIG_WAVEFORM_BUF_SIZE-offset,
			       (unsigned char *)(CONFIG_WAVEFORM_COMPRESSED_BUF_ADDR), &size);

			#ifdef CONFIG_SAVED_WB
				/*
				 * use the same waveform addr that linux uses, so 
				 * its not clobbered by quickboot preload
				 */
				memcpy(wfm_addr, CONFIG_WAVEFORM_BUF_ADDR + start, CONFIG_WAVEFORM_BUF_SIZE-offset);
			#else
				//use the static reserved one, offset into the actual waveform data.
				wfm_addr += start;
			#endif

			panel_info.epdc_data.waveform_buf_addr = wfm_addr;
		}
		else {
			printf("Compressed waveform could not be loaded from the disk\n");
		}
	} else
#endif
	{
		blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
		blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

		n = mmc_read(mmc_dev, offset, wfm_addr, size);
	}


	cache_flush();

	return n;
#else
	return -1;
#endif
}

#ifdef CONFIG_SAVED_WB
int load_working_buffer(void) {
	uint blk_start, blk_cnt, n;
	ulong addr;

	struct mmc *mmc = find_mmc_device(CONFIG_SAVED_WB_DEVNO);
	if (!mmc) {
		printf("MMC Device %d not found\n",
			CONFIG_SAVED_WB_DEVNO);
		return -1;
	}
	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	addr = CONFIG_WORKING_BUF_ADDR;
	blk_start = ALIGN(CONFIG_SAVED_WB_OFFSET, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(CONFIG_SAVED_WB_LEN, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc_read(CONFIG_SAVED_WB_DEVNO,CONFIG_SAVED_WB_OFFSET,addr,CONFIG_SAVED_WB_LEN);
	return n;
}
#endif

void setup_epdc()
{
	unsigned int reg;
	char barcode_prefix[20];
#ifdef CONFIG_SAVED_WB
	get_hibernate_property("panel_barcode_prefix", barcode_prefix, sizeof(barcode_prefix));
	mxc_epdc_vidinfo_for_barcode(barcode_prefix, &panel_info, &panel_timings);
#endif
	/* epdc iomux settings */
	epdc_iomux_config();

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0x3F000;
	reg |= (0x4 << 15) | (1 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);

	/* EPDC AXI clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x0030;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	/* EPDC PIX clk from PFD_540M, set to 540/4/5 = 27MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR2);
	reg &= ~0x03F000;
	reg |= (0x5 << 15) | (4 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR2);

	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
	reg &= ~0x03800000;
	reg |= (0x3 << 23);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCMR);

	/* EPDC PIX clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x0C00;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;


	setup_epdc_power();
}
#endif

/* For DDR mode operation, provide target delay parameter for each SD port.
 * Use cfg->esdhc_base to distinguish the SD port #. The delay for each port
 * is dependent on signal layout for that particular port.  If the following
 * CONFIG is not defined, then the default target delay value will be used.
 */
#ifdef CONFIG_GET_DDR_TARGET_DELAY
u32 get_ddr_delay(struct fsl_esdhc_cfg *cfg)
{
	/* No delay required on ARM2 board SD ports */
	return 0;
}
#endif
#endif

#ifdef CONFIG_IMX_ECSPI
s32 board_spi_get_cfg(struct imx_spi_dev_t *dev)
{
	switch (dev->slave.cs) {
	case 0:
		/* Panel flash */
		dev->base = ECSPI1_BASE_ADDR;
		dev->freq = 1000000;
		dev->ss_pol = IMX_SPI_ACTIVE_LOW;
		dev->ss = 0;
		dev->fifo_sz = 64 * 4;
		dev->us_delay = 0;
		dev->version = IMX_SPI_VERSION_2_3;
		break;
	default:
		printf("Invalid Bus ID!\n");
		break;
	}

	return 0;
}

void board_spi_io_init(struct imx_spi_dev_t *dev)
{
	u32 reg;

	switch (dev->base) {
	case ECSPI1_BASE_ADDR:
		/* Enable clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR1);
		reg |= 0x3;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR1);

		ecspi1_iomux_config();
		break;
	case ECSPI2_BASE_ADDR:
	case ECSPI3_BASE_ADDR:
		/* ecspi2-3 fall through */
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_IRAM_BOOT
/* should be called after RAM init and POST */
u8 get_ddr_mfginfo(void)
{
	u32 mapsr, madpcr0, mdscr, mdmrr;
	u8 mfgid = 0x0;
	const char *rev = (const char *) get_board_id16();

#define MMDC0_MAPSR (MMDC_P0_BASE_ADDR + 0x0404)
#define MMDC0_MADPCR0 (MMDC_P0_BASE_ADDR + 0x0410)
#define MMDC0_MDSCR (MMDC_P0_BASE_ADDR + 0x001C)
#define MMDC0_MDMRR (MMDC_P0_BASE_ADDR + 0x0034)

	// save registers
	mapsr = __raw_readl(MMDC0_MAPSR);
	madpcr0 = __raw_readl(MMDC0_MADPCR0);
	mdscr = __raw_readl(MMDC0_MDSCR);

	//disable MMDC automatic power savings
	__raw_writel(0x1, MMDC0_MAPSR);
	//set SBS_EN bit
	__raw_writel(0x100, MMDC0_MADPCR0);
	//set CON_REQ bit
	__raw_writel(0x8000, MMDC0_MDSCR);

	//wait for CON_ACK!
	while(!(__raw_readl(MMDC0_MDSCR) & 0x4000)) ;

	//allow IP access, precharge all
	__raw_writel(0x00008050, MMDC0_MDSCR);
	//set MRR cmd and addr as MR5
	__raw_writel(0x00058060, MMDC0_MDSCR);

	//poll for read data valid
	while(!(__raw_readl(MMDC0_MDSCR) & 0x400)) ;

	mdmrr = __raw_readl(MMDC0_MDMRR);

	if(BOARD_IS_WARIO(rev)) {
		mfgid = mdmrr & 0xFF;
	} else {
		/* On Icewine and Pinot--
		   swap bits according to dataline mapping
		   --lower 16-bits map(which we care)--
		   |  SoC    |  LPDDR2 |
		   | D0-D15  |  D15-D0 | */

		//reverse bits - from 'Bit Twiddling Hacks'
		u16 r, v;
		r = v = mdmrr & 0xFFFF;
		int s = (sizeof(v) * 8) - 1; // extra shift needed at end

		for (v >>= 1; v; v >>= 1)
		{
			r <<= 1; r |= v & 1; s--;
		}
		r <<= s; // shift remaining when highest bits are zero

		//get just LSB which has mfgid
		mfgid = r & 0xFF;
	}

	//restore
	__raw_writel(mapsr, MMDC0_MAPSR);
	__raw_writel(madpcr0, MMDC0_MADPCR0);
	__raw_writel(mdscr, MMDC0_MDSCR);

	return mfgid;
}
#endif /*ifdef CONFIG_IRAM_BOOT */

int board_init(void)
{
#ifdef CONFIG_MFGTOOL_MODE
/* MFG firmware need reset usb to avoid host crash firstly */
#define USBCMD 0x140
	int val = readl(OTG_BASE_ADDR + USBCMD);
	val &= ~0x1;		/*RS bit */
	writel(val, OTG_BASE_ADDR + USBCMD);
#endif

	mxc_iomux_v3_init((void *)IOMUXC_BASE_ADDR);

	setup_boot_device();

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_MX6SL_ARM2;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	setup_uart();

#ifdef CONFIG_I2C_MXC
	setup_i2c(CONFIG_SYS_I2C_PORT);
#endif

#ifdef CONFIG_MXC_EPDC
	gd->fb_base = CONFIG_FB_BASE;
#endif
	disable_watchdog_power_count_down_event();
	return 0;
}

static inline int check_abnormal_reset() {
#if defined(CONFIG_PMIC) && defined(CONFIG_PMIC_MAX77696)
	int flag0 = 0, flag1 = 0, result = 0;

	result = board_pmic_read_reg(PM_GPIO_SADDR, ERCFLAG0_REG, &flag0);
	if (!result) {
		printf("Error reading ERCFLAG0\n");
		return 1;
	}

	if (flag0) {
		printf("ERCFLAG0: %x\n", flag0);
	}

	result = board_pmic_read_reg(PM_GPIO_SADDR, ERCFLAG1_REG, &flag1);
	if (!result) {
		printf("Error reading ERCFLAG1\n");
		return 1;
	}

	if (flag1) {
		printf("ERCFLAG1: %x\n", flag1);
	}

	if (flag0 & (ERCFLAG0_WDPMIC_FRSTRT
			| ERCFLAG0_WDPMIC_FSHDN
			| ERCFLAG0_MR_FSHDN
			| ERCFLAG0_MR_FRSTRT)) {
		return 1;
	}

	if (flag1 & ERCFLAG1_RSTIN_PRSTRT) {
		return 1;
	}
#endif
	return 0;
}

/*
 * return 1 if USB plug in woke up the device
 * return 0 otherwise
 */
int board_is_usb_wakeup(void)
{
	int ret = 0;
#if defined(CONFIG_PMIC) && defined(CONFIG_PMIC_MAX77696)
	unsigned int reg = 0;
	if(!board_pmic_read_reg(USBIF_SADDR, UIC_INTSTS_REG, &reg))
		goto err_usb_wake;
	if(reg & UIC_INTSTS_UICWK_M)
		ret = 1;

err_usb_wake:
#endif
	return ret;
}

/*
 * return 1 if an RTC alarm woke up the device
 * return 0 otherwise
 */
int board_is_rtc_wakeup(int alarm_number)
{
#if defined(CONFIG_PMIC) && defined(CONFIG_PMIC_MAX77696)
	/*
	 * We cannot read the rtc interrupt registers on the MAX77696
	 * because they clear on read and linux needs to know about them.
	 * Instead, read the current time and time of the next wakeup alarm
	 * If the current time is greater than or equal to the alarm time
	 * and the alarm interrupt is enabled, we can assume that we are only
	 * awake because of the alarm.
	 */
	u32 reg = 0, result = 0;
	u32 int_mask;
	u32 sec, min, hour, dow, month, year, dom;
	u32 seca, mina, houra, dowa, montha, yeara, doma;
	u64 bigtime, bigtimea;

	// update read buffers before reading
	reg = RTC_RTCUPDATE0_RBUDR_M;
	result = board_pmic_write_reg(RTC_SADDR, RTC_RTCUPDATE0_REG, reg);
	udelay(201); // must wait at least 200 uS
	if (!result) goto err;

	//read enabled interrupts
	reg = RTC_RTCINTM_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &int_mask);
	if (!result) goto err;
//	printf("RTC int mask: %x\n",int_mask);


	switch (alarm_number) {
	case MAX_77696_RTC1:
		//read alarm1 times
		if (int_mask & RTC_RTCINT_RTCA1) {
//			printf("\n\nRTC1 MASKED\n\n");
			return 0;
		}

		reg = RTC_RTCSECA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &seca);
		if (!result) goto err;

		reg = RTC_RTCMINA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &mina);
		if (!result) goto err;

		reg = RTC_RTCHOURA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &houra);
		if (!result) goto err;

/* Dont care about DOW
		reg = RTC_RTCDOWA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &dowa);
		if (!result) goto err;
*/
		reg = RTC_RTCMONTHA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &montha);
		if (!result) goto err;

		reg = RTC_RTCYEARA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &yeara);
		if (!result) goto err;

		reg = RTC_RTCDOMA1_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &doma);
		if (!result) goto err;

		break;

	case MAX_77696_RTC2:
		//read alarm2 times
		if (int_mask & RTC_RTCINT_RTCA2) {
//			printf("\n\nRTC2 MASKED\n\n");
			return 0;
		}

		reg = RTC_RTCSECA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &seca);
		if (!result) goto err;

		reg = RTC_RTCMINA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &mina);
		if (!result) goto err;

		reg = RTC_RTCHOURA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &houra);
		if (!result) goto err;

/* Dont care about DOW
		reg = RTC_RTCDOWA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &dowa);
		if (!result) goto err;
*/
		reg = RTC_RTCMONTHA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &montha);
		if (!result) goto err;

		reg = RTC_RTCYEARA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &yeara);
		if (!result) goto err;

		reg = RTC_RTCDOMA2_REG;
		result = board_pmic_read_reg(RTC_SADDR, reg, &doma);
		if (!result) goto err;

		break;
	default:
		// test values
		sec = min = hour = dom = month = 10, year = 65;
		seca = 20, mina = 9, houra = doma = montha = 10, yeara = 2;
		break;
	}

	// read current times
	reg = RTC_RTCSEC_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &sec);
	if (!result) goto err;

	reg = RTC_RTCMIN_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &min);
	if (!result) goto err;

	reg = RTC_RTCHOUR_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &hour);
	if (!result) goto err;

/* Dont care about DOW
	reg = RTC_RTCDOW_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &dow);
	if (!result) goto err;
*/
	reg = RTC_RTCMONTH_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &month);
	if (!result) goto err;

	reg = RTC_RTCYEAR_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &year);
	if (!result) goto err;

	reg = RTC_RTCDOM_REG;
	result = board_pmic_read_reg(RTC_SADDR, reg, &dom);
	if (!result) goto err;

//	printf("Current time %d, %d, %d, %d, %d, %d \n",year, month, dom, hour, min, sec);
//	printf("Alarm %d set at %d, %d, %d, %d, %d, %d \n", alarm_number, yeara,
//		montha, doma, houra, mina, seca);

// To combine all fields we will overflow an int.  Use a  u64 as storage for comparison
// We shift by 8 bits because we have plenty of space and it's prettier to look at.
#define COMBINE_TIME_FIELDS_TO_U64(year, month, day, hour, min, sec) \
	((u64)year << 40 | (u64)month << 32 | day << 24 | hour << 16 | min << 8 | sec)

	bigtime = COMBINE_TIME_FIELDS_TO_U64(year, month, dom, hour, min, sec);
	bigtimea = COMBINE_TIME_FIELDS_TO_U64(yeara, montha, doma, houra, mina, seca);
	if (bigtime >= bigtimea) {
//		printf("\n\nRTC WAKEUP\n\n");
		return 1;
	} else {
//		printf("\n\nRTC SET BUT NOT TRIGGERED YET\n\n");
		return 0;
	}
err:
	printf("%s: Error %x (%d)\n", __func__, reg, result);
#endif
	return 0;
}

void board_reset_boot_mode(void) {
	char boot_mode[BOARD_BOOTMODE_LEN + 1];

	// invalidate user updates (Otherwise we'd use them after reboot instead of these new values)
	idme_clear_update();

	if (!idme_get_var("oldboot", boot_mode, sizeof(boot_mode))) {
            printf("Oldboot: %s\n", boot_mode);
            if(!strncmp(boot_mode, "qboot", strlen("qboot"))) {
                /*
                 * bootmode and oldbootmode are both "qboot".  This usually happens when
                 * a user has cancelled the snapshot script and then run it again.
                 * in this case, set bootmode back to main.
                 */
                 printf("Resetting bootmode to main...\n");
                 sprintf(boot_mode, "main");
            }
            idme_update_var("bootmode", boot_mode);
        } else {
            // we should never get here, but if we do, do a normal boot
            idme_update_var("bootmode", "");
        }
}

static inline int check_boot_mode(void)
{
	extern int g_fl_override;
	char boot_mode[BOARD_BOOTMODE_LEN + 1];
#define NEW_BOOTCMD_LENGTH 21
	char boot_cmd[NEW_BOOTCMD_LENGTH];
#ifdef CONFIG_QBOOT
	if (gd->flags & GD_FLG_QUICKBOOT) {
		/*
		 * If the device has reset abnormally, we don't want to quickboot
		 * Mask out quickboot so that we can see any error messages print
		 * and then reset it after determining that we still want to quickboot
		 */
		gd->flags &= ~GD_FLG_QUICKBOOT;

		if(check_abnormal_reset()) {
			puts("Detected abnormal shutdown.  Abandoning quickboot\n");
			board_reset_boot_mode();
			// fall out and continue normal boot
		} else {
			extern void make_working_buffer_sane(void);
			make_working_buffer_sane();

			if(!board_is_rtc_wakeup(MAX_77696_RTC1) && !board_is_rtc_wakeup(MAX_77696_RTC2)
					&& !board_is_usb_wakeup() && !board_is_hall_closed()) {
				// only show splash image if hall is open and not rtc or usb wakeup
				setenv("bootcmd", "splash; run fb");
			} else {
				puts("ns\n"); //no splash
				setenv("bootcmd", "run fb");
			}

			gd->flags |= GD_FLG_QUICKBOOT;
			puts("BOOTMODE OVERRIDE: QUICKBOOT\n");
			setenv("bootdelay", "0");
			setenv("bootretry", "5"); //give a couple seconds to notice
			return BOOT_MODE_QBOOT;
		}
	}
#endif

#ifdef CONFIG_BIST
	setenv("bootdelay", "-1");
#endif

#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("bootmode", boot_mode, sizeof(boot_mode)))
#endif
	{
		return BOOT_MODE_UNKNOWN;
	}

	boot_cmd[0] = 0;

	if (!strncmp(boot_mode, "diags", 5)) {
		printf ("BOOTMODE OVERRIDE: DIAGS\n");
		/* clear bootargs to let the kernel choose them */
		setenv("bootargs", "");
		strcpy(boot_cmd, "run bootcmd_diags");
	} else if (!strncmp(boot_mode, "fastboot", 8)) {
		printf ("BOOTMODE OVERRIDE: FASTBOOT\n");
		strcpy(boot_cmd, "run bootcmd_fastboot");
	} else if (!strncmp(boot_mode, "uboot", strlen("uboot"))) {
		printf ("BOOTMODE OVERRIDE: UBOOT\n");
		strcpy(boot_cmd, "bist");
	} else if (!strncmp(boot_mode, "factory", 7)) {
#if defined(CONFIG_PMIC)
		if (pmic_charging()) {
			char *cmd = (char *) CONFIG_BISTCMD_LOCATION;
			/* Ignore any bist commands */
			cmd[0] = 0;

			printf ("BOOTMODE OVERRIDE OVERRIDE: DIAGS\n");

#if defined(CONFIG_CMD_IDME)
			/* Update bootmode idme var */
			idme_update_var("bootmode", "diags");
#endif
			/* Set the bootcmd to diags and boot immediately */
			setenv("bootcmd", "run bootcmd_diags");
			setenv("bootdelay", "0");
			/* clear bootargs to let the kernel choose them */
			setenv("bootargs", "");

			return 0;
		}
#endif	//CONFIG_PMIC
		printf ("BOOTMODE OVERRIDE: FACTORY\n");
		strcpy(boot_cmd, "run bootcmd_factory");
	} else if (!strncmp(boot_mode, "reset", 7)) {
		printf ("BOOTMODE OVERRIDE: RESET\n");
		strcpy(boot_cmd, "bist reset");
	} else if (!strncmp(boot_mode, "main", 4) || !strncmp(boot_mode, "ota", 3)) {
		/* clear bootargs to let the kernel choose them */
		setenv("bootargs", "\0");

		/* set bootcmd back to default */
		sprintf(boot_cmd, "bootm 0x%x", CONFIG_MMC_BOOTFLASH_ADDR);

		if (!strncmp(boot_mode, "ota", 3)) {
			printf("BOOTMODE OTA : DONT EXPECT FL\n");
			g_fl_override = 1;
		}
		return 0;
	} else if (!strncmp(boot_mode, HAPTIC_IQC_TEST_STR, strlen(HAPTIC_IQC_TEST_STR))) {
		/* Automatic go to bist and run the haptic iqc test at IQC station  */
		sprintf(boot_cmd, "bist haptic");
		/* Passing the boot_mode variable to preserve the string. The extra character
		 * can be used to select differrent waveform.  Ex: haptic_iqc0 can select waveform
		 * ID 0, haptic_iqc1, for waveform ID 1, etc
		 */
		setenv("haptic_test", boot_mode);
	} else {
		return 0;
	}

	setenv("bootcmd", boot_cmd);

	return 0;
}

void board_power_off(void)
{
#ifdef CONFIG_PMIC
    printf("Halting...");
    pmic_power_off();
    while(1);
#endif
}

    /* disable PDE to avoid glitch seen in WDOG line */

static void disable_watchdog_power_count_down_event(void)
{
    __raw_writew(~0x1, (WDOG1_BASE_ADDR+0x08));
}

int board_late_init(void)
{
	const char *rev = (const char *) get_board_id16();	
        check_boot_mode();

#if defined(CONFIG_PMIC)
	if (pmic_init()) {
		unsigned short voltage = BATTERY_INVALID_VOLTAGE;

		if (pmic_adc_read_voltage(&voltage)) {
			printf("Battery voltage: %d mV\n\n", voltage);
		} else {
			printf("Battery voltage read fail!\n\n");
		}
	}

#if defined(CONFIG_WARIO_WOODY)
    /* Config LED's (GREEN & AMBER) to manual & OFF by default */ 
    if ( BOARD_IS_WHISKY_WAN(rev) || BOARD_IS_WHISKY_WFO(rev) ) {
		pmic_led_manual_mode();
    }
#endif

	if(BOARD_REV_GREATER(rev, BOARD_ID_BOURBON_WFO_EVT1) ||
		BOARD_IS_BOURBON_PREEVT2(rev) ||
		BOARD_IS_WARIO_4_256M_CFG_C(rev)) {
		/* wait atleast 100ms after LD7 (that powers 3.2V IO ring)
		is powered-on; before we turn on 3.2V LDO1 for touch */
		udelay(100000L);
		/* turn LDO1 to power touch */
#ifdef DEVELOPMENT_MODE
		printf("turning on LDO1 to power touch..\n");
#endif
		pmic_zforce2_pwrenable(1);
		/* delay after LDO output stabilizes */
		udelay(100);
		setup_touch_pins();
		/* SWDIO should be input */
		gpio_direction_input( IMX_GPIO_NR(3, 24));
	}
#endif

    if ( BOARD_IS_ICEWINE_WARIO(rev) || BOARD_IS_ICEWINE_WFO_WARIO(rev) )	{
        check_haptic_and_recovery();
    }
   
    /* Initialize eMMC PG GPIO's */ 
    if ( BOARD_IS_MUSCAT_WAN(rev) || BOARD_IS_MUSCAT_WFO(rev) ||
         BOARD_IS_WHISKY_WAN(rev) || BOARD_IS_WHISKY_WFO(rev)	||
         BOARD_IS_WOODY(rev) ) {
        gpio_direction_output(IMX_GPIO_NR(4, 17), 1); //EMMC_3v2_EN FEC_RXD0 
        gpio_direction_output(IMX_GPIO_NR(4, 23), 1); //EMMC_1v8_EN FEC_MDC
    }
    
    return 0;
}

int checkboard(void)
{
	const char *sn, *rev;
	const struct board_type *board;
#ifdef CONFIG_QBOOT
        if (gd->flags & GD_FLG_QUICKBOOT) {
            return 0;
        }
#endif
	printf("Board: ");

	board = get_board_type();
	if (board) {
	    printf("%s\n", board->name);
	} else {
	    printf("Unknown\n");
	}

	printf("Boot Reason: [ ");

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		printf("POR");
		break;
	case 0x0009:
		printf("RST");
		break;
	case 0x0010:
	case 0x0011:
		printf("WDOG");
		break;
	default:
		printf("unknown");
	}
	printf(" ]\n");

	printf("Boot Device: ");
	switch (get_boot_device()) {
	case WEIM_NOR_BOOT:
		printf("NOR\n");
		break;
	case ONE_NAND_BOOT:
		printf("ONE NAND\n");
		break;
	case PATA_BOOT:
		printf("PATA\n");
		break;
	case SATA_BOOT:
		printf("SATA\n");
		break;
	case I2C_BOOT:
		printf("I2C\n");
		break;
	case SPI_NOR_BOOT:
		printf("SPI NOR\n");
		break;
	case SD_BOOT:
		printf("SD\n");
		break;
	case MMC_BOOT:
		printf("MMC\n");
		break;
	case NAND_BOOT:
		printf("NAND\n");
		break;
	case UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		break;
	}

	/* serial number and board id */
	sn = (char *) get_board_serial();
	rev = (char *) get_board_id16();

	if (rev)
		printf ("Board Id: %.*s\n", BOARD_PCBA_LEN, rev);

	if (sn)
		printf ("S/N: %.*s\n", BOARD_DSN_LEN, sn);

	return 0;
}

inline int check_post_mode(void)
{
	char post_mode[BOARD_POSTMODE_LEN + 1];
#ifdef CONFIG_QBOOT
        if (gd->flags & GD_FLG_QUICKBOOT) {
                // skip automatic POST in quickboot
                gd->flags |= GD_FLG_POSTSTOP;
                return 0;
        }
#endif

#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("postmode", post_mode, sizeof(post_mode)))
#endif
	{
		return -1;
	}

	if (!strncmp(post_mode, "normal", 6)) {
		setenv("post_hotkeys", "0");
	} else if (!strncmp(post_mode, "slow", 4)) {
		setenv("post_hotkeys", "1");
	} else if (!strncmp(post_mode, "factory", 7)) {
		setenv("bootdelay", "-1");
	}

	return 0;
}

#ifdef CONFIG_MX6SL
void udc_pins_setting(void)
{
}
#endif

#ifdef CONFIG_POST
int post_hotkeys_pressed(void)
{
    char *value;
    int ret;

    check_post_mode();

    ret = ctrlc();
    if (!ret) {
        value = getenv("post_hotkeys");
        if (value != NULL)
	    ret = simple_strtoul(value, NULL, 10);
    }
    return ret;
}
#endif

#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
void post_word_store (ulong a)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_POST_WORD_ADDR);
	*save_addr = a;
}
ulong post_word_load (void)
{
  volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_POST_WORD_ADDR);
  return *save_addr;
}
#endif

#ifdef CONFIG_LOGBUFFER
unsigned long logbuffer_base(void)
{
  /* OOPS_SAVE_BASE + PAGE_SIZE in linux/include/asm-arm/arch/boot_globals.h */
  return CONFIG_SYS_SDRAM_BASE + (2*4096);
}
#endif

static iomux_v3_cfg_t rstcfg_zforce2_pads[] = {
	MX6SL_PAD_KEY_ROW5__GPIO_4_3,
	MX6SL_PAD_KEY_ROW6__GPIO_4_5,
	MX6SL_PAD_KEY_COL0__GPIO_3_24,
	MX6SL_PAD_I2C2_SCL__GPIO_3_14,
	MX6SL_PAD_I2C2_SDA__GPIO_3_15,
	MX6SL_PAD_SD1_DAT4__GPIO_5_12,
	MX6SL_PAD_SD1_DAT5__GPIO_5_9,
};

void reset_touch_zforce2_pins(void) {

	mxc_iomux_v3_setup_multiple_pads(rstcfg_zforce2_pads,
		ARRAY_SIZE(rstcfg_zforce2_pads));
	
	/* touch lines */
	gpio_direction_output(IMX_GPIO_NR(4, 3), 0);
	gpio_direction_output(IMX_GPIO_NR(4, 5), 0);
	/* I2C lines */
	gpio_direction_output(IMX_GPIO_NR(3, 14), 0);
	gpio_direction_output(IMX_GPIO_NR(3, 15), 0);
	/* BSL UART lines */
	gpio_direction_output(IMX_GPIO_NR(3, 24), 0);
	gpio_direction_output(IMX_GPIO_NR(5, 12), 0);
	gpio_direction_output(IMX_GPIO_NR(5, 9), 0);
}

static iomux_v3_cfg_t set_unused_pads[] = {
	//3V3
	MX6SL_PAD_KEY_COL0__GPIO_3_24,
	MX6SL_PAD_KEY_COL1__GPIO_3_26,
	MX6SL_PAD_SD1_DAT7__GPIO_5_10,
	MX6SL_PAD_SD1_DAT6__GPIO_5_7,
	MX6SL_PAD_EPDC_D10__GPIO_1_17,
	MX6SL_PAD_EPDC_D11__GPIO_1_18,
	MX6SL_PAD_EPDC_D12__GPIO_1_19,
	//1V8
	MX6SL_PAD_ECSPI2_SS0__GPIO_4_15,
	MX6SL_PAD_EPDC_PWRCOM__GPIO_2_11,
	MX6SL_PAD_FEC_RXD1__GPIO_4_18
};

static void setup_touch_pins(void)
{
	i2c2_iomux_config();
	gpio3_iomux_config();
	gpio4_iomux_config();
	uart4_iomux_config();
}

static iomux_v3_cfg_t set_bt_brcm_pads[] = {
	MX6SL_PAD_LCD_DAT3__AUDMUX_AUD4_RXD,
	MX6SL_PAD_LCD_DAT4__AUDMUX_AUD4_TXC,
	MX6SL_PAD_LCD_DAT5__AUDMUX_AUD4_TXFS,
	MX6SL_PAD_LCD_DAT6__AUDMUX_AUD4_TXD,
	MX6SL_PAD_EPDC_BDR0__UART3_RTS,
	MX6SL_PAD_EPDC_BDR1__UART3_CTS,
};

static void setup_bt_brcm_pins(void)
{
	mxc_iomux_v3_setup_multiple_pads(set_bt_brcm_pads,
			ARRAY_SIZE(set_bt_brcm_pads));

}

#if defined(CONFIG_WARIO_WOODY)
void soda_i2c_sda_pu_config(int op)
{
	/* MX6_SODA_I2C_SDA_PU:
			Pad Control Register: IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRWAKEUP(0x020E03EC) */
	if (op) {
		__raw_writel((LVE_ENABLED & 0x1) << 22 | (HYS_DISABLED & 0x1) << 16 |
			(PKE_DISABLED & 0x1) << 12 | (ODE_DISABLED & 0x1) << 11 |
			(SPD_50MHZ & 0x3) << 6 | (DSE_240OHM & 0x7) << 3 | (SRE_SLOW & 0x1), IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRWAKEUP);
	} else {
        __raw_writel((LVE_ENABLED & 0x1) << 22 | (HYS_DISABLED & 0x1) << 16 |
			(PKE_DISABLED & 0x1) << 12 | (ODE_ENABLED & 0x1) << 11 |
			(SPD_50MHZ & 0x3) << 6 | (DSE_240OHM & 0x7) << 3 | (SRE_SLOW & 0x1), IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRWAKEUP);
	}
}

void soda_i2c_sda_config(int op)
{
	/* MX6_SODA_I2C_SDA:
			Pad Control Register: IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRCTRL1(0x020E03D8) */
	if (op) {
		__raw_writel((LVE_ENABLED & 0x1) << 22 | (HYS_DISABLED & 0x1) << 16 |
			(PKE_DISABLED & 0x1) << 12 | (ODE_ENABLED & 0x1) << 11 |
			(SPD_50MHZ & 0x3) << 6 | (DSE_240OHM & 0x7) << 3 | (SRE_SLOW & 0x1), IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRCTRL1);
	} else {
		__raw_writel((LVE_ENABLED & 0x1) << 22 | (HYS_ENABLED & 0x1) << 16 | (PUS_100KOHM_PU & 0x3) << 14 |
			(PUE_PULL & 0x1) << 13 | (PKE_ENABLED & 0x1) << 12 | (ODE_DISABLED & 0x1) << 11 |
			(SPD_50MHZ & 0x3) << 6 | (DSE_DISABLED & 0x7) << 3 | (SRE_SLOW & 0x1), IOMUXC_SW_PAD_CTL_PAD_EPDC_PWRCTRL1);
	}
}
#endif
static void set_unused_pins(void)
{
	iomux_v3_cfg_t *p = set_unused_pads;
	int i;
	/* Set PADCTRL to 0 for all IOMUX */
	for (i = 0; i < ARRAY_SIZE(set_unused_pads); i++) {
		*p &= ~MUX_PAD_CTRL_MASK;
		*p |= ((u64)0x3000 << MUX_PAD_CTRL_SHIFT); /* enable Pull and PD */
		p++;
	}
	mxc_iomux_v3_setup_multiple_pads(set_unused_pads,
			ARRAY_SIZE(set_unused_pads));

	/* PMIC nFID pin */
	mxc_iomux_v3_setup_pad(MX6SL_PAD_FEC_TX_CLK__GPIO_4_21);

	udelay(100);

	//Set unused GPIOs to input
	gpio_direction_input(IMX_GPIO_NR(3,24));
	gpio_direction_input(IMX_GPIO_NR(3,26));
	gpio_direction_input(IMX_GPIO_NR(5,10));
	gpio_direction_input(IMX_GPIO_NR(5,7));
	gpio_direction_input(IMX_GPIO_NR(1,17));
	gpio_direction_input(IMX_GPIO_NR(1,18));
	gpio_direction_input(IMX_GPIO_NR(1,19));
	gpio_direction_input(IMX_GPIO_NR(4,15));
	gpio_direction_input(IMX_GPIO_NR(2,11));
	gpio_direction_input(IMX_GPIO_NR(4,18));
}

#define morse_delay(t)  {{unsigned long msec=(t*100); while (msec--) { udelay(1000);}}}
#define led_2_on() { }
#define led_2_off() { }
#define long_gap()  {morse_delay(6);}
#define short_gap()  {morse_delay(2);}
#define gap()  {led_2_off(); morse_delay(1);}
#define dit()  {led_2_on();  morse_delay(1); gap();}
#define dah()  {led_2_on();  morse_delay(3); gap();}
void sos (void)
{
    dit(); dit(); dit(); short_gap(); /* Morse Code S */
    dah(); dah(); dah(); short_gap(); /* Morse Code O */
    dit(); dit(); dit(); short_gap(); /* Morse Code S */
    long_gap();
}
void ok (void)
{
    dah(); dah(); dah(); short_gap(); /* Morse Code O */
    dah(); dit(); dah(); short_gap(); /* Morse Code K */
    long_gap();
}

