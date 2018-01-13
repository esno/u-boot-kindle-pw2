/*
 * Copyright (C) 2014 Amazon.com, Inc. or its affiliates.
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
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

#include <config.h>
#include <asm/io.h>
#include <asm/arch/board-mx6sl_wario.h>

#define IOMUXC_DDR_SEL_DDR2 0x80000
#define IOMUXC_PUE          0x02000
#define IOMUXC_PKE          0x01000
#define IOMUXC_DSE_48       (0x5 << 3)
#define IOMUXC_DSE_60       (0x4 << 3)

#define BOARD_DRAM_HAS_DS_48(id)       (BOARD_IS_BOURBON(id) || \
										BOARD_IS_MUSCAT_WAN(id) || \
										BOARD_IS_MUSCAT_WFO(id) || \
										BOARD_IS_WHISKY_WAN(id) || \
										BOARD_IS_WHISKY_WFO(id))


void lpddr2_init(const struct board_type *board) {
	unsigned long common_dse; 
	/* 
	 * common_dse is pad control bits 5-3 for most high speed DRAM lines, 
	 * 5=48 Ohm, 4=60 Ohm. See IMX6SLRM IOMUXC chapter.
	 */

	if (BOARD_DRAM_HAS_DS_48(board->id)) {
		common_dse = IOMUXC_DSE_48;
	} else {
		common_dse = IOMUXC_DSE_60;
	}

/*
 * CCM Configuration
 */
/* CCM_CBCDR (CCM Bus Clock Divider Register) */
	writel(0x00260324, (CCM_BASE_ADDR + 0x18));


/*
 * IOMUX Configuration
 */
/* IOMUXC_SW_PAD_CTRL_GRP_DDRMODE 
 * Differential input mode on data lines */
	writel(0x00020000, (IOMUXC_BASE_ADDR + 0x5c0));

/* IOMUXC_SW_PAD_CTRL_GRP_DDRPKE
 * Disable Pull/Keeper on DDR pads */
	writel(0x00000000, (IOMUXC_BASE_ADDR + 0x5b4));


/* CLOCK */
/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDCLK0_P */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x338));


/* CONTROL */
/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_CAS_B
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x300));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_RAS_B
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x31c));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_RESET
 * DDR_SEL=DDR2 and common drive strength */
	writel(IOMUXC_DDR_SEL_DDR2 | common_dse, (IOMUXC_BASE_ADDR + 0x320));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDCKE0
 * 60 Ohm drive strength */
	writel(0x00000020, (IOMUXC_BASE_ADDR + 0x330));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDCKE1
 * 30 Ohm drive strength */
	writel(0x00000020, (IOMUXC_BASE_ADDR + 0x334));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDBA2
 * Disabled drive strength */
	writel(0x00000000, (IOMUXC_BASE_ADDR + 0x32c));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_ODT0
 * 240 Ohm drive strength */
	writel(0x00000008, (IOMUXC_BASE_ADDR + 0x33c));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_OTD1
 * 240 Ohm drive strength */
	writel(0x00000008, (IOMUXC_BASE_ADDR + 0x340));

/* IOMUXC_SW_PAD_CTRL_GRP_ADDDS
 * common drive strength on DRAM address lines */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5ac));

/* IOMUXC_SW_PAD_CTRL_GRP_CTLDS
 * common drive strength on CS, SDBA, CKE, and SDWE pads */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5c8));


/* DATA STROBES */
/* IOMUXC_SW_PAD_CTRL_GRP_DDRMODE_CTL
 * Differential input mode on DQ lines */
	writel(0x00020000, (IOMUXC_BASE_ADDR + 0x5b0));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDQS0_P
 * pull, enable pull/keep, common drive strength */
	writel(IOMUXC_PKE | IOMUXC_PUE | common_dse, (IOMUXC_BASE_ADDR + 0x344));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDQS1_P
 * pull, enable pull/keep, common drive strength */
	writel(IOMUXC_PKE | IOMUXC_PUE | common_dse, (IOMUXC_BASE_ADDR + 0x348));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDQS2_P
 * pull, enable pull/keep, common drive strength */
	writel(IOMUXC_PKE | IOMUXC_PUE | common_dse, (IOMUXC_BASE_ADDR + 0x34c));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_SDQS3_P
 * pull, enable pull/keep, common drive strength */
	writel(IOMUXC_PKE | IOMUXC_PUE | common_dse, (IOMUXC_BASE_ADDR + 0x350));


/* DATA */
/* IOMUXC_SW_PAD_CTRL_GRP_DDR_TYPE
 * DDR mode LPDDR2 */
	writel(0x00080000, (IOMUXC_BASE_ADDR + 0x5d0));

