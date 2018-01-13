ifeq ($(TYPE),prod)
LDSCRIPT := $(SRCTREE)/board/$(VENDOR)/$(BOARD)/u-boot.lds

TEXT_BASE = 0x00980000
endif

ifeq ($(TYPE),bist)

ifndef CONFIG_LAB126_LPM_TEST
LDSCRIPT := $(SRCTREE)/board/$(VENDOR)/$(BOARD)/bist.lds
else
LDSCRIPT := $(SRCTREE)/board/$(VENDOR)/$(BOARD)/bist_lpm.lds
endif

TEXT_BASE = 0x80400000
endif

ifeq ($(TYPE),mfgtool)
LDSCRIPT := $(SRCTREE)/board/$(VENDOR)/$(BOARD)/mfgtool.lds

TEXT_BASE = 0x80500000
endif
