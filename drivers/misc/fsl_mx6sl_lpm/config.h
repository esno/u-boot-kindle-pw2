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

typedef struct
{
    unsigned int index;
    char * name;
    char * description;
    unsigned int parameter0;
    unsigned int parameter1;
    unsigned int parameter2;
    unsigned int parameter3;
    int  (* update_call)(unsigned int);
} config_t;

void update_config(config_t * config, unsigned int new_parameter0);
void update_config_lpm(config_t * config, unsigned int new_parameter1);
