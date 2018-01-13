/*
 * cmd_fsr.c  
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
#include "fsr_keypad.h"

#define FSR_CMD_STR_QUIT        "quit"
#define FSR_CMD_STR_SUSPEND     "suspend"
#define FSR_CMD_STR_ADC         "adc"
#define FSR_CMD_STR_HOOK        "hook"
#define FSR_CMD_STR_HW_RESET    "hw_reset"
#define FSR_CMD_STR_VER    "ver"
#define FSR_CMD_STR_TEST   "test"
#define FSR_NO_TEST     -2

typedef int (*bist_cb_func)(void *p_void);

typedef struct _BIST_CMD
{
    const char      *name;
    const char      *cmd;
    bist_cb_func    func;
    const char      *desc;
} BIST_CMD;


BIST_CMD bist_cmd[] =
{
	{ "test"    , FSR_CMD_STR_TEST      , fsr_test_all      , "[bootloader, app] test"      },
	{ "ver"     , FSR_CMD_STR_VER       , fsr_app_ver       , "[app] ver"                   },
    { "quit"    , FSR_CMD_STR_QUIT      , fsr_bl_quit       , "[bootloader] quit"           },
    { "adc"     , FSR_CMD_STR_ADC       , fsr_app_adc       , "[app] adc"                   },
    { "hook"    , FSR_CMD_STR_HOOK      , fsr_bl_hook       , "[bootloader] hook"           },
    { "suspend" , FSR_CMD_STR_SUSPEND   , fsr_app_suspend   , "[app] suspend, house keeping"},
    { "hw_reset", FSR_CMD_STR_HW_RESET  , fsr_hw_reset      , "[bootloader, app] hw_reset"  },
    { NULL, NULL, NULL, NULL}
};

int fsr_usage(void)
{
    BIST_CMD *p_bist_cmd = bist_cmd;

    printf("fsr  - FSR test commands\n");

    while (p_bist_cmd && p_bist_cmd->name != NULL) {
        printf("fsr %s     -- %s\n", p_bist_cmd->cmd, p_bist_cmd->desc);
        p_bist_cmd++;
    }
    return 0;
}

int do_fsr (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int res = 1;  /* default is failure */
    BIST_CMD *p_bist_cmd = bist_cmd;

    while (p_bist_cmd && p_bist_cmd->name != NULL) {
        if (strncmp(argv[1], p_bist_cmd->cmd, strlen(p_bist_cmd->cmd)) == 0) {
            res = p_bist_cmd->func(NULL);
            printf("fsr test: %s [%s], result: %s, err code: %d\n", 
                p_bist_cmd->desc ,
                p_bist_cmd->cmd  ,
                res == 0 ? "pass" : "fail",
                res);
            return res;
        }
        p_bist_cmd++;
    }
    fsr_usage();
    return FSR_NO_TEST;
}

/***************************************************/

U_BOOT_CMD(
    fsr,    8,  1,  do_fsr,
    "fsr  - FSR test commands\n",
    "fsr adc\n"
    "fsr hook\n"
    "fsr quit\n"
    "fsr suspend\n"
    "fsr ver"
);

