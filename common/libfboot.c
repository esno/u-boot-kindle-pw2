#include <libfboot.h>
#include <asm-arm/types.h>
#include <config.h>

/*
 * common.h and DECLARE_GLOBAL_DATA_PTR are used to detect quickboot via GD_FLG_QUICKBOOT
 * If this check becomes no longer necessary, these can be removed.
 */
#include <common.h>
DECLARE_GLOBAL_DATA_PTR;

#define PAGE_SIZE	0x1000
#define PAGE_MASK	(~(PAGE_SIZE-1))
#define PAGE_ALIGN(x)	(((x) + PAGE_SIZE - 1) & PAGE_MASK)

#ifndef FBOOT_WB_SIZE
#define FBOOT_WB_SIZE	(PAGE_SIZE * 1)
#endif

static unsigned char work_buff[PAGE_ALIGN(FBOOT_WB_SIZE + PAGE_SIZE) /
			       sizeof(unsigned char)];

#define PRINTF(...)
//#define PRINTF(...) printf(__VA_ARGS__)

/**
 * fb_is_exist_bios
 *
 * Check BIOS is exists at addr.
 *
 * @param[in]	bios_addr	BIOS address
 * @param[in]	sbios_addr	Storage BIOS address
 * @retval		true 	exist
 * @retval		false	not exist
 */
int fb_is_exist_bios(unsigned long bios_addr, unsigned long sbios_addr)
{
	return *(unsigned long *)(bios_addr + 0x30) == 0x51494255 &&
		*(unsigned long *)(sbios_addr + 0x20) == 0x51494255;
}

/**
 * fb_get_vector_base
 *
 * Get a vector base address.
 *
 * @return		vector base address
 */
unsigned long fb_get_vector_base(void)
{
	unsigned long control;

	__asm__ __volatile__ (
		"mrc p15, 0, %0, c1, c0, 0\n"
		:"=r"(control)::"cc");

	/* Check if vector is low */
	if ((control & (1 <<13)) == 0) {
		unsigned long vbar;

		/* Check current VBAR */
		__asm__ __volatile__ (
			"mrc p15, 0, %0, c12, c0, 0\n"
			:"=r"(vbar)::"cc");
		PRINTF("VBAR: %x\n", vbar);

		/* set VBAR into 0x00000000 if it isn't */
		if (vbar) {
			vbar = 0x00000000;
			__asm__ __volatile__ (
				"mcr p15, 0, %0, c12, c0, 0\n"
				::"r"(vbar):"cc");
		}

		return 0x00000000;
	}

	return 0xffff0000;
}

static int set_vector(int offset, unsigned long bios_addr)
{
	volatile unsigned long *vector;
	volatile unsigned long *addr;
	int i;

	vector = (unsigned long *)(fb_get_vector_base() + offset); /* SVC vector */
	addr = vector + 0x20 / 4;

	for (i = 0; i < 0x100; i++) {
		if (!addr[0] && !addr[1])
			break;
		addr++;
	}

	if (*addr) {
		PRINTF("Fatal: Falcon workarea is not found.\n");
		return -1;
	}

	/* set BIOS vector addr */
	*addr = bios_addr + offset;

	/* set Jump code */
	/* ldr pc, [pc, #??] */
	*vector = 0xe59ff000 | ((unsigned long)addr - (unsigned long)vector - 8);
	__asm__ __volatile__ (
		"mcr p15, 0, %0, c7, c10, 1\n"	// D-cache clean by MVA
		"mcr p15, 0, %1, c7, c5, 0\n"	// Invalidate I-Cache
		::"r"(vector), "r"(0):"cc");
	return 0;
}

/**
 * fb_bios_init
 *
 * Set vector to F-BIOS. And, call bios_init API.
 *
 * @param[in]	bios_addr	address of F-BIOS
 * @param[in]	sbios_addr	addresss of S-BIOS
 * @param[in]	arg			argument of bios_init API
 * @return		fail or success
 * @retval		0		success
 * @retval		!=0		fail
 */
int fb_bios_init(unsigned long bios_addr, unsigned long sbios_addr, int arg)
{
	void *buff;
	int ret;
	int size = PAGE_ALIGN(FBOOT_WB_SIZE) / PAGE_SIZE;
        u32 *p_bios_caller = (u32 *)(CONFIG_SBIOS_LOADADDR + CONFIG_SBIOS_SIZE - sizeof(u32));

        u32 *__bios_printk = (u32 *)(CONFIG_SBIOS_LOADADDR + CONFIG_SBIOS_SIZE - 3*sizeof(u32));
	/*
	 * s-bios uses this pointer to access Linux's printk.
	 * We need to clear this in case it is non-null before linux starts managing it.
	 */
	*__bios_printk = NULL;

	if (!fb_is_exist_bios(bios_addr, sbios_addr)) {
		PRINTF("BIOS is not exist.\n");
		return -1;
	}

	buff = (void*) PAGE_ALIGN((unsigned long)work_buff);

	set_vector(4, bios_addr);
	set_vector(8, bios_addr);

	/*
	 * If we are running a quickboot, we must run mmc initialization code from sbios
	 * Pretend to not be uboot in this case, so that initialization will run
	 */
	if(gd->flags & GD_FLG_QUICKBOOT)
		*p_bios_caller = 0;
	else
		*p_bios_caller = 0xafc10650;
	cache_flush();

	__asm__ __volatile__ (
		"mov r0, %1\n\t"
		"mov r1, %2\n\t"
		"mov r2, %3\n\t"
		"mov r3, %4\n\t"
		"swi 0x1\n\t"
		"mov %0, r0\n\t"
		:"=r"(ret)
		:"r"(arg), "r"(sbios_addr), "r"(buff), "r"(size)
		:"r0", "r1", "r2", "r3", "r14", "cc", "memory");

        *p_bios_caller = 0;
	if (ret) {
		PRINTF("BIOS initialization is failed.\n");
		return -1;
	}

	return 0;
}

/**
 * fb_fastboot
 *
 * Enter to fast boot sequence.
 * \note This function never returns to caller.
 */
void fb_fastboot(void)
{
	__asm__ __volatile__("swi 0x2\n\t");
	/* NOT_REACHED */
}

/**
 * fb_is_valid_image
 *
 * Check a image is valid or not.
 *
 * @retval		true 	valid
 * @retval		false	invalid
 */
int fb_is_valid_image(void)
{
	int ret;

	__asm__ __volatile__(
		"swi 0x3\n\t"
		"mov %0, r0\n\t"
		:"=r"(ret)
		::"r0", "r14", "cc", "memory");
	return ret;
}

