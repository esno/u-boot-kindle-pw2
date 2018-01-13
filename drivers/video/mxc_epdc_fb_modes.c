#include <common.h>
#include <lcd.h>

// Copied from kernel: arch/arm/plat-mxc/include/mach/epdc.h:
struct imx_epdc_fb_mode {
    struct fb_videomode *vmode;
    int vscan_holdoff;
    int sdoed_width;
    int sdoed_delay;
    int sdoez_width;
    int sdoez_delay;
    int gdclk_hp_offs;
    int gdsp_offs;
    int gdoe_offs;
    int gdclk_offs;
    int num_ce;
    int physical_width; // Physical width in mm
    int physical_height; // Physical height in mm
    int material; // Display material
};

#define FLAG_SCAN_X_INVERT  1


// Copied from kernel: include/linux/mxcfb.h:
enum panel_modes {

	PANEL_MODE_E60_PINOT = 0,
	PANEL_MODE_EN060OC1_3CE_225,
	PANEL_MODE_ED060TC1_3CE,
	PANEL_MODE_ED060SCN,
	PANEL_MODE_ED060SCP,
	PANEL_MODE_EN060TC1_CARTA_1_2,
	PANEL_MODE_ED060TC1_3CE_CARTA_1_2,
	PANEL_MODE_COUNT,
};

/* Display material */
#define EPD_MATERIAL_V220 0x00
#define EPD_MATERIAL_V320 0x01
#define EPD_MATERIAL_CARTA_1_2 0x02

// Copied from kernel: include/linux/fb.h
struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;
	u32 left_margin;
	u32 right_margin;
	u32 upper_margin;
	u32 lower_margin;
	u32 hsync_len;
	u32 vsync_len;
	u32 sync;
	u32 vmode;
	u32 flag;
};

#define FB_VMODE_NONINTERLACED  0	/* non interlaced */


// Copied from kernel: arch/arm/mach-mx6/board-mx6sl_wario.c
static struct fb_videomode e60_pinot_mode = {
	.name         = "E60_V220",
	.refresh      = 85,
	.xres         = 1024,
	.yres         = 758,
	.pixclock     = 40000000,
	.left_margin  = 12,
	.right_margin = 76,
	.upper_margin = 4,
	.lower_margin = 5,
	.hsync_len    = 12,
	.vsync_len    = 2,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = 0,
};

static struct fb_videomode en060oc1_3ce_225_mode = {
	.name         = "EN060OC1-3CE-225",
	.refresh      = 85,
	.xres         = 1440,
	.yres         = 1080,
	.pixclock     = 120000000,
	.left_margin  = 24,
	.right_margin = 528,
	.upper_margin = 4,
	.lower_margin = 3,
	.hsync_len    = 24,
	.vsync_len    = 2,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = FLAG_SCAN_X_INVERT,
};

static struct fb_videomode ed060tc1_3ce_mode = {
	.name         = "ED060TC1-3CE",
	.refresh      = 85,
	.xres         = 1448,
	.yres         = 1072,
	.pixclock     = 80000000,
	.left_margin  = 16,
	.right_margin = 104,
	.upper_margin = 4,
	.lower_margin = 4,
	.hsync_len    = 26,
	.vsync_len    = 2,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = 0,
};

static struct fb_videomode e60_v220_wj_mode = {
	.name         = "E60_V220_WJ",
	.refresh      = 85,
	.xres         = 800,
	.yres         = 600,
	.pixclock     = 32000000,
	.left_margin  = 17,
	.right_margin = 172,
	.upper_margin = 4,
	.lower_margin = 18,
	.hsync_len    = 15,
	.vsync_len    = 4,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = 0,
};


static struct fb_videomode ed060scp_mode = {
	.name         = "ED060SCP",
	.refresh      = 85,
	.xres         = 800,
	.yres         = 600,
	.pixclock     = 26666667,
	.left_margin  = 8,
	.right_margin = 100,
	.upper_margin = 4,
	.lower_margin = 8,
	.hsync_len    = 4,
	.vsync_len    = 1,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = 0,
};

