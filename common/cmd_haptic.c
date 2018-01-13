/*
 * cmd_pmic.c  
 *
 * Copyright 2013 Amazon Technologies, Inc. All Rights Reserved.
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
#include "lab126_mxc_i2c.h"
#include "drv2667.h"

#define HAPTIC_CMD_STR_RESET    "reset"
#define HAPTIC_CMD_STR_AUTO     "auto"

#define HAPTIC_CMD_TEST_ERR     -1
#define HAPTIC_NO_TEST          -2

typedef int (*haptic_cb_func)(void *p_void);

typedef struct _HAPTIC_CMD
{
    const char      *cmd;
    haptic_cb_func  func;
    int             waveform_id;
    const char      *desc;
} HAPTIC_CMD;

/**
 * Local functions
 */
static int haptic_test_waveform    (void *p_void);
static int haptic_test_reset       (void *p_void);
static int haptic_test_auto_detect (void *p_void);
static int haptic_usage(void);

HAPTIC_CMD haptic_cmd_arr[] =
{
    { HAPTIC_CMD_STR_RESET, haptic_reset           , HAPTIC_ID_RESET     , "reset"                   },
    { HAPTIC_CMD_STR_AUTO , haptic_test_auto_detect, HAPTIC_ID_AUTO      , "auto detect"             },

    { NULL, NULL, 0, NULL}
};

/**
 * @brief haptic_usage() 
 * Print usage for all commands from the table and manual 
 * waveform 
 * 
 * @return int 
 */
int haptic_usage(void)
{
    HAPTIC_CMD *p_haptic_cmd = haptic_cmd_arr;

    printf("HAPTIC  - HAPTIC test commands\n");

    while (p_haptic_cmd && p_haptic_cmd->cmd != NULL) {
        printf("haptic %s [%s]\n", p_haptic_cmd->cmd, p_haptic_cmd->desc );
        p_haptic_cmd++;
    }
    printf("haptic wr amplitude gain frequency cycles envelope repeat_count\n");
    return 0;
}

/**
 * @brief haptic_test_waveform() 
 * Play selected waveform 
 * 
 * @param p_void 
 * 
 * @return int 
 */
int haptic_test_waveform(void *p_void)
{
    if (p_void) {
        HAPTIC_CMD *p_haptic_cmd = ( HAPTIC_CMD *) p_void; 
        return haptic_set_waveform(&waveforms[p_haptic_cmd->waveform_id]);    
    }
    return -1;
}

/**
 * @brief haptic_test_auto_detect() 
 * Auto detect the actuator type and run the appropriate 
 * waveform 
 * 
 * @param p_void 
 * 
 * @return int 
 */
int haptic_test_auto_detect(void *p_void)
{
    /* Only support AAC now */
    return haptic_set_waveform(&waveforms[0]);    
}

/**
 * @brief to_hex() 
 * Take a string in hex format (without 0x prefix, or h suffix, 
 * and convert to integer format 
 * 
 * @param str 
 * @param p_val 
 * 
 * @return int 
 */
int to_hex(char *str, uchar *p_val)
{
    uchar val  = 0;
    uchar ch = (uchar) *str++;
    if ((ch >= '0') && (ch <= '9')) {
        val = ch - '0';
    } else {
        ch |= 0x20; /* to lower case */
        if ((ch < 'a') && (ch > 'f')) {
            return -1;
        }
        val = ch - 'a' + 10;
    }
    val = val << 4;
    ch = (uchar) *str;
    if ((ch >= '0') && (ch <= '9')) {
        val += ch - '0';
    } else {
        ch |= 0x20; /* to lower case */
        if ((ch < 'a') && (ch > 'f')) {
            return -1;
        }
        val += ch - 'a' + 10;
    }
    *p_val = val;
    return 0;
}

/**
 * @brief haptic_cmd() 
 *  
 * Generate waveform with parameters from command line 
 * 
 * @param gain 
 * @param amplitude 
 * @param frequency 
 * @param cycles 
 * @param envelope 
 * @param repeat_count 
 * 
 * @return int 
 */
int haptic_cmd(char *gain, char *amplitude, char *frequency, char *cycles, char *envelope, char *repeat_count)
{
    int val;
    HAPTIC_WAVEFORM_SYNTHESIS waveform;

    if (gain == NULL || amplitude == NULL || frequency == NULL || cycles == NULL || repeat_count == NULL) {
        printf("usage: haptic wr gain amplitude frequency cycles envelope repeat_count\n");
        return HAPTIC_ERR_NO_TEST;
    }
    val  = to_hex(gain      , &waveform.gain);
    val |= to_hex(amplitude , &waveform.amplitude);
    val |= to_hex(frequency , &waveform.frequency);
    val |= to_hex(cycles    , &waveform.cycles);
    val |= to_hex(envelope  , &waveform.envelope);
    val |= to_hex(repeat_count, &waveform.repeat_count);
    if (val != 0) {
        printf("usage: haptic wr gain amplitude frequency cycles envelope repeat_count\n");
        return HAPTIC_ERR_NO_TEST;
    }
    return haptic_set_waveform(&waveform);
}

/**
 * @brief do_haptic
 * Main test command entry 
 *  
 * @param cmdtp 
 * @param flag 
 * @param argc 
 * @param argv 
 * 
 * @return int 
 */
int do_haptic (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int res = 1;  /* default is failure */
    char *env = getenv("haptic_test");
    HAPTIC_CMD *p_haptic_cmd = haptic_cmd_arr;

    printf("%s\n", __FUNCTION__);
    
    /**
     * Run if the command is listed from the array.
     */
    while (p_haptic_cmd && p_haptic_cmd->cmd != NULL) {
        if (strncmp(argv[1], p_haptic_cmd->cmd, strlen(p_haptic_cmd->cmd)) == 0) {
            res = p_haptic_cmd->func(p_haptic_cmd);
            printf("haptic test: %s [%s], result: %s, err code: %d\n", 
                p_haptic_cmd->desc ,
                p_haptic_cmd->cmd  ,
                res == 0 ? "pass" : "fail",
                res);
            return res;
        }
        p_haptic_cmd++;
    }

    /** 
     *  Manual set the haptic waveform synthesis parameters if the
     *  command is not from the array
     */
    if (strncmp(argv[1], "wr", 2) == 0) {
        res = haptic_cmd(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
        return res;
    }

    /* Check command from bootmode */
    if (env)
    {
        printf("%s: env: %s\n", __FUNCTION__, env);
	res = haptic_test_auto_detect(NULL);
        return res;
    }
    haptic_usage();
    return HAPTIC_ERR_NO_TEST;
}

/***************************************************/

U_BOOT_CMD(
    haptic, 8,  1,  do_haptic,
    "haptic  - HAPTIC test commands\n",
    "cmd from bootmode, auto run\n"
    "bootmode  haptic_iqc  - auto detect\n"
    "cmd from bist command line\n"
        "haptic  auto - auto detect\n"
        "haptic  reset\n"
        "haptic wr amplitude gain frequency cycles envelope repeat_count\n"
);

