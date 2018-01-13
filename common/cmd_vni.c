/*
 * cmd_vni.c  
 *
 * Copyright 2015 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <pmic.h>
#include <pmic_max77696.h>
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/mx6sl_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/gpio.h>
#include <asm/arch/board-mx6sl_wario.h>
#include <boardid.h>
#include <mmc.h>
extern void config_board_100k_pullup(void);

#define atoi(x)         simple_strtoul(x,NULL,10)
#define HALL_SENSOR_IS_DETECT          0x0
#define HALL_SENSOR_IS_NOT_DETECT      0x1
extern int pmic_fg_otp_check(void);
#if defined(CONFIG_WARIO_WOODY)
extern void soda_i2c_sda_pu_config(int op);
extern void soda_i2c_sda_config(int op);
#endif

int do_vni (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int res = 1;  /* default is failure */
	unsigned short val = 0;
	int curr, temp;
	int brightness = 0;
	const char * rev = (const char *) get_board_id16();
        ulong	addr;

    if(strcmp(argv[1], "vddh") == 0) {
          if(strcmp(argv[2], "set") == 0 ){
              if(pmic_vddh_set(argv[3]) != PMIC_VNI_TEST_PASS ) {
                  printf("Set VDDH value failed!\n");
                  res = 1;
              }else {                                 
                  printf("VDDH value set OK\n");
              }
          }
           else {
              cmd_usage(cmdtp);
              return 1;
        }

    }else if(strcmp(argv[1], "vcom") == 0) {
	    if(strcmp(argv[2], "set") == 0 ){
			if(pmic_vcom_set(argv[3]) != PMIC_VNI_TEST_PASS ) {
				printf("Enable Vcom	failed!\n");
				res = 1;
			}else {									
				printf("Turn on VCOM\n");
			}
	    }
	    else if(strcmp(argv[2], "off") == 0 ) {
			if( pmic_enable_vcom(0) != PMIC_VNI_TEST_PASS )	{
				printf("disable Vcom  failed!\n");
				res = 1;
			}else{
				printf("Turn off VCOM\n");
			}
	   }else {
			cmd_usage(cmdtp);
			return 1;
	  }
	}else if(strcmp(argv[1], "batt") == 0) {
	/*will add read battery info here*/	
	    if(strcmp(argv[2], "vol") == 0 ){
			if(!pmic_fg_read_voltage(&val)) {
				printf("Read voltage failed!\n");
				res = 1;
			}else {									
				printf("FG voltage =%dmV\n",val);
			}
	    }
	    else if(strcmp(argv[2], "curr") == 0 ) {
			if(!pmic_fg_read_avg_current(&curr))	{
				printf("Read FG Current failed!\n");
				res = 1;
			}else{
				printf("FG Current = %dmA\n",curr);
			}
	   }
	   else if(strcmp(argv[2], "temp") == 0 ){
		    if(!pmic_fg_read_temperature(&temp)) {
				 printf("Read FG temperature failed!\n");
				 res = 1;
			 }else {								 
				printf("FG Temperature = %dC\n",temp);
			 }
		 }
	   else if(strcmp(argv[2], "cap") == 0 ) {
			 if(!pmic_fg_read_capacity(&val)) {
				 printf("Read FG capacity  failed!\n");
				 res = 1;
			 }else{
				 printf("FG Capacity = %d Percent\n",val);
			 }
		}
	   else {
		   cmd_usage(cmdtp);
		   return 1;

	   	}
	   
	}else if(strcmp(argv[1], "chg") == 0) {
  	       if(strcmp(argv[2], "trickle") == 0 ){
                    printf("Set charge current to 66.7mA\n");
                    pmic_charger_set_current(MAX77696_CHARGER_CURRENT_TRICKLE);
  	       }else if(strcmp(argv[2], "default") == 0 ) {
  	            printf("Set charge current to 300mA\n");
                    pmic_charger_set_current(MAX77696_CHARGER_CURRENT_DEFAULT);
               }else if(strcmp(argv[2], "fast") == 0 ) {
                    printf("Set charge current to 466mA\n");
	            pmic_charger_set_current(MAX77696_CHARGER_CURRENT_FAST);
               } else if(strcmp(argv[2], "custom") == 0 ) {
                   if (argc == 4) {
                       curr = simple_strtoul(argv[3], NULL, 16);
                       if(curr < MAX77696_CHARGER_CURRENT_MIN || curr > MAX77696_CHARGER_CURRENT_MAX) {
                                printf("Failed. Valid charge current range is 0x00 to 0x3f \n");
                                return PMIC_VNI_TEST_FAIL;
                       }                      
                       printf("Success. Charge current set to 0x%x\n", curr);
                       pmic_charger_set_current(curr);
                   }
                   else {
                       cmd_usage(cmdtp);
                       return 1;
                   }
               } else {
	            cmd_usage(cmdtp);
	            return 1;
	            }
	}else if(strcmp(argv[1], "qboot") == 0){
		if(strcmp(argv[2], "pullup") == 0 ) {
            config_board_100k_pullup();
            }
        else if(strcmp(argv[2], "low") == 0 ) {	
            addr = 0x209c000;
            *((uint   *)addr) = *((uint   *)addr) & 0xefffffff;
            printf(" imx6 register 0x209c000 = 0x%8x\n", *((uint   *)addr));
            }
        else if(strcmp(argv[2], "high") == 0 ) {
            addr = 0x209c000;
            *((uint   *)addr) = *((uint   *)addr) | 0x10000000;
            printf(" imx6 register 0x209c000 = 0x%8x\n", *((uint   *)addr));
            }
        else if(strcmp(argv[2], "state") == 0 ) {
            addr = 0x20A8000;	
            printf(" imx6 register 0x20A8000 = 0x%8x\n", *((uint   *)addr));
            if ((*((uint   *)addr)) & 0x00000080)
                printf("<HIGH>\n");
            else
                printf("<LOW>\n");
            }
        else if(strcmp(argv[2], "read_on_off") == 0 ){
            if( pmic_i2c_read_reg(PM_GPIO_SADDR,0x05,&val) == 0) {
                if(val & 0x00c0)
                    printf("<PASS>\n");
                else
                    printf("<FAIL>\n");
                 }
            else
                printf("<FAIL>\n");
            }
        if(strcmp(argv[2], "clear_on_off") == 0 ){
            pmic_i2c_read_reg(PM_GPIO_SADDR,0x05,&val);
        }
     }
    else if(strcmp(argv[1], "gpio") == 0) {
		if ((argc == 4) && (strcmp(argv[2], "3gm_shutdown") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set 3gm_shutdown to %d\n", curr);
			gpio_direction_output(MX6_WARIO_3GM_SHUT_DOWN, curr);
		}

#if defined(CONFIG_WARIO_WOODY)
		if ((argc == 4)  && (strcmp(argv[2], "brcm_wifi") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set brcm wifi to %d\n", curr);
			gpio_direction_output(MX6SL_WARIO_WL_REG_ON, curr);
		}

		if ((argc == 4)  && (strcmp(argv[2], "brcm_bt") == 0)) {
			 curr = atoi(argv[3]) ? 1 : 0;
			 printf("set brcm bt to %d\n", curr);
			 gpio_direction_output(MX6SL_WARIO_BT_REG_ON, curr);
		}

		if ((argc == 4)  && (strcmp(argv[2], "alta_3g") == 0)) {
			 curr = atoi(argv[3]) ? 1 : 0;
		         printf("set alta 3g to %d\n", curr);
			 gpio_direction_output(MX6SL_WARIO_3GM_POWER_ON, curr);
		}

		if ((argc == 4)  && (strcmp(argv[2], "soda_vbus_en") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set soda vbus enable to %d\n", curr);
			gpio_direction_output(MX6_SODA_VBUS_ENABLE, curr);
		}

		if ((argc == 4)  && (strcmp(argv[2], "soda_boost_dis") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set soda boost disable to %d\n", curr);
			if (BOARD_REV_GREATER(rev, BOARD_ID_WHISKY_WAN_HVT1) || BOARD_REV_GREATER(rev, BOARD_ID_WHISKY_WFO_HVT1) ||
				BOARD_REV_GREATER_EQ(rev, BOARD_ID_WOODY_2)) {
				pmic_gpio1_direction_output(curr);
			} else {
				gpio_direction_output(MX6_SODA_BOOST, curr);
			}
		}

		if ((argc == 4)  && (strcmp(argv[2], "soda_otg_sw") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set soda otg mlb sw to %d\n", curr);
			gpio_direction_output(MX6_SODA_OTG_SW, curr);
		}

		if ((argc == 4)  && (strcmp(argv[2], "soda_i2c_sda_pu") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			if (curr) {
				printf("set soda i2c sda pullup to output/HI :%d\n", curr);
				soda_i2c_sda_pu_config(1);
				gpio_direction_output(MX6_SODA_I2C_SDA_PU, curr);
			} else {
				printf("set soda i2c sda pullup to input/TRISTATE :%d\n", curr);
				soda_i2c_sda_pu_config(0);
				gpio_direction_input(MX6_SODA_I2C_SDA_PU);
			}
		}

		if ((argc == 4)  && (strcmp(argv[2], "soda_i2c_sda") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			if (curr) {
				printf("set soda i2c sda to output/HI :%d\n", curr);
				soda_i2c_sda_config(1);
				gpio_direction_output(MX6_SODA_I2C_SDA, curr);
			} else {
				printf("set soda i2c sda to input/TRISTATE(dock interrupt) :%d\n", curr);
				soda_i2c_sda_config(0);
				gpio_direction_input(MX6_SODA_I2C_SDA);
			}
		}

		if ((argc == 3)  && (strcmp(argv[2], "soda_ext_chg_det") == 0)) { 
			if(gpio_get_value(MX6_SODA_CHG_DET)) 
				printf("ext_charger disconnected\n");
			else 
				printf("ext_charger connected\n");
		}

		if ((argc == 3)  && (strcmp(argv[2], "soda_dock_det") == 0)) { 
			if(gpio_get_value(MX6_SODA_I2C_SDA)) 
				printf("soda undocked\n");
			else 
				printf("soda docked\n");
		}
#endif
				
	} else if(strcmp(argv[1], "display") == 0) {
		unsigned int rail_enable;

		if (strcmp(argv[2], "on") == 0) {
			rail_enable = 1;
		} else {
			rail_enable = 0;
		}
		printf("set LSW4 to %d\n", rail_enable);
		pmic_enable_display_power_rail(rail_enable);

	}else if(strcmp(argv[1], "fl") == 0) {	         
             brightness = simple_strtoul(argv[2], NULL, 10); 
			 printf("set front light brightness:%d\n",brightness);
             pmic_fl_set(brightness);

    } else if(strcmp(argv[1], "fg") == 0) {
       if (strcmp(argv[2], "otpchk") == 0) {
            printf("MAX77696-FG OTP TIMER TEST\n");
            pmic_fg_otp_check();
       } else {
           cmd_usage(cmdtp);
           return 1;
       }
	}else {
	       cmd_usage(cmdtp);
	       return 1;
	}

	return 0;
}

void mx60_wan4v2_ldo_gpio(int on)
{
	mxc_iomux_v3_setup_pad(MX6SL_PAD_EPDC_PWRCTRL3__GPIO_2_10);
	/* set as output high */
	gpio_direction_output(MX6SL_ARM2_EPDC_PWRCTRL3, on);
}
void mx60_wan4v2_fet_gpio(int on)
{
	mxc_iomux_v3_setup_pad(MX6SL_PAD_EPDC_PWRCTRL1__GPIO_2_8);
	/* set as output high */
	gpio_direction_output(MX6SL_ARM2_EPDC_PWRCTRL1, on);
}

int do_wan4v2 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	
	if(strcmp(argv[1], "ldo") == 0) {
	    mx60_wan4v2_fet_gpio(0);//turn off fet first
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_ldo_gpio(1);//turn on ldo
	    printf("Turn on 3GM_LDO_EN\n");
	}
	else if (strcmp(argv[1], "fet") == 0) {
	    mx60_wan4v2_ldo_gpio(0);//turn off ldo
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_fet_gpio(1);//turn on fet 
	    printf("Turn on 3GM_FET_EN\n");
	}
	else if (strcmp(argv[1], "off") == 0) {
	    mx60_wan4v2_ldo_gpio(0);//turn off ldo
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_fet_gpio(0);//turn off fet 
	    printf("Turn off 3GM_FET_EN and 3GM_LDO_EN\n");
	}
	else {
	    cmd_usage(cmdtp);
	    return 1;

	}
        return 0;
}


int do_hall (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{	
       int rval,hall_val;
       rval = gpio_direction_input(MX6_WARIO_HALL_SNS);
       if ( rval != 0 ) {
            printf("%s gpio_direction_input(MX6_WARIO_HALL_SNS) failed, rval %d\n", __func__, rval);
            return rval;
       }
       hall_val = gpio_get_value(MX6_WARIO_HALL_SNS);
       if( strcmp(argv[1], "not_detect") == 0 ) {
	    if( hall_val == HALL_SENSOR_IS_NOT_DETECT ) {// test hall not detect status 
	   	 printf("HALL NOT_DETECT TEST PASS\n");

	    }else {
                 printf("HALL NOT_DETECT TEST FAIL\n");

	   }
	   

	}else if ( strcmp(argv[1], "detect") == 0 ) {
	   if( hall_val == HALL_SENSOR_IS_DETECT ) { // test hall detect status 
		 printf("HALL DETECT TEST PASS\n");
		
	   }else {
		 printf("HALL DETECT TEST FAIL\n");		
           }

	}
	else {
	    cmd_usage(cmdtp);
	    return 1;

	}
        return 0;
}


/***************************************************/

U_BOOT_CMD(
	vni,	5,	1,	do_vni,
	"vni    - vni pmic test\n",
	"[vddh|vcom|batt|chg|fg|qboot] args...\n"
	"vddh set <vaule>\n"
	"    - set vddh value (15000mV to 29570mV\n"
	"vcom set <vaule> or <off>\n"
	"    - set vcom value/turn off vcom\n"
	"batt [vol|temp|curr|cap]\n"
	"    - read battery infomation\n"
	"chg [trickle|default|fast|custom <value>]\n"
	"    - set fast charge current\n"
	"    - custom <value>: value ranage is 0x00 to 0x3f\n"
	"gpio 3gm_shutdown [0/1]\n"
    	"qboot [pullup|low|high|read_on_off|clear_on_off]\n"
	"    - pullup : configure HALL_SNS_1P8 to 100k pullup\n"
	"    - low    : drive QBOOT_ENA_B low\n"
	"    - high   : drive QBOOT_ENA_B high\n"
	"    - read_on_off : read on/off switch flags in MAX77696\n"
	"    - clear_on_off : clear on/off switch flags in MAX77696\n"

#if defined(CONFIG_WARIO_WOODY)
	"gpio brcm_wifi [0/1]\n"
	"gpio brcm_bt [0/1]\n"
	"gpio alta_3g [0/1]\n"
	"gpio soda_vbus_en [0=output-lo / 1=output-hi]\n"
	"gpio soda_boost_dis [0=output-lo / 1=output-hi]\n"
	"gpio soda_otg_sw [0=output-lo / 1=output-hi]\n"
	"gpio soda_i2c_sda_pu [0=input-tristate / 1=output-hi]\n"
	"gpio soda_i2c_sda [0=input-tristate / 1=output-hi]\n"
	"gpio soda_ext_chg_det\n"
	"gpio soda_dock_det\n"
#endif
	"display [on/off]\n"
	"fg otpchk\n"
);

U_BOOT_CMD(
	wan4v2,	2,	1,	do_wan4v2,
	"wan4v2    - WAN module 4V2 control\n",
	"[fet|ldo|off] args...\n"
	"wan4v2 fet\n"
	"    - turn off ldo then turn on fet\n"
	"wan4v2 ldo\n"
	"    - turn off fet then turn on ldo \n"
	"wan4v2 off\n"
	"    - turn off fet and ldo\n"
);
/////////////////////////////////////////////////////////////////////////

int do_read_mmc_revision(void)
{
	struct mmc *mmc;
	int i = 0;
	mmc = find_mmc_device(1);

	if (mmc) 
        {
	    mmc_init(mmc);
	    int rev = (mmc->cid[2] >> 16) & 0xff;	
	    printf("eMMC Revision:0x%x\n",rev);	
	    return 0;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}



typedef struct _emmc_vendors_list
{
	int vendor_id;
	char *vendor_name;
}EMMC_VENDORS_LIST;

EMMC_VENDORS_LIST emmc_vendors[] =
{
	{0x11, "Toshiba"},
	{0x15, "Samsung"},
	{0x45, "Sandisk"},
	{0x90, "Hynix"},
	{0xfe, "Micron"},
	{0, NULL}
};




int do_read_mmc_mfgid(void)
{
	struct mmc *mmc;
	int i = 0;
	mmc = find_mmc_device(1);

	if (mmc) 
        {
	    mmc_init(mmc);
	    int mfgid = mmc->cid[0] >> 24;
	    while(emmc_vendors[i++].vendor_id != 0)
	    {
		if(emmc_vendors[i].vendor_id == mfgid)
		{
			printf("eMMC vendor: %s\n",emmc_vendors[i].vendor_name);
			return 0;
		}
	    }	
	    printf("No eMMC vendor is found,mfgid=%x\n",mfgid);	
	    return 1;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}

//*********************************************************************/

#define _2K_MB_EMMC_SIZE (2*1024)
#define _4K_MB_EMMC_SIZE (4*1024)
#define _1MB (1024*1024)

int do_read_mmc_capacity(void)
{
	struct mmc *mmc;

	mmc = find_mmc_device(1);

	if (mmc) 
        {
	    mmc_init(mmc);
		int mmccap = mmc->capacity/_1MB;

	    if ( mmccap > 0 && mmccap <= _2K_MB_EMMC_SIZE )
	    {
		    printf("eMMC Capacity:2GB\n");
	    }
	    else if ( mmccap > _2K_MB_EMMC_SIZE  && mmccap <= _4K_MB_EMMC_SIZE )
	    {
		    printf("eMMC Capacity:4GB\n");
	    }else
	    {
		    printf("Capacity: %lld\n", mmc->capacity);
	    }
	    return 0;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}


int do_vni_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	if(strcmp(argv[1], "cap") == 0) {
		return do_read_mmc_capacity();

	}else if (strcmp(argv[1], "mfgid") == 0) {
		return do_read_mmc_mfgid();
        }else if (strcmp(argv[1], "ver") == 0) {
		return do_read_mmc_revision();
	}else {
		cmd_usage(cmdtp);
		return 1;
	}
	return 1;
}


U_BOOT_CMD(
	vnimmc, 2, 0, do_vni_mmc,
	"display MMC mfgid",
	"[cap|mfgid|ver] args...\n"
	"cap\n"
	"    - get the eMMC capacity\n"
	"mfgid\n"
	"    - get the eMMC vendor name \n"
	"ver\n"
	"    - get the eMMC FW version \n"
);


U_BOOT_CMD(
	hall,	2,	1,	do_hall,
	"hall    - hall test\n",
	"[not_detect|detect] args...\n"
	"hall not_detect\n"
	"    - test the hall not detect\n"
	"hall detect\n"
	"    - test the hall detec \n"
);


