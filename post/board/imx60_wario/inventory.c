/*
 * inventory.c
 *
 * Copyright 2012-2015 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */


#include <common.h>
#include <post.h>

#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/mx6sl_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/gpio.h>

#include "lab126_mxc_i2c.h"

#include <boardid.h>
#include <fsr_keypad.h>
#include <drv2667.h>

#ifdef CONFIG_PMIC
#include <pmic.h>
#endif

#ifdef CONFIG_PMIC_MAX77696
#include <pmic_max77696.h>
#endif

extern const u8 *get_board_id16(void);
#define DBG(fmt,args...)\
		serial_printf("[%s] %s:%d: "fmt,\
				__FILE__,__FUNCTION__,__LINE__, ##args)

#if CONFIG_POST

#define MAX77696_FG_REG_CONFIG        0x1D
#define MAX77696_UIC_REG_ENUCTRL      0x0C
#define MAX77696_EPD_REG_EPDINTM      0x63
#define MAX77696_RTC_REG_RTCDOWAX     0x11
#define ACCELEROMETER_I2C_ADDR        0x18
#define PROXIMITY_I2C_ADDR            0x28
#define ACC_REG_OFC_OFFSET_Z          0x3A
#define PROX_REG_SAR_CTRL             0x10
#define I2C_DEV_ACC_PROX              2
#define I2C_2_ALS                     0x4A
#define I2C_2_KL05                    0x58
#define I2C_2_PROX                    0x0D        
#define I2C_2_HAPTIC                  0x59
#define HAPTIC_RECOVERY_ATTEMPTS      3
#define ACC_REG_SW_RESET_OFFSET       0x14
#define ACC_SW_RESET_VALUE            0xB6
#define ACC_REG_PMU_RANGE_OFFSET      0x0F
#define ACC_PMU_RANGE_VALUE           0x08
#define ACC_REG_MSBX_OFFSET           0x03
#define ACC_REG_MSBY_OFFSET           0x05
#define ACC_REG_MSBZ_OFFSET           0x07
#define ACC_REG_SELF_TEST_OFFSET      0x32
#define ACC_SELF_TEST_POSX_VALUE      0x15
#define ACC_SELF_TEST_NEGX_VALUE      0x11
#define ACC_SELF_TEST_POSY_VALUE      0x16
#define ACC_SELF_TEST_NEGY_VALUE      0x12
#define ACC_SELF_TEST_POSZ_VALUE      0x17
#define ACC_SELF_TEST_NEGZ_VALUE      0x13
typedef struct{
    uint origin_val;
    uint test_val;
    uint now_val;
}inventory;


int board_i2c_read_reg(unsigned int dev, unsigned char saddr, unsigned int reg, unsigned int *val) 
{
    int ret;

    *val = 0;

    i2c_set_bus_num(dev);
    switch(saddr) {
#ifdef CONFIG_PMIC_MAX77696
    case FG_SADDR:
      ret = i2c_read(saddr, reg, 1, (unsigned char *) val, 2);
    break;
#endif
    default:
      ret = i2c_read(saddr, reg, 1, (unsigned char *) val, 1);
    break;
    }
    if (ret)
    {
        DBG("\n%s: failed! for sddr %x, regaddr: %x \n", __func__, saddr, reg);
	return 0;
    }

    return 1;
}

int board_i2c_write_reg(unsigned int dev,unsigned char saddr, unsigned int reg, const unsigned int val) 
{
    int ret;

    i2c_set_bus_num(dev);
    switch(saddr) {
#ifdef CONFIG_PMIC_MAX77696
    case FG_SADDR: 
      ret = i2c_write(saddr, reg, 1, (unsigned char *) &val, 2);
    break;
#endif
    default:
      ret = i2c_write(saddr, reg, 1, (unsigned char *) &val, 1);
    break;
    }

    if (ret)
    {
        DBG("\n%s: failed! for sddr %x, regaddr: %x \n", __func__, saddr, reg);
	return 0;
    }

    return 1;
}


/* Read register value from i2c slave.
 *
 * unsigned char saddr ----slave address
 * unsigned int reg    ----register to read
 * unsigned int *val   ----register value
 */
int pmic_i2c_read_reg(unsigned char saddr,unsigned int reg,unsigned int *val)
{
    int retry = 3;
    while(retry--)
    {
        if(board_pmic_read_reg(saddr,reg,val))
        {
            break;
        }
    }
    if(retry <=0 )
    {
        return 1;
    }
    udelay(100);

    return 0;
}
/* Write value to i2c slave.
 *
 * unsigned char saddr ----slave address
 * unsigned int reg    ----register to write to
 * unsigned int *val   ----register value
 */
int pmic_i2c_write_reg(unsigned char saddr,unsigned int reg,unsigned int val)
{
    int retry = 3;
    while( retry-- )
    {
        if( board_pmic_write_reg(saddr,reg,val))
        {
            break;
        }
    }
    if(retry<=0)
    {
        return 1;
    }
    udelay(100);
    return 0;
}
enum
{
    I2C_PASS           = 0,
    I2C_FAIL           = -1,
    I2C_TEST_NOT_MATCH = -2
};

 int check_i2c_dev (uint dev, uint saddr,uchar reg,uint check_val)
 {
	 int ret = I2C_FAIL;
	 //int bus;
	 inventory temp;
	 temp.test_val = check_val;
 
	 if( board_i2c_read_reg(dev,saddr,reg,&temp.origin_val) == 1)
	 {
		 if( board_i2c_write_reg(dev,saddr,reg,temp.test_val) == 1 )
		 {
			 if( board_i2c_read_reg(dev, saddr,reg,&temp.now_val) == 1 )
			 {
				 if(temp.test_val == temp.now_val)
				 {
					 ret = I2C_PASS;
				 }
				 else
				 {
					 ret = I2C_TEST_NOT_MATCH;
				 }
			 }
			 /**
			  Restore the original value no matter if it pass or fail
			  */
			 if( board_i2c_write_reg(dev,saddr,reg,temp.origin_val) != 1 )
			 {
				 ret = I2C_FAIL;
			 }
		 }
	 }
 
	 switch (ret)
	 {
		 case I2C_PASS:
			 printf("PASS\n");
			 break;
		 case I2C_TEST_NOT_MATCH:
			 printf("Test value mismatch\n");
			 break;
		 case I2C_FAIL:
		 default:
			 printf("I2C bus failed\n");
			 break;
	 }
	 //i2c_set_bus_num(bus);
	 return ret;
 }

 int check_i2c_acc (void)
 {
 
	 printf("Accelerometer ");
	 return check_i2c_dev(I2C_DEV_ACC_PROX,ACCELEROMETER_I2C_ADDR,ACC_REG_OFC_OFFSET_Z,0x55);
 }

 int check_i2c_prox (void)
{

    printf("Proximity ");
    return check_i2c_dev(I2C_DEV_ACC_PROX,PROXIMITY_I2C_ADDR,PROX_REG_SAR_CTRL,0x55);
}


 /* test max77696 i2c slave.
 *
 * uint saddr       ----slave address
 * uchar reg        ----register to read
 * uint check_val   ----write this value to the register,then read it again then check.
 * 1. read the value from the register, save it
 * 2. write the check_val to the register, then read that register again to check the
 *    vaule is equal to the check_val or not, if equal, then pass, not will fail
 * 3. write the original value back to the register, make sure it does destroy anything
 */
int check_max77696 (uint saddr,uchar reg,uint check_val)
{
    int ret = I2C_FAIL;
    //int bus;
    inventory temp;
    temp.test_val = check_val;

    //bus = i2c_get_bus_num();

    if( pmic_i2c_read_reg(saddr,reg,&temp.origin_val) == 0)
    {
        if( pmic_i2c_write_reg(saddr,reg,temp.test_val) == 0 )
        {
            if( pmic_i2c_read_reg(saddr,reg,&temp.now_val) == 0 )
            {
                if(temp.test_val == temp.now_val)
                {
                    ret = I2C_PASS;
                }
                else
                {
                    ret = I2C_TEST_NOT_MATCH;
                }
            }
            /**
             Restore the original value no matter if it pass or fail
             */
            if( pmic_i2c_write_reg(saddr,reg,temp.origin_val) )
            {
                ret = I2C_FAIL;
            }
        }
    }

    switch (ret)
    {
        case I2C_PASS:
            printf("PASS\n");
            break;
        case I2C_TEST_NOT_MATCH:
            printf("Test value mismatch\n");
            break;
        case I2C_FAIL:
        default:
            printf("I2C bus failed\n");
            break;
    }
    //i2c_set_bus_num(bus);
    return ret;
}
/* check fuel gauge.
 *
 * unsigned char saddr ----0x34
 * unsigned int reg    ----0x1d
 * unsigned int *val   ----0x5555
 */
int check_max77696_fuelgauge (void)
{

    printf("Max77696 FG ");
    return check_max77696(FG_SADDR,MAX77696_FG_REG_CONFIG,0x5555);
}
/* check usb interface circuit.
 *
 * unsigned char saddr ----0x35
 * unsigned int reg    ----0x0c
 * unsigned int *val   ----0x03
 */
int check_max77696_uic (void)
{
    printf("\nMax77696 UIC ");
    return check_max77696(USBIF_SADDR,MAX77696_UIC_REG_ENUCTRL,0x03);
}
 /* check max77696 core(io/eink display supplies/..).
 *
 * unsigned char saddr ----0x3c
 * unsigned int reg    ----0x63
 * unsigned int *val   ----0x03
 */
int check_max77696_core (void)
{
    printf("Max77696 Core ");
    return check_max77696(PM_GPIO_SADDR,MAX77696_EPD_REG_EPDINTM,0x03);
}

 /* check rtc.
 *
 * unsigned char saddr ----0x68
 * unsigned int reg    ----0x11
 * unsigned int *val   ----0x03
 */
int check_max77696_rtc (void)
{
    printf("Max77696 RTC ");
    return check_max77696(RTC_SADDR,MAX77696_RTC_REG_RTCDOWAX,0x03);
}

/**
 *
 */
int check_fsr_keypad(void)
{
    /**
     * From bootloader mode
     */
    int rval = -1;
    struct fsr_simple_cmd hook_cmd =
    {
        FSR_SIMPLE_CMD_LEN  ,
        FSR_CMD_HOOK        ,
        0x08
    };
    u8 buffer       [2];

    rval = i2c_set_bus_num(FSR_I2C_BUS_NUM);
    if (rval != 0)
    {
        printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, FSR_I2C_BUS_NUM);
        return FSR_ERR_I2C;
    }
    rval = i2c_write(FSR_I2C_ADDR, 0, 0, (u8 *) &hook_cmd, FSR_SIMPLE_CMD_LEN);
    if (rval == 0)
    {
        rval = i2c_raw_read(FSR_I2C_ADDR, buffer, FSR_HOOK_CMD_LEN);
        if (rval == 0)
        {
            if ((buffer[0] == (FSR_CMD_ACK_FLAG | FSR_CMD_HOOK)) &&
                (buffer[1] == FSR_CMD_ACK))
            {
                printf("FSR check PASSED, rval: 0x%02x 0x%02X\n", buffer[0], buffer[1]);
                return FSR_SUCCESS ;
            }
            printf("FSR hook failed, rval: 0x%02x 0x%02X\n", buffer[0], buffer[1]);
        }
    }
    else 
    {
	printf("%s: i2c_write error \n", __FUNCTION__);
    }
    return rval;
}

