/*
 * mx50_gpio.c
 *
 * (C) Copyright 2010 Amazon Technologies, Inc.  All rights reserved.
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
#include <asm/arch/mx6.h>

/* GPIO port description */
static unsigned long gpio_ports[] = {
    [0] = GPIO1_BASE_ADDR,
    [1] = GPIO2_BASE_ADDR,
    [2] = GPIO3_BASE_ADDR,
    [3] = GPIO4_BASE_ADDR,
    [4] = GPIO5_BASE_ADDR,
    [5] = GPIO6_BASE_ADDR,
    [6] = GPIO7_BASE_ADDR,
};


int mx60_gpio_direction(unsigned int port,unsigned int pin, enum mx60_gpio_direction direction)
{   
    u32 val;

    if (port >= ARRAY_SIZE(gpio_ports))
	return -1;
    
    port -= 1;

    val = __REG(gpio_ports[port] + GPIO_GDIR);
    switch (direction) {
      case MX60_GPIO_DIRECTION_OUT:
          val |= 1 << pin;
          break;
      case MX60_GPIO_DIRECTION_IN:
          val &= ~(1 << pin);
          break;
      default:
          break;
    }
    __REG(gpio_ports[port] + GPIO_GDIR) = val;

    return 0;
}

void mx60_gpio_set(unsigned int port,unsigned int pin, unsigned int value)
{
    u32 val;

    if (port >= ARRAY_SIZE(gpio_ports))
	return;

    port -= 1;

    val = __REG(gpio_ports[port] + GPIO_DR);
    if (value)
	val |= 1 << pin;
    else
	val &= ~(1 << pin);
    __REG(gpio_ports[port] + GPIO_DR) = val;
}

int mx60_gpio_get(unsigned int port,unsigned int pin)
{
    if (port >= ARRAY_SIZE(gpio_ports))
	return -1;

    port -= 1;

    return (( __REG(gpio_ports[port] + GPIO_PSR) >> pin) & 1);
}
