/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Auto Generate file, please don't edit it
 *
 */

#ifndef __MACH_MX6_GPIO_H__
#define __MACH_MX6_GPIO_H__

#include <asm/arch/mx6.h>
#include <asm/arch/iomux.h>

#define IMX_GPIO_NR(bank, nr)		(((bank) - 1) * 32 + (nr))

enum mxc_gpio_direction {
	MXC_GPIO_DIRECTION_IN,
	MXC_GPIO_DIRECTION_OUT,
};

void gpio_set_value(int gpio, int value);
int gpio_get_value(int gpio);
int gpio_request(int gp, const char *label);
void gpio_free(int gp);
void gpio_toggle_value(int gp);
int gpio_direction_input(int gp);
int gpio_direction_output(int gp, int value);

/*!
 * @file mach-mx6/gpio.h
 *
 * @brief Simple GPIO definitions and functions
 *
 * @ingroup GPIO_MX6
 */
#endif
