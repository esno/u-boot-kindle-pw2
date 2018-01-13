#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include "lab126_mxc_i2c.h"

#include <asm/arch/iomux-v3.h>
#include <asm/arch/gpio.h>

#include <asm/arch/board-mx6sl_wario.h>

#include <drv2667.h>

int haptic_i2c_write(uint addr, uchar *buf, int len)
{
        int i;

        printf("%s: 0x%02x, ", __FUNCTION__, addr);
        for (i = 0; i < len; i++) {
                printf("0x%02x ", buf[i]);
        }
        printf("\n");
        return i2c_write(HAPTIC_I2C_ADDR, addr, 1, buf, len);
}

int haptic_i2c_write_byte(uint addr, uchar value)
{
        uchar buf = value;

        printf("%s: 0x%02x, 0x%02x\n", __FUNCTION__, addr, value);
        return i2c_write(HAPTIC_I2C_ADDR, addr, 1, &buf, 1);
}

int haptic_reset(void *p_void)
{
        int rval;

        /**
         * Reset the haptic chip
         */
        rval = haptic_i2c_write_byte(HAPTIC_REG_CONTROL_2, HAPTIC_DEV_RST | HAPTIC_STANDBY);
        if (rval != 0) {
                printf("%s haptic_i2c_write_byte(HAPTIC_REG_CONTROL_2, HAPTIC_DEV_RST | HAPTIC_STANDBY) failed, rval: %d\n", __func__, rval);
        }
        return rval;
}

int haptic_set_waveform(HAPTIC_WAVEFORM_SYNTHESIS *p_waveform)
{
        int rval;
        uchar val;
        uchar chip_id = 0;

        rval = i2c_set_bus_num(HAPTIC_I2C_BUS_NUM);
        if (rval != 0) {
                printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, HAPTIC_I2C_BUS_NUM);
                return HAPTIC_ERR_I2C;
        }
        rval = i2c_read(HAPTIC_I2C_ADDR, HAPTIC_REG_CONTROL, 1, &val, 1);
        if (rval == 0)  {
                chip_id = (val >> 3) & 0x0F;
                printf("%s: ID = 0x%02x\n", __FUNCTION__, (val >> 3) & 0x0F);
        } else {
                printf("%s: Cannot read haptic ID\n", __FUNCTION__);
        }

        /* Take device out of STANDBY mode */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_CONTROL_2, 0x00);
        /* Set gain, gain 0, 50 Vpp maximum */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_CONTROL, (p_waveform->gain & 0x3) | (chip_id << 3));
        /* Set sequencer to play waveform ID #1 */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_WAVEFORM_SEQUENCER_03, 0x01);
        /* Switch to programing space. Set memory to page 1  */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_PAGE, HAPTIC_PROGRAMMING_SPACE);
        /* write the header  */
        rval |= haptic_i2c_write(HAPTIC_REG_RAM_HEADER_SIZE, (uchar *) &haptic_header, sizeof(HAPTIC_HEADER));
        /* repeat count */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_RAM_REPEAT_COUNT, p_waveform->repeat_count);
        /* Write the selected waveform  */
        rval |= haptic_i2c_write(HAPTIC_REG_WS_AMPLITUDE, (uchar *) &p_waveform->amplitude, 4);
        /* Set page register to control space */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_PAGE, HAPTIC_CONTROL_SPACE);
        /* Set GO bit, execute waveform sequence */
        rval |= haptic_i2c_write_byte(HAPTIC_REG_CONTROL_2, HAPTIC_GO);

        return rval;
}