int check_haptic(void)
{
    int rval;
    u8  val;
    uchar chip_id = 0;

    rval = i2c_set_bus_num(HAPTIC_I2C_BUS_NUM);
    if (rval != 0)
    {
        printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, HAPTIC_I2C_BUS_NUM);
        return HAPTIC_ERR_I2C;
    }
    
    val = HAPTIC_DEV_RST;
    if( i2c_write(HAPTIC_I2C_ADDR, HAPTIC_REG_CONTROL_2, 1, &val, 1) )
    {   
        printf("%s:failed to reset!\n", __func__);
        return -1;
    }
    udelay(1000);
    
    val = HAPTIC_CONTROL_SPACE;
    rval = i2c_write(HAPTIC_I2C_ADDR, HAPTIC_REG_PAGE, 1, &val, 1);
    if (rval == 0)
    {
        rval = i2c_read(HAPTIC_I2C_ADDR, HAPTIC_REG_CONTROL, 1, &val, 1);
        if (rval == 0)
        {
            chip_id = (val >> 3) & 0x0F;
            printf("%s: ID = 0x%02x\n", __FUNCTION__, (val >> 3) & 0x0F);
            if (chip_id == HAPTIC_CHIP_ID)
            {
                return 0;
            }
            printf("%s: haptic ID does not match (0x%02x != 0x%02x)\n", __FUNCTION__,
                       chip_id, HAPTIC_CHIP_ID);
        }
        else
        {
            printf("%s: Cannot read haptic ID\n", __FUNCTION__);
        }
    }
    else 
    {
        printf("%s: i2c_write error \n", __FUNCTION__);
    }
    return -1;
}

