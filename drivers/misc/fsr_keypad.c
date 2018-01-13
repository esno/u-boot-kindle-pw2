#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/gpio.h>
#include <asm/arch/board-mx6sl_wario.h>

#include "lab126_mxc_i2c.h"

#define __FSR_DEBUG

#include <fsr_keypad.h>
int fsr_keypad_get_adc(u16 *adc, int num_adc);
static int fsr_switch_mode(u8 mode);
static int app_mode;

/**
 * Boot loader section
 */

static u8 check_sum(u8 *str, int len)
{
    int chksum = 0;
    int i;
    for (i = 0; i < len - 1; i++)
    {
        chksum += *str++;
    }
    return (u8) chksum;
}

static void fsr_bl_print_cmd(struct fsr_simple_cmd *fsr_cmd)
{
    int i;
    u8 *str = (u8 *) fsr_cmd;
    for (i = 0; i < FSR_SIMPLE_CMD_LEN; i++)
    {
        printf("0x%02x ", *str++);
    }
    printf("\n");
    printf("Cmd: 0x%02x, len: 0x%02x, chksum: 0x%02x\n", fsr_cmd->cmd, fsr_cmd->len, fsr_cmd->check_sum);
}

static int fsr_bl_send_cmd(u8 cmd)
{
    int rval;
    struct fsr_simple_cmd fsr_cmd;
    
    fsr_cmd.cmd         = cmd;
    fsr_cmd.len         = FSR_SIMPLE_CMD_LEN;
    fsr_cmd.check_sum   = check_sum((u8 *) &fsr_cmd, fsr_cmd.len);

    fsr_bl_print_cmd(&fsr_cmd);
    rval = i2c_set_bus_num(FSR_I2C_BUS_NUM);
    if (rval != 0) {
        printf("%s Cannot set i2c bus: %d, rval: %d\n", __FUNCTION__, FSR_I2C_BUS_NUM, rval);
        return FSR_ERR_I2C;
    }
    rval = i2c_write(FSR_I2C_ADDR, 0, 0, (u8 *) &fsr_cmd, FSR_SIMPLE_CMD_LEN);
    if (rval != 0) {
        printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    return FSR_SUCCESS;
}

int fsr_bl_verify_cmd(char *cmd_str, u8 cmd, int len)
{
    int i;
    int rval;
    int retry = 5;
    u8 buffer[MAX_BUFFER_READ_LEN];

    if ((cmd_str == NULL) || (len > MAX_BUFFER_READ_LEN))
    {
        return -1;
    }
    do
    {
        rval = i2c_raw_read(FSR_I2C_ADDR, buffer, len);
        if (rval == 0) {
            printf("verifying 0x%02x\n", buffer[0]);
            for (i = 0; i < len; i++) {
                printf("0x%02x ", buffer[i]);
            }
            printf("\n");
            if (buffer[0] == (FSR_CMD_ACK_FLAG | cmd)) {
                printf("Cmd %s success\n", cmd_str);
                return FSR_SUCCESS ;
            }
        }
        udelay(10);
    } while (retry-- > 0);

    printf("Cmd %s failed\n", cmd_str);
    return FSR_ERR_TEST;
}

int fsr_hw_reset(void *p_void)
{
    gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_9, 0);
    printf("%s gpio_direction_output(0)\n", __func__);
    udelay(1000);
    printf("%s gpio_direction_output(1)\n", __func__);
    gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_9, 1);
    
    app_mode = 0;
    
    return 0;
}

/**
 * @brief fsr_bl_quit() 
 * Just make sure it is in bootloader mode before going to app 
 * mode. Save vni station the reboot/hw_reset step after 
 * programming the MCU firmware
 * 
 * @param p_void 
 * 
 * @return int 
 */
int fsr_bl_quit(void *p_void)
{
	int ret;
	
	if(app_mode) {
		printf("firmware is in app mode, run fsr hw_reset before running this command!!\n");
		return 0;
	}
	
	ret = fsr_bl_send_cmd(FSR_CMD_QUIT);
	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
	app_mode = 1;
	return ret;
}

