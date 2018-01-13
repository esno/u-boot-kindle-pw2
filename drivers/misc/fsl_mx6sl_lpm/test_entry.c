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

#include "common.h"
#include "dram_api.h"
#include "config.h"
#include "low_power_mode.h"
#include "system_config.h"
#include <i2c.h>
#include "asm-arm/arch-mx6/regs-anadig.h"
#include "asm-arm/arch-mx6/gpio.h"
#include "asm-arm/arch-mx6/iomux-v3.h"
#include "asm-arm/arch-mx6/mx6sl_pins.h"

#include <diag_struct.h>
#include <boardid.h>
#include <asm/arch/board-mx6sl_wario.h>

#ifdef CONFIG_PMIC
#include <pmic.h>
#endif

#ifdef CONFIG_PMIC_MAX77696
#include <pmic_max77696.h>
#include <max77696_registers.h>
extern int pmic_fl_enable ( int enable);
#endif
extern int board_i2c_write_reg(unsigned int dev,unsigned char saddr, unsigned int reg, const unsigned int val) ;
extern config_t lpm_config[];
extern config_t system_config[];
extern void lpm_suspend_iomux_setup(void);
extern void reset_touch_zforce2_pins(void);
extern const u8 *get_board_id16(void);
unsigned long current_lpm_config = 0;

static void i2c_clock_enable(void)
{
	reg32_write(CCM_CCGR2, reg32_read(CCM_CCGR2) | 0x000000C0);
}

static void i2c_clock_disable(void)
{
	reg32_write(CCM_CCGR2, reg32_read(CCM_CCGR2) & (~0x000000C0));
}

void lower_arm_soc_voltages(void)
{
	// Set SoC LDO to Bypass
	update_config(system_config + SYS_CFG_ID_SOC_VOLTAGE     , 1500);
	//Lower ARM frequency
	update_config(system_config + SYS_CFG_ID_PLL_ARM_BYPASS     , 1);

	i2c_clock_enable();

	//Lower ARM & SoC voltages
#ifdef CONFIG_PMIC_MAX77696
	/*VDDCORE 0.95V: SW1*/
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT1_REG, (unsigned int)0x1C);
	/*VDDSOC 1.15V : SW2*/
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT2_REG, (unsigned int)0x2C);
#endif
	
	i2c_clock_disable();
}

void emmc_power_gate_off(void)
{
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_RST__GPIO_4_27);
	gpio_direction_output(MX6SL_SD2_RST, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_CLK__GPIO_5_5);
	gpio_direction_output(MX6SL_SD2_CLK, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_CMD__GPIO_5_4);
	gpio_direction_output(MX6SL_SD2_CMD, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT0__GPIO_5_1);
	gpio_direction_output(MX6SL_SD2_DAT0, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT1__GPIO_4_30);
	gpio_direction_output(MX6SL_SD2_DAT1, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT2__GPIO_5_3);
	gpio_direction_output(MX6SL_SD2_DAT2, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT3__GPIO_4_28);
	gpio_direction_output(MX6SL_SD2_DAT3, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT4__GPIO_5_2);
	gpio_direction_output(MX6SL_SD2_DAT4, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT5__GPIO_4_31);
	gpio_direction_output(MX6SL_SD2_DAT5, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT6__GPIO_4_29);
	gpio_direction_output(MX6SL_SD2_DAT6, 0); 
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_SD2_DAT7__GPIO_5_0);
	gpio_direction_output(MX6SL_SD2_DAT7, 0); 
	
	
	mxc_iomux_v3_setup_pad(MX6SL_PAD_FEC_RXD0__GPIO_4_17);
	mxc_iomux_v3_setup_pad(MX6SL_PAD_FEC_MDC__GPIO_4_23);			
	gpio_direction_output(MX6SL_FEC_RXD0_EMMC_3v2, 0); //EMMC_3v2_EN FEC_RXD0 
	gpio_direction_output(MX6SL_FEC_MDC_EMMC_1v8, 0); //EMMC_1v8_EN FEC_MDC	
}

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int test_entry (unsigned long)
	__attribute__ ((section (".low_power_code")));