/*
 * we need to show if the device was in a bad state or not. There are multiple reasons for this. 
 * 1) the 1st i2c probe loop would show the status before reset, and the second probe would show the status after; 
 * 2) the second i2c probe resets the bus so that we can talk to the reappearing address 59.
 * 3) in some rare case, the address 59 might not appear, that's why we want to loop all 128 addresses. 
 * */
void haptic_i2c_recovery(void)
{
    int rval = 0;
    u8  val;
    u8 i;
    int test_addr = 0;
    rval = i2c_set_bus_num(HAPTIC_I2C_BUS_NUM);
    if (rval != 0)
    {
        printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, HAPTIC_I2C_BUS_NUM);
    }
    
    printf("%s:", __FUNCTION__);
    for (i = 0; i < 128; i++)
        printf(i2c_probe(i) == 0 ? "%02X ": "", i) ;
    printf("\n");
    
    printf("%s:sleeping for 10ms\n", __FUNCTION__); 
    udelay(10*1000);
    
    val = HAPTIC_DEV_RST;
    rval = i2c_write(test_addr, HAPTIC_REG_CONTROL_2, 1, &val, 1);
    printf("%s:wrote to haptic recovery address 0x%x rval=%d\n", __FUNCTION__, test_addr, rval);
    
    printf("%s:sleeping for another 10ms\n", __FUNCTION__);    
    udelay(10*1000);
    
    printf("%s:", __FUNCTION__);
    for (i = 0; i < 128; i++)
        printf(i2c_probe(i) == 0 ? "%02X ": "", i) ;
    printf("\n");
    
}
int check_haptic_and_recovery(void)
{
    int haptic_err = 0;
    int count_down = HAPTIC_RECOVERY_ATTEMPTS;
    if(check_haptic()) {
        do {
            haptic_i2c_recovery();
            haptic_err = check_haptic();
        }while(haptic_err && count_down--);
        
        printf("%s:return=%d count_down=%d\n", __FUNCTION__, haptic_err, count_down);
        
        if(haptic_err && count_down <= 0) {
            printf("%s:haptic failed!\n", __FUNCTION__);
            return -1;	
        }
    } 
    return 0;
}