int fsr_bl_hook(void *p_void)
{
    int rval;
	
	if(app_mode) {
		printf("firmware is in app mode, run fsr hw_reset before running this command!!\n");
		return 0;
	}
	
	rval = fsr_bl_send_cmd(FSR_CMD_HOOK);
	if (rval == 0) {
		rval = fsr_bl_verify_cmd("hook", FSR_CMD_HOOK, FSR_HOOK_CMD_LEN);
	}
	
    return rval;
}

/**
 * App section
 *  
 */
 
int fsr_app_ver(void *p)
{
	int rval;
	u8 reg[2];
	int i;
	
	if(!app_mode) {
		printf("firmware is in bl mode, run fsr quit before running this command!!\n");\
		return 0;
	}

	rval = fsr_switch_mode(ACTIVE_MODE_NORMAL);
	if (rval != FSR_SUCCESS) {
		printf("%s Cannot switch mode: ACTIVE_MODE_NORMAL, rval: %d\n", __FUNCTION__, rval);
		return FSR_ERR_I2C;
	}

	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 0);
	udelay(15);	
	rval = i2c_write(FSR_I2C_ADDR, REG_N_VERSION_MAJOR, 1, NULL, 0);
	if (rval != 0) {
		printf("%s i2c write failed, rval %d %d\n", __FUNCTION__, rval, __LINE__);
		return FSR_ERR_I2C;
	}
	rval = i2c_raw_read(FSR_I2C_ADDR, reg, 2);
	if (rval != 0) {
		printf("%s i2c write failed, rval %d %d\n", __FUNCTION__, rval, __LINE__);
		return FSR_ERR_I2C;
	}
	printf("fsr app version: 0x%02X%02X\n", reg[0], reg[1]);
	
	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
}

