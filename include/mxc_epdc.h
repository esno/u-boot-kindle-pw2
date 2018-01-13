/*
 * Copyright 2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 */
#pragma once
#include <lcd.h>

void mxc_epdc_vidinfo_for_barcode(const char *barcode_prefix, vidinfo_t *info, struct epdc_timing_params *timings);

int setup_waveform_file();
void epdc_power_on(void);
void epdc_power_off(void);
void blit_splash_img(void *fb, int fb_width, int fb_height, int *x0, int *y0, int *width, int *height);
void blit_crit_batt_img(void *fb, int fb_width, int fb_height, int *x0, int *y0, int *width, int *height);
void setup_epdc(void);