#define MX6_WARIO_I2C1_EN	 IMX_GPIO_NR(4, 17)


int inventory_post_test (int flags)
{
    const char *rev;
    int ret   = 0;

    /* Set i2c1 enable line gpio4_17 pull high*/
    mxc_iomux_v3_setup_pad(MX6SL_PAD_FEC_RXD0__GPIO_4_17);
    /* set as output high */
    gpio_direction_output(MX6_WARIO_I2C1_EN, 1);

    rev = (char *) get_board_id16();
    if ( BOARD_IS_WARIO(rev) || BOARD_IS_WOODY(rev) )
    {

        ret |= check_max77696_uic();
        ret |= check_max77696_fuelgauge();
        ret |= check_max77696_rtc();
        ret |= check_max77696_core();
        //ret |= check_usbmux();
    }
    else if ( BOARD_IS_PINOT(rev) || BOARD_IS_PINOT_WFO(rev)  
                || BOARD_IS_MUSCAT_WAN(rev) 
                || BOARD_IS_MUSCAT_WFO(rev)
                || BOARD_IS_BOURBON(rev) )
    {
        ret |= check_max77696_uic();
        ret |= check_max77696_fuelgauge();
        ret |= check_max77696_rtc();
        ret |= check_max77696_core();
    }
	else if ( BOARD_IS_WHISKY_WAN(rev) )
    {
        ret |= check_max77696_uic();
        ret |= check_max77696_fuelgauge();
        ret |= check_max77696_rtc();
        ret |= check_max77696_core();
	ret |= check_i2c_acc();
	ret |= check_i2c_prox();

    }
    else if ( BOARD_IS_WHISKY_WFO(rev) )
    {
        ret |= check_max77696_uic();
        ret |= check_max77696_fuelgauge();
        ret |= check_max77696_rtc();
        ret |= check_max77696_core();
	    ret |= check_i2c_acc();
    }
    else if ( BOARD_IS_ICEWINE_WARIO(rev) || BOARD_IS_ICEWINE_WFO_WARIO(rev) )
    {
        ret |= check_max77696_uic();
        ret |= check_max77696_fuelgauge();
        ret |= check_max77696_rtc();
        ret |= check_max77696_core();
        ret |= check_fsr_keypad();
        ret |= check_haptic_and_recovery(); 
    }
    else
    {
        printf("Device type not found!\n");
    }
    return ret;
}

