// Nordic modules
#include <zephyr/zephyr.h>
#include <stdlib.h> 
#include <zephyr/shell/shell.h>

// Citysage modules
#include "lte_connect_mgr.h"    // need LTE connection mgr to display/clear stats
#include "bsp/modem.h"       // need modem to fetch important  debug info

/** 
* @brief    Function to display the LTE connection statistics
*
* @param    shell variable length parameter list
*
* @return   err
*
* @note      
*/
static int app_lte_display(const struct shell *shell, size_t argc,
                char *argv[])
{
    lte_stats_print();
    return 0;
}

/** 
* @brief    Function to clear the LTE connection statistics  
*
* @param    shell variable length parameter list
*
* @return   err
*
* @note      
*/
static int app_lte_clear(const struct shell *shell, size_t argc, char *argv[])
{
    lte_stats_clear();
    printk("LTE network statistics cleared\n");
    return 0;
}

/** 
* @brief    Function to clear the LTE connection statistics  
*
* @param    shell variable length parameter list
*
* @return   err
*
* @note      
*/

void app_shell_init(void)
{
SHELL_STATIC_SUBCMD_SET_CREATE(    
        lte_display_statistics_cmds,
        SHELL_CMD_ARG(display, NULL,
            "displays modem statistics & state\n"
            "usage: lte display\n",
            app_lte_display, 1, 0),

        SHELL_CMD_ARG(clear, NULL,
            "clears LTE connection statistics\n"
            "usage: lte clear\n",
            app_lte_clear, 1, 0),
        
        SHELL_SUBCMD_SET_END
        );
    SHELL_CMD_REGISTER(lte, &lte_display_statistics_cmds, "Shows & clears LTE connection statistics", NULL);
}
