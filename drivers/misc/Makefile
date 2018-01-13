#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

#ifdef CONFIG_LAB126_LPM_TEST
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)fsl_mx6sl_lpm)
endif
#endif

LIB	:= $(obj)libmisc.a

AOBJS-$(CONFIG_LAB126_LPM_TEST) += fsl_mx6sl_lpm/suspend_resume.o

COBJS-$(CONFIG_ALI152X) += ali512x.o
COBJS-$(CONFIG_DS4510)  += ds4510.o
COBJS-$(CONFIG_FSL_LAW) += fsl_law.o
COBJS-$(CONFIG_IMX_IIM) += imx_iim.o
COBJS-$(CONFIG_FSL_OTP) += fsl_otp.o
COBJS-$(CONFIG_IMX_OTP) += imx_otp.o
COBJS-$(CONFIG_IMX_PWM) += imx_pwm.o
COBJS-$(CONFIG_NS87308) += ns87308.o
COBJS-$(CONFIG_STATUS_LED) += status_led.o
COBJS-$(CONFIG_TWL4030_LED) += twl4030_led.o
COBJS-$(CONFIG_PMIC_13892) += pmic_13892.o
COBJS-$(CONFIG_PMIC_34708) += pmic_34708.o
COBJS-$(CONFIG_PMIC_MAX77696) += pmic_max77696.o
COBJS-$(CONFIG_LAB126_LPM_TEST) += fsl_mx6sl_lpm/test_entry.o \
  				   fsl_mx6sl_lpm/common_api.o \
				   fsl_mx6sl_lpm/config.o \
				   fsl_mx6sl_lpm/dram_api.o \
				   fsl_mx6sl_lpm/low_power_mode.o \
				   fsl_mx6sl_lpm/system_config.o
COBJS-$(CONFIG_CMD_HAPTIC) += drv2667.o
COBJS-$(CONFIG_CMD_FSR) += fsr_keypad.o

AOBJS   := $(AOBJS-y)
COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c) 
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
