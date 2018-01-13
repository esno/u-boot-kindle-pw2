#ifndef __DRV2667_H_
#define __DRV2667_H_
#define HAPTIC_I2C_BUS_NUM      2
/*
 * Address: 0x59
 *      Read: 0xB3
 *      Write 0xB2
 */
#define HAPTIC_I2C_ADDR         0x59

#define HAPTIC_CHIP_ID          0x07

#define HAPTIC_SUCCESS          0
#define HAPTIC_ERR_I2C          -1
#define HAPTIC_ERR_TEST         -2
#define HAPTIC_ERR_NO_TEST      -3
/* should match with HAPTIC_IQC_TEST_STR in imx60_wario.c */
#define HAPTIC_IQC_TEST_STR             "haptic_iqc"
#define HAPTIC_CMD_LINE_STR             "iqc"

#define HAPTIC_AMPLITUDE_FULL           0xFF
#define HAPTIC_AMPLITUDE_HALF           0x7F

#define HAPTIC_GO                       0x01
#define HAPTIC_DEV_RST                  0x80
#define HAPTIC_STANDBY                  0x40

#define HAPTIC_MAX_WAVEFORM             2

#define HAPTIC_ENV_NO_ENV               0x00
#define HAPTIC_ENV_32_MS                0x11
#define HAPTIC_ENV_64_MS                0x22

#define HAPTIC_FREQ_200_HZ              0x19
#define HAPTIC_FREQ_400_HZ              0x34

#define HAPTIC_CYCLES                   0x15

/* IQC config 1, 90 VPP, for AAC actuator */
#define HAPTIC_IQC_AMPLITUDE_90_VPP 0xDA
#define HAPTIC_IQC_CFG1_GAIN            0x01    

/* IQC config 2, 120 VPP for Semco */
#define HAPTIC_IQC_AMPLITUDE_120_VPP    0xc7
#define HAPTIC_IQC_CFG2_GAIN            0x2

/* 
 * When writing single byte to address 0xFF, it goes to the PAGE CONTROL register. 
 * When writing multiple bytes to address 0xFF, it goes to the RAM 
 */
#define HAPTIC_PROGRAMMING_SPACE        0x01
#define HAPTIC_CONTROL_SPACE            0x00

/*
 * Control space
 */
enum
{
    HAPTIC_REG_STATUS                       ,
    HAPTIC_REG_CONTROL                      ,
    HAPTIC_REG_CONTROL_2                    ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_03        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_04        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_05        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_06        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_07        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_08        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_09        ,
    HAPTIC_REG_WAVEFORM_SEQUENCER_0A        ,
    HAPTIC_REG_FIFO                         ,
    HAPTIC_REG_PAGE = 0xFF
};

/*
 * Data space
 * First byte is the header size,
 * next 5 bytes are for header. 
 * The rest up to 2048 bytes are for waveform data
 */
enum
{
    HAPTIC_REG_RAM_HEADER_SIZE              ,
    HAPTIC_REG_RAM_START_ADDR_MSB           ,
    HAPTIC_REG_RAM_START_ADDR_LSB           ,
    HAPTIC_REG_RAM_STOP_ADDR_MSB            ,
    HAPTIC_REG_RAM_STOP_ADDR_LSB            ,
    HAPTIC_REG_RAM_REPEAT_COUNT             ,
/*
 * Mode 3
 * Waveform synthesis 
 */
    HAPTIC_REG_WS_AMPLITUDE                 ,
    HAPTIC_REG_WS_FREQUENCY                 ,
    HAPTIC_REG_WS_CYCLES                    ,
    HAPTIC_REG_WS_ENVELOPE
};

typedef struct _HAPTIC_HEADER
{
    uchar header_size       ;
    uchar start_addr_msb    ;
    uchar start_addr_lsb    ;
    uchar stop_addr_msb     ;
    uchar stop_addr_lsb     ;
} HAPTIC_HEADER;

typedef struct _HAPTIC_WAVEFORM_SYNTHESIS
{
    uchar amplitude         ; /* Peak = amplitude / 255 * Full Scale Peak Voltage */
    uchar frequency         ; /* Sinosoid Freq(Hz) = 7.8125 * frequency */
    uchar cycles            ; /* Duration (ms) = 1000 * cycles / (7.8125 * frequency */
    uchar envelope          ; /* Ramp up = envelope[7:4], Ramp down = envelope[3:0] */
    /*
     * Nibble value from 0 - 15 to (ms):
     * no envelope, 32, 64, 96, 192, 224, 256, 512, 768, 1024, 1280, 1536, 1792, 2048
     */
    uchar gain              ; /* gain */
    uchar repeat_count      ;
} HAPTIC_WAVEFORM_SYNTHESIS;

static HAPTIC_HEADER  haptic_header =
{
    0x05,
    0x80, /* start address upper byte, also indicate Mode 3 */
    0x06, /* start address lower byte, default to first waveform at 0x06 */
    0x00, /* stop address upper byte */
    0x09  /* stop address lower byte, default to first waveform at 0x06 + 3 */
};

enum {
    HAPTIC_ID_AUTO          ,
    HAPTIC_ID_RESET         ,
    HAPTIC_ID_UNKNOWN
};

static HAPTIC_WAVEFORM_SYNTHESIS waveforms[HAPTIC_MAX_WAVEFORM] =
{
    {
        /* IQC config 1, 90 VPP */
        HAPTIC_IQC_AMPLITUDE_90_VPP,    /* amplitude, 90 VPP */
        HAPTIC_FREQ_200_HZ,     /* frequency 200 Hz */
        HAPTIC_CYCLES,          /* duration 20 cycles */
        HAPTIC_ENV_NO_ENV,      /* No envelope */
        HAPTIC_IQC_CFG1_GAIN,
        0               /* repeat infinite */
    },
    {
        /* IQC config 2, 120 VPP */
        HAPTIC_IQC_AMPLITUDE_120_VPP,   /* amplitude 120 VPP */
        HAPTIC_FREQ_200_HZ,     /* frequency 200 Hz */
        HAPTIC_CYCLES,          /* duration 20 cycles */
        HAPTIC_ENV_NO_ENV,      /* no envelope */
        HAPTIC_IQC_CFG2_GAIN,
        0               /* repeat infinite */
    }

};

extern int haptic_reset(void *p_void);
extern int haptic_set_waveform(HAPTIC_WAVEFORM_SYNTHESIS *p_waveform);

#endif // __DRV2667_H_