int test_entry(unsigned long test_num)
{
	unsigned long action_id;
	unsigned int pmic_cnfg = 0x0;
	unsigned int pmic_glblcnfg0 = 0;
	const char * rev = (const char *) get_board_id16();
	asm ("ldr sp, =0x00960000"); // Change the stack pointer to IRAM

	action_id = test_num;
	/* Mask MMDC channel 0 Handshake */
	reg32_write(0x020c4004, 0x20000);
	//place the proximity into sleep mode
	if (BOARD_IS_WHISKY_WAN(rev)) 
	    board_i2c_write_reg(2,0x28,0x6,0);
#ifdef CONFIG_PMIC_MAX77696
	/* disable FL */
	if (!BOARD_IS_BOURBON(rev)) 
		pmic_fl_enable(0);
#endif

	if(action_id == TEST_ID_SUSPEND_MODE || action_id == TEST_ID_SUSPEND_MODE_MMC_POWER_GATE ) {	
#ifdef CONFIG_PMIC_MAX77696
		// set STBYEN bit
		pmic_read_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG1_REG, &pmic_cnfg);
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG1_REG, 
			(unsigned int)(pmic_cnfg| 1<<GLBLCNFG1_STBYEN_SHIFT));

		// adjust LDO7 voltage down to 1.8 - maintain previous enabled/disabled state
		if (!BOARD_IS_BOURBON(rev)) {
			pmic_read_reg(PM_GPIO_SADDR, (unsigned int)LDO_L07_CNFG1_REG, &pmic_cnfg);
			pmic_write_reg(PM_GPIO_SADDR, (unsigned int)LDO_L07_CNFG1_REG,
				(unsigned int)( (pmic_cnfg & LDO_CNFG1_PWRMD_M) | 0x14) );
		}

		if ( BOARD_IS_BOURBON(rev) || BOARD_IS_WARIO_4_256M_CFG_C(rev)
			|| BOARD_IS_BOURBON_PREEVT2(rev) ) {
#ifdef DEVELOPMENT_MODE
			printf("turning-off LDO touch power in suspend..");
#endif
			reset_touch_zforce2_pins();
			/* delay for pin configs to take effect */
			udelay(100);
			pmic_zforce2_pwrenable(0);
		}
#endif
		
	}
	
	if(action_id == TEST_ID_SUSPEND_MODE_MMC_POWER_GATE) {
		if (BOARD_IS_MUSCAT_WAN(rev) || BOARD_IS_MUSCAT_WFO(rev) ||
			BOARD_IS_WHISKY_WAN(rev) || BOARD_IS_WHISKY_WFO(rev) ||
			BOARD_IS_WOODY(rev) )
			emmc_power_gate_off();
	}

	if(action_id == TEST_ID_SHIPPING_MODE) {
#ifdef CONFIG_PMIC_MAX77696
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG, 0);
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG, 
				(unsigned int)(GLBLCNFG0_FSENT | GLBLCNFG0_SFTPDRR));
#endif		
	} else if(action_id == TEST_ID_HALT_MODE) {
#ifdef CONFIG_PMIC_MAX77696
		if (!BOARD_REV_GREATER_EQ(rev, BOARD_ID_WHISKY_WAN_DVT1_1_REV_C) && 
			!BOARD_REV_GREATER_EQ(rev, BOARD_ID_WHISKY_WFO_DVT1_1_REV_C)) {
			/* Note: WS-1212 Enable LDO bandgap bias when entering FSHDN (SW workaround - option 1b) */
			pmic_write_reg(PM_GPIO_SADDR, (unsigned int)LDO_CNFG3_REG,
				((LDO_CNFG3_IMON_TF_1K_DEF<<LDO_CNFG3_L_IMON_TF_S) | (LDO_CNFG3_BIAS_EN<<LDO_CNFG3_BIASEN_S)));	
		}

		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GPIO_CNFG_GPIO1_REG,
			(1<<GPIO_CNFG_GPIO_PPDRV_S) | (1<<GPIO_CNFG_GPIO_DO_S));
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG, 0);
		/* Note: SFTPDDR is set to 0 to not reset PMIC registers during full-shutdown
		 * this is the only way to retain PMIC GPIO state during full-shutdown.
		 */
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG, 
			(unsigned int)(GLBLCNFG0_FSHDN));
#endif		
	}

	/* Need to mask the Brown out interrupt as PU is disabled. */
	reg32_write(GPC_BASE_ADDR + GPC_IMR3_OFFSET, 0x20000000); //95:64
	reg32_write(GPC_BASE_ADDR + GPC_IMR4_OFFSET, 0x80000000); //127:96

	reg32_write(ANATOP_BASE_ADDR + 0x150, reg32_read(ANATOP_BASE_ADDR + 0x150) | 0x00002000);

	switch(action_id)
	{
		case TEST_ID_SUSPEND_MODE:
		case TEST_ID_SUSPEND_MODE_MMC_POWER_GATE: 
			current_lpm_config = LPM_LAB126_SUSPEND_MODE;
			reg32_write(ANATOP_BASE_ADDR + 0x130, reg32_read(ANATOP_BASE_ADDR + 0x130) | 0x00000008);
			lpm_config_mode(LPM_LAB126_SUSPEND_MODE);
			lpm_config_init();
			system_config_init();
			lpm_enter_exit();
			reg32_write(ANATOP_BASE_ADDR + 0x130, reg32_read(ANATOP_BASE_ADDR + 0x130) & (~0x00000008));
			break;
		case TEST_ID_IDLE_MODE:
			current_lpm_config = LPM_LAB126_IDLE_2_MODE;
			/* 3v2 display power gate will be enabled in idle */
			pmic_enable_display_power_rail(1); 
			/* Idle without ARM powergate mode */
			lpm_config_mode(LPM_LAB126_IDLE_2_MODE); 
			lpm_config_init();
			system_config_init();
			lower_arm_soc_voltages();
			lpm_enter_exit();
			break;
		default:
			break;
	}

	return 0;
}
