/*
 * Copyright (C) 2012, Freescale Semiconductor, Inc. All Rights Reserved.
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

// File: iomux_config.c

#include "include/iomux_config.h"

// Function to configure iomux for i.MX6SL BGA board Wario rev. 1.
void iomux_config(void)
{
    /* !! Any change to pad settings here should be reflected
     * in linux as well and vice versa  
     */

    gpio1_iomux_config();
    gpio2_iomux_config();
    gpio3_iomux_config();
    gpio4_iomux_config();
    gpio5_iomux_config();

    sjc_iomux_config(); 

    uart2_iomux_config();
    uart3_iomux_config();

    //wdog
    wdog1_iomux_config();
}

// Definitions for unused modules.
void ccm_iomux_config(void)
{
};

void cortex_a9_iomux_config(void)
{
};

void ecspi2_iomux_config(void)
{
};

void ecspi3_iomux_config(void)
{
};

void ecspi4_iomux_config(void)
{
};

void epit1_iomux_config(void)
{
};

void epit2_iomux_config(void)
{
};

void fec_iomux_config(void)
{
};

void gpt_iomux_config(void)
{
};

void i2c3_iomux_config(void)
{
};

void kpp_iomux_config(void)
{
};

void lcdif_iomux_config(void)
{
};

void pwm2_iomux_config(void)
{
};

void pwm3_iomux_config(void)
{
};

void pwm4_iomux_config(void)
{
};

void sdma_iomux_config(void)
{
};

void snvs_iomux_config(void)
{
};

void spdc_iomux_config(void)
{
};

void spdif_iomux_config(void)
{
};

void src_iomux_config(void)
{
};

void uart5_iomux_config(void)
{
};

void usb_iomux_config(void)
{
};

void usdhc4_iomux_config(void)
{
};

void wdog2_iomux_config(void)
{
};

void weim_iomux_config(void)
{
};

void xtalosc_iomux_config(void)
{
};