/* IOMUXC_SW_PAD_CTRL_GRP_B0DS
 * common drive strength for DRAM_DATA01-DRAM_DATA07 */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5c4));

/* IOMUXC_SW_PAD_CTRL_GRP_B1DS
 * common drive strength for DRAM_DATA08-DRAM_DATA15 */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5cc));

/* IOMUXC_SW_PAD_CTRL_GRP_B2DS
 * common drive strength for DRAM_DATA16-DRAM_DATA23 */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5d4));

/* IOMUXC_SW_PAD_CTRL_GRP_B3DS
 * common drive strength for DRAM_DATA24-DRAM_DATA31 */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x5d8));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_DQM0
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x30c));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_DQM1
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x310));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_DQM2
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x314));

/* IOMUXC_SW_PAD_CTRL_PAD_DRAM_DQM3
 * common drive strength */
	writel(common_dse, (IOMUXC_BASE_ADDR + 0x318));


/*
 * MMDC Configuration
 */
/* MMDC_MDSCR (MMDC Core Special Command Register)
 * Set Configuration Request */
	writel(0x00008000, (MMDC_P0_BASE_ADDR + 0x01c));

/* MMDC_MPZQLP2CTL (MMDC ZQ LPDDR2 HW Control Register) */
	writel(0x1b4700c7, (MMDC_P0_BASE_ADDR + 0x85c));

/* MMDC_MPZQHWCTRL (MMDC PHY ZQ HW control register) */
	writel(0xa1390003, (MMDC_P0_BASE_ADDR + 0x800));

/* MMDC_MPPDCMPR2 (MMDC PHY Pre-defined Compare and CA delay-line Configuration Register) */
	writel(0x00400000, (MMDC_P0_BASE_ADDR + 0x890));

/* MMDC_MPMUR0 (MMDC PHY Measure Unit Register) */
	writel(0x00000800, (MMDC_P0_BASE_ADDR + 0x8b8));

/* MMDC_MPRDQBY0DL (MMDC PHY Read DQ Byte0 Delay Register) */
	writel(0x33333333, (MMDC_P0_BASE_ADDR + 0x81c));

/* MMDC_MPRDQBY1DL (MMDC PHY Read DQ Byte1 Delay Register) */
	writel(0x33333333, (MMDC_P0_BASE_ADDR + 0x820));

/* MMDC_MPRDQBY2DL (MMDC PHY Read DQ Byte2 Delay Register) */
	writel(0x33333333, (MMDC_P0_BASE_ADDR + 0x824));

/* MMDC_MPRDQBY3DL (MMDC PHY Read DQ Byte3 Delay Register) */
	writel(0x33333333, (MMDC_P0_BASE_ADDR + 0x828));

/* MMDC_MPWRDQBY0DL (MMDC PHY Write DQ Byte0 Delay Register) */
	writel(0xf3333333, (MMDC_P0_BASE_ADDR + 0x82c));

/* MMDC_MPWRDQBY1DL (MMDC PHY Write DQ Byte1 Delay Register) */
	writel(0xf3333333, (MMDC_P0_BASE_ADDR + 0x830));

/* MMDC_MPWRDQBY2DL (MMDC PHY Write DQ Byte2 Delay Register) */
	writel(0xf3333333, (MMDC_P0_BASE_ADDR + 0x834));

/* MMDC_MPWRDQBY3DL (MMDC PHY Write DQ Byte3 Delay Register) */
	writel(0xf3333333, (MMDC_P0_BASE_ADDR + 0x838));

/* MMDC_MPRDDLCTL (MMDC PHY Read delay-lines Configuration Register) */
	writel(0x4241444a, (MMDC_P0_BASE_ADDR + 0x848));

/* MMDC_MPWRDLCTL (MMDC PHY Write delay-lines Configuration Register) */
	writel(0x3030312b, (MMDC_P0_BASE_ADDR + 0x850));

/* MMDC_MPDGCTRL0 (MMDC PHY Read DQS Gating Control Register 0) */
	writel(0x20000000, (MMDC_P0_BASE_ADDR + 0x83c));

/* MMDC_MPDGCTRL1 (MMDC PHY Read DQS Gating Control Register 1) */
	writel(0x0, (MMDC_P0_BASE_ADDR + 0x840));

/* MMDC_MPDCCR (MMDC Duty Cycle Control Register) */
	writel(0x24911492, (MMDC_P0_BASE_ADDR + 0x8c0));
    