/*************************************************************************************************
  The post mode self test for accelerometer HVT1.1 build
  Test steps:
  a. set +-8g full scale
  b. set to positive, then sleep 50ms
  c. get register value positive
  d. set to negative, then sleep 50ms
  e. get register value of negative
  f. compare the diff data between negative and positive
  g. softe reset accelerometer
  h. next axis
 
  Test sequence
  x axis, y axis, z axis
 
  Test limit(From WS-168, sensor engineer Micheal)
  x, y diff data >= 12, z diff data >= 6
**************************************************************************************************/
#define XY_MAX   12
#define XY_MIN  -12
#define Z_MAX    6
#define Z_MIN   -6

int soft_reset_acc(void)
{
    int ret;
    
    ret = board_i2c_write_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, ACC_REG_SW_RESET_OFFSET, ACC_SW_RESET_VALUE);
    if (ret == 1)
    {
        ret = I2C_PASS;
    }
    else if (ret == 0)
    {
        ret = I2C_FAIL;
        return ret;    
    }
}

int set_acc_PMU_range(void)
{
    int ret;

    ret = board_i2c_write_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, ACC_REG_PMU_RANGE_OFFSET, ACC_PMU_RANGE_VALUE);
    if (ret == 1)
    {
        ret = I2C_PASS;
    }
    else if (ret == 0)
    {
        ret = I2C_FAIL;
    }
    return ret; 
}

