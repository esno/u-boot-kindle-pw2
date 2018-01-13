#
# (C) Copyright 2010-2011 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)wario_iomux)
endif

LIB	= $(obj)lib$(BOARD).a

COBJS	:= $(BOARD).o ram_init.o

ifdef CONFIG_PMIC
COBJS	+= pmic.o
endif
COBJS	+= wario_iomux/iomux_config.o \
	   wario_iomux/epdc_iomux_config.o \
	   wario_iomux/ecspi1_iomux_config.o \
	   wario_iomux/gpio1_iomux_config.o \
	   wario_iomux/gpio2_iomux_config.o \
	   wario_iomux/gpio3_iomux_config.o \
	   wario_iomux/gpio4_iomux_config.o \
	   wario_iomux/gpio5_iomux_config.o \
	   wario_iomux/i2c1_iomux_config.o \
	   wario_iomux/i2c2_iomux_config.o \
	   wario_iomux/sjc_iomux_config.o \
	   wario_iomux/uart1_iomux_config.o \
	   wario_iomux/uart2_iomux_config.o \
	   wario_iomux/uart3_iomux_config.o \
	   wario_iomux/uart4_iomux_config.o \
	   wario_iomux/usdhc1_iomux_config.o \
	   wario_iomux/usdhc2_iomux_config.o \
	   wario_iomux/usdhc3_iomux_config.o \
	   wario_iomux/wdog1_iomux_config.o

SOBJS	:= lowlevel_init.o flash_header.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