/* MMDC_MPMUR0 (MMDC PHY Measure Unit Register) */
	writel(0x00000800, (MMDC_P0_BASE_ADDR + 0x8b8));

/* MMDC_MDCFG0 (MMDC Core Timing Configuration Register 0) */
	writel(0x33374133, (MMDC_P0_BASE_ADDR + 0x00c));

/* MMDC_MDPDC (MMDC Core Power Down Control Register) */
	writel(0x00020024, (MMDC_P0_BASE_ADDR + 0x004));

/* MMDC_MDCFG1 (MMDC Core Timing Configuration Register 1) */
	writel(0x00100A82, (MMDC_P0_BASE_ADDR + 0x010));

/* MMDC_MDCFG2 (MMDC Core Timing Configuration Register 2) */
	writel(0x00000093, (MMDC_P0_BASE_ADDR + 0x014));

/* MMDC_MDMISC (MMDC Core Miscellaneous Register) */
	writel(0x00001688, (MMDC_P0_BASE_ADDR + 0x018));

/* MMDC_MDRWD (MMDC Core Read/Write Command Delay Register) */
	writel(0x0f9f26d2, (MMDC_P0_BASE_ADDR + 0x02c));

/* MMDC_MDOR (MMDC Core Out of Reset Delays Register) */
    /* RST_to_CKE changed to 14 cycles as per JEDEC spec
       this affects Idle time after first CKE assertion */
	if (PLATFORM_IS_DUET(board->id))
		writel(0x009F0E10, (MMDC_P0_BASE_ADDR + 0x030));
	else
		writel(0x00000010, (MMDC_P0_BASE_ADDR + 0x030));	

/* MMDC_MDCFG3LP (MMDC Core Timing Configuration Register 3) */
	writel(0x00190778, (MMDC_P0_BASE_ADDR + 0x038));

/* MMDC_MDOTC (MMDC Core ODT Timming Control Register) */
	writel(0x00000000, (MMDC_P0_BASE_ADDR + 0x008));

/* MMDC_MDASP (MMDC Core Address Space Partition Register) */
	writel(0x0000004f, (MMDC_P0_BASE_ADDR + 0x040));

/* MMDC_MDCTL (MMDC Core Control Register) */
	if (board->mem_size == MEMORY_SIZE_512MB) {
		// 10 bits column
		writel(0x83110000, (MMDC_P0_BASE_ADDR + 0x000));
	} else {
		// 9 bits column
		writel(0x83010000, (MMDC_P0_BASE_ADDR + 0x000));
	}

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * LPDDR2 MRW address 0x3f=0x00, chip reset */
	writel(0x003f8030, (MMDC_P0_BASE_ADDR + 0x01c));

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * LPDDR2 MRW address 0x0a=0xff, calibration command */
	writel(0xff0a8030, (MMDC_P0_BASE_ADDR + 0x01c));

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * LPDDR2 MRW address 0x01=0x82, Burst Length 4, Sequential Burst, Wrap, 4 WR clock cycles*/
	writel(0x82018030, (MMDC_P0_BASE_ADDR + 0x01c));

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * LPDDR2 MRW address 0x02=0x04, RL=6, WL=3 */
	writel(0x04028030, (MMDC_P0_BASE_ADDR + 0x01c));

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * LPDDR2 MRW address 0x03=0x03, drive strength 48 ohm */
	writel(0x03038030, (MMDC_P0_BASE_ADDR + 0x01c));


/* MMDC_MPZQHWCTRL (MMDC PHY ZQ Hw control register) */
	writel(0xa1310003, (MMDC_P0_BASE_ADDR + 0x800));

/* MMDC_MDREF (MMDC Core REfresh Control Register) */
	writel(0x00001800, (MMDC_P0_BASE_ADDR + 0x020));

/* MMDC_MPODTCTRL (MMDC PHY ODT control register) */
	writel(0x00000000, (MMDC_P0_BASE_ADDR + 0x818));

/* MMDC_MPMUR0 (MMDC PHY Measurement Unit Register) */
	writel(0x00000800, (MMDC_P0_BASE_ADDR + 0x8b8));

/* MMDC_MDPDC (MMDC Core Power Down Control Register) */
	writel(0x00025564, (MMDC_P0_BASE_ADDR + 0x004));

/* MMDC_MAPSR MMDC Core Power Savings Control and  Status Register */
	writel(0x00011006, (MMDC_P0_BASE_ADDR + 0x404));

/* MMDC_MDSCR (MMDC Core Special Command Register)
 * Clear Configuration Request */
	writel(0x00000000, (MMDC_P0_BASE_ADDR + 0x01c));
}