int acc_axis_self_test(unsigned int reg, const unsigned int reg_value1, const unsigned int reg_value2)
{
    int ret = I2C_PASS;
    int rval;
    unsigned int val;
    __s8 pos_value;
    __s8 neg_value;
    int diff_value;

    rval = soft_reset_acc();
    if (rval == I2C_FAIL) 
    {
        printf("ACC softe reset fail\n");
        ret = I2C_FAIL;
    }

    udelay(10*1000); 

    rval = set_acc_PMU_range();
    if (rval == I2C_FAIL) 
    {
        printf("ACC set PMU range fail\n");
        ret = I2C_FAIL;
    }
//read out positive value
    rval = board_i2c_write_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, ACC_REG_SELF_TEST_OFFSET, reg_value1);
    if (rval == 0) 
    {
        printf("write register fail\n");
        ret = I2C_FAIL;
    }
    udelay (50*1000);
    board_i2c_read_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, reg, &val);
    if (rval == 1) 
    {
        pos_value = val;
    }
    else if (rval == 0)
    {
        printf("read register fail\n");
        ret = I2C_FAIL;
    }
//read out negative value    
    rval = board_i2c_write_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, ACC_REG_SELF_TEST_OFFSET, reg_value2);
    if (rval == 0) 
    {
        printf("write register fail\n");
        ret = I2C_FAIL;
    }
    udelay (50*1000);
    board_i2c_read_reg(I2C_DEV_ACC_PROX, ACCELEROMETER_I2C_ADDR, reg, &val);
    if (rval == 1) 
    {
        neg_value = val;
    }
    else if (rval == 0)
    {
        printf("read register fail\n");
        ret = I2C_FAIL;
    }

    diff_value = pos_value - neg_value;
    printf ("diff is %d, pos value is %d, neg value is %d\n", diff_value, pos_value, neg_value);

    switch (reg) 
    {
    case ACC_REG_MSBX_OFFSET:
    case ACC_REG_MSBY_OFFSET:
        if ((diff_value < XY_MAX) && (diff_value > XY_MIN))
        {
            //printf ("diff is %d, value1 is %d, value2 is %d\n", diff_value, origin_value, test_value);
            ret = I2C_FAIL;
        }
        break;
    case ACC_REG_MSBZ_OFFSET:
        if ((diff_value < Z_MAX) && (diff_value > Z_MIN))
        {
            //printf ("diff is %d, value1 is %d, value2 is %d\n", diff_value, origin_value, test_value);
            ret = I2C_FAIL;
        }
        break;
    default:
        break;
    }

    return ret;
}

int accelerometer_self_test(int flags)
{
    int ret = I2C_PASS;
    int rval;
    
    //Do i2c access check before test start
    //if fail, return directly, test fail    
    rval = check_i2c_acc();
    if (rval == I2C_FAIL) 
    {
        printf("ACC access fail\n");
        ret = I2C_FAIL;
        return ret;
    }

    //x axis
    rval = acc_axis_self_test(ACC_REG_MSBX_OFFSET, ACC_SELF_TEST_POSX_VALUE, ACC_SELF_TEST_NEGX_VALUE);
    if (rval == I2C_FAIL) 
    {
        printf("X axis FAIL\n");
        ret = I2C_FAIL;
    }

    //y axis
    rval = acc_axis_self_test(ACC_REG_MSBY_OFFSET, ACC_SELF_TEST_POSY_VALUE, ACC_SELF_TEST_NEGY_VALUE);
    if (rval == I2C_FAIL) 
    {
        printf("Y axis FAIL\n");
        ret = I2C_FAIL;
    }

    //z axis
    rval = acc_axis_self_test(ACC_REG_MSBZ_OFFSET, ACC_SELF_TEST_POSZ_VALUE, ACC_SELF_TEST_NEGZ_VALUE);
    if (rval == I2C_FAIL) 
    {
        printf("Z axis FAIL\n");
        ret = I2C_FAIL;
    }
    
    printf ("Run accelerometer self test done\n");
    return ret;
}

#endif /* CONFIG_POST */
