#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_SPLASH
#include <lcd.h>
#include <asm/io.h>
#include "epdc_regs.h"

DECLARE_GLOBAL_DATA_PTR;

extern void lcd_enable(void );
extern int drv_lcd_init_noconsole (void);

int do_splash(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	int do_reset_qb = 0;

	lcd_base = (void *)(gd->fb_base);

	if (gd->flags & GD_FLG_QUICKBOOT) {
		gd->flags &= ~GD_FLG_QUICKBOOT;
		do_reset_qb = 1;
	}
	puts("ss\n"); // start splash screen
	if (do_reset_qb) {
		// must reset here so that drv_lcd_init_noconsole doesnt print
		gd->flags |= GD_FLG_QUICKBOOT;
	}

	drv_lcd_init_noconsole();

	if (gd->flags & GD_FLG_QUICKBOOT) {
		gd->flags &= ~GD_FLG_QUICKBOOT;
	}
	puts("ds\n"); // done splash screen
	if (do_reset_qb) {
		gd->flags |= GD_FLG_QUICKBOOT;
	}

	return 0;
}

U_BOOT_CMD(
	splash, CONFIG_SYS_MAXARGS, 1, do_splash,
	"draw splash screen",
	"draw splash screen"
);

int dump_epdc_reg(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf( "\n\n");
	printf( "EPDC_CTRL 0x%x\n", __raw_readl(EPDC_CTRL));
	printf( "EPDC_WVADDR 0x%x\n", __raw_readl(EPDC_WVADDR));
	printf( "EPDC_WB_ADDR 0x%x\n", __raw_readl(EPDC_WB_ADDR));
	printf( "EPDC_RES 0x%x\n", __raw_readl(EPDC_RES));
	printf( "EPDC_FORMAT 0x%x\n", __raw_readl(EPDC_FORMAT));
	printf( "EPDC_FIFOCTRL 0x%x\n", __raw_readl(EPDC_FIFOCTRL));
	printf( "EPDC_UPD_ADDR 0x%x\n", __raw_readl(EPDC_UPD_ADDR));
	printf( "EPDC_UPD_STRIDE 0x%x\n", __raw_readl(EPDC_UPD_STRIDE));
	printf( "EPDC_UPD_FIXED 0x%x\n", __raw_readl(EPDC_UPD_FIXED));
	printf( "EPDC_UPD_CORD 0x%x\n", __raw_readl(EPDC_UPD_CORD));
	printf( "EPDC_UPD_SIZE 0x%x\n", __raw_readl(EPDC_UPD_SIZE));
	printf( "EPDC_UPD_CTRL 0x%x\n", __raw_readl(EPDC_UPD_CTRL));
	printf( "EPDC_TEMP 0x%x\n", __raw_readl(EPDC_TEMP));
	printf( "EPDC_AUTOWV_LUT 0x%x\n", __raw_readl(EPDC_AUTOWV_LUT));
	printf( "EPDC_TCE_CTRL 0x%x\n", __raw_readl(EPDC_TCE_CTRL));
	printf( "EPDC_TCE_SDCFG 0x%x\n", __raw_readl(EPDC_TCE_SDCFG));
	printf( "EPDC_TCE_GDCFG 0x%x\n", __raw_readl(EPDC_TCE_GDCFG));
	printf( "EPDC_TCE_HSCAN1 0x%x\n", __raw_readl(EPDC_TCE_HSCAN1));
	printf( "EPDC_TCE_HSCAN2 0x%x\n", __raw_readl(EPDC_TCE_HSCAN2));
	printf( "EPDC_TCE_VSCAN 0x%x\n", __raw_readl(EPDC_TCE_VSCAN));
	printf( "EPDC_TCE_OE 0x%x\n", __raw_readl(EPDC_TCE_OE));
	printf( "EPDC_TCE_POLARITY 0x%x\n", __raw_readl(EPDC_TCE_POLARITY));
	printf( "EPDC_TCE_TIMING1 0x%x\n", __raw_readl(EPDC_TCE_TIMING1));
	printf( "EPDC_TCE_TIMING2 0x%x\n", __raw_readl(EPDC_TCE_TIMING2));
	printf( "EPDC_TCE_TIMING3 0x%x\n", __raw_readl(EPDC_TCE_TIMING3));
	printf( "EPDC_PIGEON_CTRL0 0x%x\n", __raw_readl(EPDC_PIGEON_CTRL0));
	printf( "EPDC_PIGEON_CTRL1 0x%x\n", __raw_readl(EPDC_PIGEON_CTRL1));
	printf( "EPDC_IRQ_MASK1 0x%x\n", __raw_readl(EPDC_IRQ_MASK1));
	printf( "EPDC_IRQ_MASK2 0x%x\n", __raw_readl(EPDC_IRQ_MASK2));
	printf( "EPDC_IRQ1 0x%x\n", __raw_readl(EPDC_IRQ1));
	printf( "EPDC_IRQ2 0x%x\n", __raw_readl(EPDC_IRQ2));
	printf( "EPDC_IRQ_MASK 0x%x\n", __raw_readl(EPDC_IRQ_MASK));
	printf( "EPDC_IRQ 0x%x\n", __raw_readl(EPDC_IRQ));
	printf( "EPDC_STATUS_LUTS 0x%x\n", __raw_readl(EPDC_STATUS_LUTS));
	printf( "EPDC_STATUS_LUTS2 0x%x\n", __raw_readl(EPDC_STATUS_LUTS2));
	printf( "EPDC_STATUS_NEXTLUT 0x%x\n", __raw_readl(EPDC_STATUS_NEXTLUT));
	printf( "EPDC_STATUS_COL1 0x%x\n", __raw_readl(EPDC_STATUS_COL));
	printf( "EPDC_STATUS_COL2 0x%x\n", __raw_readl(EPDC_STATUS_COL2));
	printf( "EPDC_STATUS 0x%x\n", __raw_readl(EPDC_STATUS));
	printf( "EPDC_UPD_COL_CORD 0x%x\n", __raw_readl(EPDC_UPD_COL_CORD));
	printf( "EPDC_UPD_COL_SIZE 0x%x\n", __raw_readl(EPDC_UPD_COL_SIZE));
	printf( "EPDC_DEBUG 0x%x\n", __raw_readl(EPDC_DEBUG));
	printf( "EPDC_DEBUG_LUT 0x%x\n", __raw_readl(EPDC_DEBUG_LUT));
	printf( "EPDC_HIST1_PARAM 0x%x\n", __raw_readl(EPDC_HIST1_PARAM));
	printf( "EPDC_HIST2_PARAM 0x%x\n", __raw_readl(EPDC_HIST2_PARAM));
	printf( "EPDC_HIST4_PARAM 0x%x\n", __raw_readl(EPDC_HIST4_PARAM));
	printf( "EPDC_HIST8_PARAM0 0x%x\n", __raw_readl(EPDC_HIST8_PARAM0));
	printf( "EPDC_HIST8_PARAM1 0x%x\n", __raw_readl(EPDC_HIST8_PARAM1));
	printf( "EPDC_HIST16_PARAM0 0x%x\n", __raw_readl(EPDC_HIST16_PARAM0));
	printf( "EPDC_HIST16_PARAM1 0x%x\n", __raw_readl(EPDC_HIST16_PARAM1));
	printf( "EPDC_HIST16_PARAM2 0x%x\n", __raw_readl(EPDC_HIST16_PARAM2));
	printf( "EPDC_HIST16_PARAM3 0x%x\n", __raw_readl(EPDC_HIST16_PARAM3));
	printf( "EPDC_GPIO 0x%x\n", __raw_readl(EPDC_GPIO));
	printf( "EPDC_VERSION 0x%x\n", __raw_readl(EPDC_VERSION));
	printf( "\n\n");
	return 0;
}
U_BOOT_CMD(
	dump_epdc_regs, CONFIG_SYS_MAXARGS, 1, dump_epdc_reg,
	"dump epdc regs",
	"dump epdc regs"
);


#endif