//whisky
static struct fb_videomode en060tc1_1ce_mode = {
	.name="20150213_EN060TC1_1CE",
	.refresh      = 85,
	.xres         = 1072,
	.yres         = 1448,
	.pixclock=80000000,
	.left_margin  = 8,
	.right_margin=92,
	.upper_margin = 4,
	.lower_margin=7,
	.hsync_len    = 8,
	.vsync_len    = 2,
	.sync         = 0,
	.vmode        = FB_VMODE_NONINTERLACED,
	.flag         = FLAG_SCAN_X_INVERT,
};

static struct imx_epdc_fb_mode panel_modes[PANEL_MODE_COUNT] = {
	[PANEL_MODE_E60_PINOT] = {
		.vmode         = &e60_pinot_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 524,
		.gdsp_offs     = 25,
		.gdoe_offs     = 0,
		.gdclk_offs    = 19,
		.num_ce        = 1,
		.physical_width  = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_V320,
	},
	[PANEL_MODE_EN060OC1_3CE_225] = {
		.vmode         = &en060oc1_3ce_225_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 1116,
		.gdsp_offs     = 86,
		.gdoe_offs     = 0,
		.gdclk_offs    = 57,
		.num_ce        = 3,
		.physical_width  = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_V320,
	},
	[PANEL_MODE_ED060TC1_3CE] = {
		.vmode         = &ed060tc1_3ce_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 562,
		.gdsp_offs     = 662,
		.gdoe_offs     = 0,
		.gdclk_offs    = 225,
		.num_ce        = 3,
		.physical_width  = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_V320,
	},
	[PANEL_MODE_ED060SCN] = {
		.vmode         = &e60_v220_wj_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 425,
		.gdsp_offs     = 321,
		.gdoe_offs     = 0,
		.gdclk_offs    = 17,
		.num_ce        = 1,
		.physical_width = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_V220,
	},
	[PANEL_MODE_ED060SCP] = {
		.vmode         = &ed060scp_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 419,
		.gdsp_offs     = 263,
		.gdoe_offs     = 0,
		.gdclk_offs    = 5,
		.num_ce        = 1,
		.physical_width = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_V220,
	},
	//whisky
	[PANEL_MODE_EN060TC1_CARTA_1_2] = {
		.vmode         = &en060tc1_1ce_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 464,
		.gdsp_offs     = 451,
		.gdoe_offs     = 0,
		.gdclk_offs    = 127,
		.num_ce        = 1,
		.physical_width = 91,
		.physical_height = 122,
		.material = EPD_MATERIAL_CARTA_1_2,
	},
	/* panel mode param for muscat */
	[PANEL_MODE_ED060TC1_3CE_CARTA_1_2] = {
		.vmode         = &ed060tc1_3ce_mode,
		.vscan_holdoff = 4,
		.sdoed_width   = 10,
		.sdoed_delay   = 20,
		.sdoez_width   = 10,
		.sdoez_delay   = 20,
		.gdclk_hp_offs = 562,
		.gdsp_offs     = 662,
		.gdoe_offs     = 0,
		.gdclk_offs    = 225,
		.num_ce        = 3,
		.physical_width  = 122,
		.physical_height = 91,
		.material = EPD_MATERIAL_CARTA_1_2,
	},
};


// Copied from kernel: drivers/video/mxc/mxc_epdc_fb_lab126.c
struct fbmode_override {
	char *barcode_prefix;
	int vmode_index;
	int vddh;
	int lve;
};

