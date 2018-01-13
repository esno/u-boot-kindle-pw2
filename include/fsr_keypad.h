#ifndef __FSR_KEYPAD_H_
#define __FSR_KEYPAD_H_

#define FSR_I2C_BUS_NUM      2
#define FSR_I2C_ADDR         0x58

#define FSR_SUCCESS          0
#define FSR_ERR_I2C          -1
#define FSR_ERR_TEST         -2
#define FSR_ERR_NO_TEST      -3

/**
 * Boot loader
 */
#define FSR_CMD_HOOK            0x02
#define FSR_CMD_QUIT            0x51
#define FSR_CMD_ADC             0x60
#define FSR_CMD_ACK             0xFC

/**
 * App mode
 */
#define FSR_SWITCH_MODE_REG 0x00

#define MAX_BUFFER_READ_LEN     16

/**
 * Normal command, just read one byte to check
 */
#define FSR_HOOK_CMD_LEN        2
#define FSR_ADC_READ_CMD_LEN    10
#define FSR_CALIB_CMD_LEN       2

#define FSR_CMD_ACK_FLAG        0x80

/**
 * Definition from kernel fsr_keypad.h
 */

#define ACTIVE_MODE_NORMAL      0x00
#define ACTIVE_MODE_CONFIG      0x01
#define ACTIVE_MODE_DIAGS       0x02
/**
 * List of registers
 */
#define REG_ACTIVE_MODE	        0x00

// Registers in normal mode
#define REG_N_POWER         0x01
#define REG_N_VERSION_MAJOR 0x02
#define REG_N_VERSION_MINOR 0x03
#define REG_N_NUM_BUTTONS   0x04
#define REG_N_BUTTON_STATE  0x05
#define REG_N_BUTTON_ID     0x06
#define REG_N_CAL_VALID     0x07
//Registers in diags mode
/**
        The commands should match with the FSRModeDiags offset in
        FSR firmaware: whisper_touch/Sources/fsr/fsr.h
        Make sure everytime the FSR firmware structure is changed:
        linux: drivers/input/keyboard/fsr_keypad.h
        and
        uboot: opensource/uboot/dist/include/fsr_keypad.h
        should also changed accordingly
typedef struct __attribute__((packed))
{
       uint8_t reserved1;                               // 0x00
       uint8_t adc_values;                                    // 0x01
       uint8_t adc_depth;                                     // 0x02
       volatile uint8_t dataready_flag;         // 0x03
       volatile uint16_t adc[FSR_NUM_CHANNELS];// 0x04
       uint8_t reserved[24];                                  // 0x10
       uint8_t num_cal_weights;                        // 0x28
       uint8_t num_adc_per_weight;                     // 0x29
       uint16_t cal_weights[MAX_NUM_CAL_WEIGHTS]; // 0x2A
       uint16_t cal_data[NUM_BUTTONS][MAX_NUM_CAL_WEIGHTS][NUM_ADC_PER_WEIGHT]; // 0x42
       uint8_t padding[2];                     // 0xA2
} FSRModeDiags;

 */

//Registers in diags mode
#define REG_D_NUM_ADC_VALS     0x01
#define REG_D_ADC_DEPTH        0x02
#define REG_D_READY            0x03
#define REG_D_ADC_VALS         0x04
#define REG_D_NUM_CAL_WEIGHTS  0x28
#define REG_D_ADC_PER_WEIGHT   0x29
#define REG_D_CAL_WEIGHTS      0x2A
#define REG_D_CAL_DATA         0x42

/**
 * End definition from kernel fsr_keypad.h
 */
/**
 * Definition from whisper_touch/Sources/fsr/fsr.h
 */
#define PWR_SUSPEND 0x01
#define PWR_IDLE    0x02

#define FLAG_ADC_READY_FOR_READ  0xAA
#define FLAG_HOST_DONE_ADC_READ  0x55

struct fsr_simple_cmd
{
    u32    len;
    u8     cmd;
    u8     check_sum; 
};

#define FSR_SIMPLE_CMD_LEN      6

/**
 * Any mode 
 *  
 */
extern int fsr_hw_reset     (void *p_void);

/**
 * Bootloader mode
 * 
 */
extern int fsr_bl_quit      (void *p_void);
extern int fsr_bl_hook      (void *p_void);

/**
 * App mode
 */
extern int fsr_app_suspend  (void *p_void);
extern int fsr_app_adc      (void *p_void);
extern int fsr_app_ver      (void *p_void);
extern int fsr_test_all     (void *p_void);

#endif // __FSR_KEYPAD_H_ 

