#include <common.h>
#include <command.h>
#include <environment.h>
#include <diag_struct.h>
#include <boardid.h>

extern int test_entry(unsigned long action_id);

int do_low_power(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    unsigned long test_id;
    const char * rev = (const char *) get_board_id16();
    if (argc < 2) {
	cmd_usage(cmdtp);    
        return 1;
    }
    test_id = simple_strtoul(argv[1], NULL, 10);

#ifdef CONFIG_LAB126_LPM_TEST
    switch(test_id)
    {
        case TEST_ID_IDLE_MODE:
             printf("Entering Idle mode...\n");
             break;
        case TEST_ID_SUSPEND_MODE:
             printf("Entering Suspend mode...\n");
             break;
        case TEST_ID_SUSPEND_MODE_MMC_POWER_GATE:
             if ( BOARD_IS_MUSCAT_WAN(rev) || BOARD_IS_MUSCAT_WFO(rev) ||
                  BOARD_IS_WHISKY_WAN(rev) || BOARD_IS_WHISKY_WFO(rev) ||
                  BOARD_IS_WOODY(rev) )	
                  printf("Entering Suspend mode with mmc power gate...\n");
             else {
           	      printf("Device does not support emmc power gate!");
                  cmd_usage(cmdtp);
                  return 0;
             }
             break;
        case TEST_ID_SHIPPING_MODE:
             printf("Entering Shipping mode...\n");
             break;
        case TEST_ID_HALT_MODE:
             printf("Entering Halt (FullShutdown) mode...\n");
             break;
        default:
            cmd_usage(cmdtp);
            return 0;
    }

    test_entry(test_id);
#endif    

    return 0;
}

U_BOOT_CMD(
	lpm, 2, 1, do_low_power,
	"Exercise Low Power modes in iMX6SL",
	"lpm <test number> : 1=Enter Idle mode(ARM WFI), 2=Enter Suspend mode, 22=Enter suspend mode with emmc power gate, 3=Enter Ship mode\n");

