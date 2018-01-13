#include <common.h>
#include <command.h>
#include <libfboot.h>
#include <mmc.h>

static unsigned long bios_addr = 0;
static unsigned long sbios_addr = 0;

DECLARE_GLOBAL_DATA_PTR;

/*
 * do_bios_init :
 */
int do_bios_init(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int arg;
	int tuning;
	u32 *p_bios_tuning;

	if (argc != 4 && argc != 5) {
		cmd_usage(cmdtp);
		return 1;
	}

	bios_addr = simple_strtol(argv[1], NULL, 16);
	sbios_addr = simple_strtol(argv[2], NULL, 16);
	arg = simple_strtol(argv[3], NULL, 16);
	p_bios_tuning = (u32 *)(CONFIG_SBIOS_LOADADDR + CONFIG_SBIOS_SIZE - 2 * sizeof(u32));

	if (argc == 5) {
		
		tuning = simple_strtol(argv[4], NULL, 10);
		if (tuning <= 127 && tuning >= 0)
			*p_bios_tuning = tuning;
		else {
			printf("%s passed in tuning value : %d out of range, set to 0\n", __func__, tuning);
			*p_bios_tuning = 0;
		}
	} else 
		*p_bios_tuning = 0;

	printf("BIOS: %lx  S-BIOS:%lx\n", bios_addr, sbios_addr);

	/* clean vector */
	memset((void *)fb_get_vector_base(), 0, 0x40);

	fb_bios_init(bios_addr, sbios_addr, arg);

	return 0;
}

U_BOOT_CMD(biosinit, 5, 0, do_bios_init,
		   "biosinit <F-BIOS ADDR> <S-BIOS ADDR> <ARG>",
		   "initialize bios before fast boot"
	);

extern void board_reset_boot_mode(void);
static void reset_bootmode() {
	puts("Quickboot error.  Resetting bootmode\n");
	mmc_initialize(gd->bd);
	mmc_init(find_mmc_device(CONFIG_MMC_BOOTFLASH));

	board_reset_boot_mode();
}

/*
 * do_fboot:
 */
int do_fboot(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	// quickboot is going to use its own printouts anyways
	// unmask qb flag here so that we see any error printouts
	gd->flags &= ~GD_FLG_QUICKBOOT;

	if (!bios_addr || !sbios_addr ||
		!fb_is_exist_bios(bios_addr, sbios_addr)) {
		printf("BIOS is not loaded or not found.\n");
		reset_bootmode();
		return 1;
	}

	if (!fb_is_valid_image()) {
		printf("image is invalid.\n");
		reset_bootmode();
		return 1;
	}

	fb_fastboot();

	reset_bootmode();

	return 1;
}

U_BOOT_CMD(fb, 1, 0, do_fboot,
		   "fb",
		   "fastboot from saved image."
);