static struct fbmode_override fbmode_overrides[] = {
	{
		.barcode_prefix = "EC3",
		.vmode_index = PANEL_MODE_EN060TC1_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{ /* 1448x1072 85Hz G&T */
		.barcode_prefix = "EDG",
		.vmode_index = PANEL_MODE_EN060TC1_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{ /* 1448x1072 85Hz Icewine */
		.barcode_prefix = "ED4",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "ED5",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EB3",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EB4",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EB6",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EBA",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EBF",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EDH",
		.vmode_index = PANEL_MODE_ED060TC1_3CE,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EE1",
		.vmode_index = PANEL_MODE_ED060TC1_3CE_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EE2",
		.vmode_index = PANEL_MODE_ED060TC1_3CE_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EE3",
		.vmode_index = PANEL_MODE_ED060TC1_3CE_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "EEB",
		.vmode_index = PANEL_MODE_ED060TC1_3CE_CARTA_1_2,
		.vddh = 25000,
		.lve = 1
	},
	{
		.barcode_prefix = "S1V",
		.vmode_index = PANEL_MODE_EN060OC1_3CE_225,
		.vddh = 22000,
		.lve = 0
	}, /* 600x800 85Hz Sauza */
	{
		.barcode_prefix = "E6T",
		.vmode_index = PANEL_MODE_ED060SCN,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "E6U",
		.vmode_index = PANEL_MODE_ED060SCN,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "E6V",
		.vmode_index = PANEL_MODE_ED060SCN,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "E6S",
		.vmode_index = PANEL_MODE_ED060SCN,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "E6R",
		.vmode_index = PANEL_MODE_ED060SCN,
		.vddh = 22000,
		.lve = 0
	},
	{ /* 600x800 85Hz Bourbon */
		.barcode_prefix = "EBR",
		.vmode_index = PANEL_MODE_ED060SCP,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "EBV",
		.vmode_index = PANEL_MODE_ED060SCP,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "EBU",
		.vmode_index = PANEL_MODE_ED060SCP,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "EBS",
		.vmode_index = PANEL_MODE_ED060SCP,
		.vddh = 22000,
		.lve = 0
	},
	{
		.barcode_prefix = "EBT",
		.vmode_index = PANEL_MODE_ED060SCP,
		.vddh = 22000,
		.lve = 0
	},
};

void mxc_epdc_vidinfo_for_barcode(const char *barcode_prefix, vidinfo_t *info, struct epdc_timing_params *timings) {
	int i, match = 0;
	
	struct fb_videomode *vmode;
	struct imx_epdc_fb_mode *emode;

	for (i=0; i<ARRAY_SIZE(fbmode_overrides); i++) {
		if (strcmp(barcode_prefix, fbmode_overrides[i].barcode_prefix) == 0) {
			match = i;
		}
	}

	emode = panel_modes + fbmode_overrides[match].vmode_index;
	vmode = emode -> vmode;

	info -> vl_refresh = vmode->refresh;
	info -> vl_col = vmode->xres;
	info -> vl_row = vmode->yres;
	info -> vl_pixclock = vmode -> pixclock;
	info -> vl_left_margin = vmode -> left_margin;
	info -> vl_right_margin = vmode -> left_margin;
	info -> vl_upper_margin = vmode -> upper_margin;
	info -> vl_lower_margin = vmode -> lower_margin;
	info -> vl_hsync = vmode->hsync_len;
	info -> vl_vsync = vmode->vsync_len;
	info -> vl_sync = vmode->sync;
	info -> vl_mode = vmode->vmode;
	info -> vl_flag = vmode->flag;
	info -> vl_bpix = 3;
	info -> vl_vddh = fbmode_overrides[match].vddh;


	timings -> vscan_holdoff = emode -> vscan_holdoff;
	timings -> sdoed_width = emode -> sdoed_width;
	timings -> sdoed_delay = emode -> sdoed_delay;
	timings -> sdoez_width = emode -> sdoez_width;
	timings -> sdoez_delay = emode -> sdoez_delay;
	timings -> gdclk_hp_offs = emode -> gdclk_hp_offs;
	timings -> gdsp_offs = emode -> gdsp_offs;
	timings -> gdoe_offs = emode -> gdoe_offs;
	timings -> gdclk_offs = emode -> gdclk_offs;
	timings -> num_ce = emode -> num_ce;
}