int fsr_app_suspend(void *p_void)
{
    int rval;
    u8 reg[1];
    
	if(!app_mode) {
		printf("kl05 in bl mode, run fsr quit before running this command!!\n");\
		return 0;
	}
    
    rval = fsr_switch_mode(ACTIVE_MODE_NORMAL);
    if (rval != FSR_SUCCESS) {
        printf("%s Cannot switch mode: ACTIVE_MODE_NORMAL, rval: %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    
	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 0);
	udelay(15);	
    reg[0] = PWR_SUSPEND;
    rval = i2c_write(FSR_I2C_ADDR, REG_N_POWER, 1, reg, 1);
    if (rval != 0) {
        printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
    
    printf("\n----------fsr suspended-------\n\n");
    
    return FSR_SUCCESS;
}

static int fsr_switch_mode(u8 mode)
{
    u8 reg[2];
    int rval;
   
	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 0);
	udelay(15);
	
    rval = i2c_set_bus_num(FSR_I2C_BUS_NUM);
    if (rval != 0) {
        printf("%s Cannot set i2c bus: %d, rval: %d\n", __FUNCTION__, FSR_I2C_BUS_NUM, rval);
        return FSR_ERR_I2C;
    }

    reg[0] = mode;
    reg[1] = 0;
    rval = i2c_write(FSR_I2C_ADDR, REG_ACTIVE_MODE, 1, (u8 *) reg, 1);
    if (rval != 0) {
        printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
    udelay(1000); //delay 1 ms for MCU to sleep
    
    return FSR_SUCCESS;
}

/**
 * @brief fsr_keypad_get_single_adc() 
 * Get the adc value stored in  
 * 
 * @param adc 
 * @param num_adc 
 * 
 * @return int 
 */
int fsr_keypad_get_single_adc(u16 *adc, u32 num_adc)
{
    int rval;
    u8 ready[4];
    int retry = 100;

	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 0);
	udelay(15);

    do {
        ready[0] = 0;
        ready[1] = 0;
		
        rval = i2c_write(FSR_I2C_ADDR, REG_D_READY, 1, NULL, 0);
        if (rval != 0) {
            printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
            return FSR_ERR_I2C;
        }
        rval = i2c_raw_read(FSR_I2C_ADDR, ready, 1);
        if (rval != 0) {
            printf("%s i2c read failed, rval %d\n", __FUNCTION__, rval);
            return FSR_ERR_I2C;
        }
#ifdef FSR_DEBUG
        printf("%s i2c read, ready 0x%02X rval %d\n", __FUNCTION__, 
               (unsigned int) ready[0], rval);
#endif // FSR_DEBUG
        if (ready[0] == FLAG_ADC_READY_FOR_READ) {
            break;
        }
        udelay(10000);
    }while(--retry);
    
    
    if (retry == 0) {
        printf("%s Failed retry %d\n", __FUNCTION__, retry);
        return FSR_ERR_I2C;
    }
#ifdef FSR_DEBUG
    printf("%s i2c read, ready 0x%02X rval %d\n", __FUNCTION__, 
           (unsigned int) ready[0], rval);
#endif // FSR_DEBUG
    
    rval = i2c_write(FSR_I2C_ADDR, REG_D_ADC_VALS, 1, NULL, 0);
    if (rval != 0) {
        printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    rval = i2c_raw_read(FSR_I2C_ADDR, (u8 *) adc, num_adc * sizeof(u16));
#ifdef FSR_DEBUG
    printf("%s i2c read, rval %d\n", __FUNCTION__, rval);
#endif // FSR_DEBUG

    ready[0] = FLAG_HOST_DONE_ADC_READ;
    rval = i2c_write(FSR_I2C_ADDR, REG_D_READY, 1, ready, 1);
    if (rval != 0) {
        printf("%s i2c write failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    
    gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
	
    return rval;
}

int fsr_keypad_get_adc(u16 *adc, int num_adc)
{
    int rval;
    int retry = 2;

    rval = fsr_switch_mode(ACTIVE_MODE_DIAGS);
    if (rval != 0) {
        printf("%s fsr_switch_mode failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }

    /**
     * Need to read adc twice to flush the invalid adc value 
     * captured before 
     */
    do {
        rval = fsr_keypad_get_single_adc(adc, num_adc);
    } while ((rval != FSR_SUCCESS) && (--retry > 0));

    if (rval != FSR_SUCCESS) {
        printf("%s Failed retry %d\n", __FUNCTION__, retry);
        return rval;
    }
    
    rval = fsr_switch_mode(ACTIVE_MODE_NORMAL);
    if (rval != 0) {
        printf("%s fsr_switch_mode failed, rval %d\n", __FUNCTION__, rval);
        return FSR_ERR_I2C;
    }
    return FSR_SUCCESS;
}

int fsr_app_adc(void *p_void)
{
    int rval;
    u16 adc[6] = 
    {
        0xDEAF,
        0xBEEF,
        0xBAAD,
        0xBEEF,
        0xDEAD,
        0xBEEF
    };
    
	if(!app_mode) {
		printf("kl05 in bl mode, run fsr quit before running this command!!\n");\
		return 0;
	}

    rval = fsr_keypad_get_adc(adc, 6);
    if (rval == FSR_SUCCESS) {
        printf("adc 0x%04X 0x%04X 0x%04X 0x%04x, 0x%04X 0x%04x\n", 
               adc[0], adc[1], adc[2], adc[3], adc[4], adc[5]);
    }
    else
    {
        printf("%s, fsr_keypad_get_adc failed\n", __func__);
    }

    return FSR_SUCCESS;
}

int fsr_test_all(void *p)
{
	int rval;
	int i;
	
	for (i = 1; i <= 100; i++)
	{
		printf("#%d iteration\n", i);
		rval = fsr_hw_reset(NULL);
		if(rval) break;
		
		rval = fsr_bl_hook(NULL);
		if(rval) break;
		
		rval = fsr_bl_quit(NULL);
		if(rval) break;
		
		udelay(10000);
		rval = fsr_app_adc(NULL);
		if(rval) break;
		
		udelay(1000);
		rval = fsr_app_ver(NULL);
		if(rval) break;
		
		udelay(1000);
		rval = fsr_app_suspend(NULL);
		if(rval) break;
	}
	
	gpio_direction_output(MX6SL_ARM2_EPDC_SDDO_13, 1);
}


