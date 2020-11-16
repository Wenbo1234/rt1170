/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "fsl_common.h"
#include "fsl_shell.h"
#include "shell_color.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "logging.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static shell_status_t loglevel(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t logbit(shell_handle_t context, int32_t argc, char **argv);
/*******************************************************************************
 * Variables
 ******************************************************************************/\
uint32_t g_loglevel;
uint32_t g_logflags = 0xFFFFFFFF;
const char *g_log_color[] =
{
    "",     /* LOG_DEBUG */
    "",     /* LOG_INFO */
    YELLOW, /* LOG_WARN */
    HL_RED /* LOG_ERROR */ 
};

const char *g_log_level[] =
{
    "DEBUG: ",  /* LOG_DEBUG */
    "INFO: ",  /* LOG_INFO */
    "WARN: ",  /* LOG_WARN */
    "ERROR: " /* LOG_ERROR */    
};
extern shell_handle_t g_shellHandle;
static shell_command_t s_logcmds[] = {
	{"loglvl", FORMAT_CMD_COLOR("loglvl")"               Get or set the logging level\r\n", loglevel, SHELL_IGNORE_PARAMETER_COUNT},
	{"logbit", FORMAT_CMD_COLOR("logbit")"               Get, set or clear logging flags (a bit per module)\r\n", logbit, SHELL_IGNORE_PARAMETER_COUNT},
};
/*******************************************************************************
 * Code
 ******************************************************************************/
 
/*******************************************************************************
* The function is used to set the global logging level. 
* All loggings with level lower than the global logging level are filtered
*******************************************************************************/
 static shell_status_t loglevel(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t value;
    char * endp;
	
    if (argc == 1) 
    {
        SHELL_Printf(context, "Logging level is "HL_PURPLE"0x%08X"NOCOLOR"\r\n", g_loglevel);
        return kStatus_SHELL_Success; 
    }
    
    if (argc == 2) 
    {
        if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0)) 
        {
            SHELL_Printf(context,"Usage:\r\n    loglvl                Get current logging level\r\n"\
                                           "    loglvl num            Set logging level to num\r\n");
            return kStatus_SHELL_Success;
        }
        
        value= strtoul(argv[1], &endp, 0);
        
        if (argv[1][0] != '-' && *endp == 0) 
        {
            g_loglevel = value;
            SHELL_Printf(context, "Logging level is set to "HL_PURPLE"0x%08X"NOCOLOR"\r\n", g_loglevel);
            return kStatus_SHELL_Success;
        }
    }
    
    SHELL_Printf(context, "Invalid parameter\r\n");
    
    return kStatus_SHELL_Success;    
}

/*******************************************************************************
* The function is used to set/clear the specific bit of the global logging flags.
* There're 32 bits available. A bit is for a modules. User can use this function
* to disable other modules' logging by clearing other bits but its own bit set. 
********************************************************************************/
 static shell_status_t logbit(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t value;
    char * endp;
	
    if (argc == 1)
    {
        SHELL_Printf(context, "Logging flags is "HL_PURPLE"0x%08X"NOCOLOR"\r\n", g_logflags);
        return kStatus_SHELL_Success; 
    }
            
    if (argc == 2 && ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0))) 
    {
        SHELL_Printf(context,"Usage:\r\n    logbit                Get current logging flags\r\n"\
		                       "    logbit -g num         Set logging group flags with num\r\n"\
		                       "    logbit -s n           Set logging flag in nth bit\r\n"\
		                       "    logbit -c n           Clear logging flag in nth bit\r\n");
        return kStatus_SHELL_Success;
    }
    else if (argc == 3) 
    {
        /* Clear the errno before a call after which you'll check errno */
        errno = 0;
        value = strtoul(argv[2], &endp, 0);
        /* Excluding minus and non-data nput */
        if (*endp == 0 && argv[2][0] != '-')
        {
            if (strcmp(argv[1], "-g") == 0 && errno != ERANGE)
            {
                g_logflags = value;
                SHELL_Printf(context, "Logging flags is set to "HL_PURPLE"0x%08X"NOCOLOR"\r\n", g_logflags);
                return kStatus_SHELL_Success;
            }
            else if (strcmp(argv[1], "-s") == 0 && value < 32)
            {          
                g_logflags |= (1 << value);
                SHELL_Printf(context, "Logging flag bit "HL_PURPLE"%d"NOCOLOR" is set ("HL_PURPLE"0x%08X"NOCOLOR")\r\n", value, g_logflags);
                return kStatus_SHELL_Success;
            }
            else if ((strcmp(argv[1], "-c") == 0) && value < 32)
            {          
                g_logflags &= ~(1 << value);
                SHELL_Printf(context, "Logging flag bit "HL_PURPLE"%d"NOCOLOR" is cleared ("HL_PURPLE"0x%08X"NOCOLOR")\r\n", value, g_logflags);
                return kStatus_SHELL_Success;
            }
        }
    }
    
    SHELL_Printf(context, "Invalid parameter\r\n");
    
    return kStatus_SHELL_Success;    
}

/* function to register logging related commands to shell */
void LogCmdRegister(void)
{
    for (uint32_t i = 0; i < (sizeof(s_logcmds) / sizeof(s_logcmds[0])); i++) 
    {
        SHELL_RegisterCommand(g_shellHandle, &s_logcmds[i]);
    }
}