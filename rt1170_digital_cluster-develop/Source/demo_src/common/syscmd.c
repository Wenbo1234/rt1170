/*
 * Copyright 2020 NXP
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
#include "fsl_device_registers.h"   /* minimum device-independent header level for NVIC_SystemReset() */
#include "fsl_shell.h"
#include "FreeRTOS.h"
#include "task.h"
#include "syscmd.h"
#include "shell_color.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_TASK_MSG_LEN                       ( 50U )


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 
#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1U)
/*******************************************************************************
 * Support for FreeRTOS runtime stats using hi-res PIT timer
 ******************************************************************************/
#include <fsl_gpt.h> 

volatile uint32_t u32runtimeTimerValue=0;
static shell_status_t cpuld(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t ps(shell_handle_t context, int32_t argc, char **argv);
#endif /* configUSE_STATS_FORMATTING_FUNCTIONS == 1 */

static shell_status_t reboot(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t ps(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t heapinfo(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t uptime(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t readword(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t readhalfword(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t readbyte(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t writeword(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t writehalfword(shell_handle_t context, int32_t argc, char **argv);
static shell_status_t writebyte(shell_handle_t context, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern shell_handle_t g_shellHandle;
static shell_command_t s_syscmds[] = {
    {"reboot",       FORMAT_CMD_COLOR("reboot")"               Reboot Gateway system\r\n",           reboot,         0},
#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1U)
    {"ps",           FORMAT_CMD_COLOR("ps")"                   Show Task information\r\n",           ps,             0},
    {"cpuld",        FORMAT_CMD_COLOR("cpuld")"                Show Task run time satus\r\n",        cpuld,          0},
#endif /* configUSE_STATS_FORMATTING_FUNCTIONS == 1 */
    {"heapinfo",     FORMAT_CMD_COLOR("heapinfo")"             Show Heap information\r\n",           heapinfo,       0},
    {"uptime",       FORMAT_CMD_COLOR("uptime")"               Show Up time\r\n",                    uptime,         0},
    {"rw",           FORMAT_CMD_COLOR("rw")"                   Read a word from an address\r\n",     readword,       1},
    {"rh",           FORMAT_CMD_COLOR("rh")"                   Read a halfword from an address\r\n", readhalfword,   1},
    {"rb",           FORMAT_CMD_COLOR("rb")"                   Read a byte from an address\r\n",     readbyte,       1}, 
    {"ww",           FORMAT_CMD_COLOR("ww")"                   Write a word to an address\r\n",      writeword,       SHELL_IGNORE_PARAMETER_COUNT},
    {"wh",           FORMAT_CMD_COLOR("wh")"                   Write a halfword to an address\r\n",  writehalfword,   SHELL_IGNORE_PARAMETER_COUNT},
    {"wb",           FORMAT_CMD_COLOR("wb")"                   Write a byte to an address\r\n",      writebyte,       SHELL_IGNORE_PARAMETER_COUNT}, 
};


/*******************************************************************************
 * Code
 ******************************************************************************/
static shell_status_t reboot(shell_handle_t context, int32_t argc, char **argv)
{
    SHELL_Printf(context, "System rebooting...\r\n");
    // TODO: do safety clean up
    
    NVIC_SystemReset();
    
    return kStatus_SHELL_Success;     
}


static shell_status_t heapinfo(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t CurrentUsedHeapSize = 0;
    uint32_t MaxEverUsedHeapSize = 0;
    uint32_t CurrentFreeHeapSize = 0;
    uint32_t MiniEverFreeHeapSize = 0;

    uint32_t CurrentUsedHeapPercent = 0;
    uint32_t MaxEverUsedHeapPercent = 0;

    CurrentFreeHeapSize = xPortGetFreeHeapSize();
    MiniEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();

    CurrentUsedHeapSize = configTOTAL_HEAP_SIZE - CurrentFreeHeapSize;
    MaxEverUsedHeapSize = configTOTAL_HEAP_SIZE - MiniEverFreeHeapSize;

    CurrentUsedHeapPercent = (int)(((float)CurrentUsedHeapSize / configTOTAL_HEAP_SIZE * 100) + 0.5);
    MaxEverUsedHeapPercent = (int)(((float)MaxEverUsedHeapSize / configTOTAL_HEAP_SIZE * 100) + 0.5);

    SHELL_Printf(context, HL_PURPLE"%d%%"NOCOLOR" used("HL_BLUE"%d "NOCOLOR"bytes left), "HL_PURPLE"%d%%"NOCOLOR" ever used("HL_BLUE"%d "NOCOLOR"bytes left)\r\n" , CurrentUsedHeapPercent, CurrentFreeHeapSize , MaxEverUsedHeapPercent, MiniEverFreeHeapSize);
    
    return kStatus_SHELL_Success;    
}

static shell_status_t uptime(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t hour = 0;
    uint32_t min = 0;
    uint32_t sec = 0;

    TickType_t ticktime = xTaskGetTickCount();

    /* Convert the system up time to seconds + hundredths of seconds */
    sec = ticktime / configTICK_RATE_HZ;
    
    hour = sec / (60 * 60);
    sec = sec % (60 * 60);
    min = sec / 60;
    sec = sec % 60;

    if (hour > 0) 
    {
        SHELL_Printf(context, "up %ud, %um, %us\r\n", hour, min, sec);
    } else if (min > 0) {
        SHELL_Printf(context, "up %um, %us\r\n", min, sec);
    } else {
        SHELL_Printf(context, "up %us\r\n", sec);
    }
    
    return kStatus_SHELL_Success;    
}

static shell_status_t readword(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    char * endp;

        
    if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rw");
         return kStatus_SHELL_Success;
    }
    
    address= strtoul(argv[1], &endp, 0);
    if (argv[1][0] == '-' || ULONG_MAX == address || *endp != 0) 
    {
        SHELL_Printf(context, "Invalid address\r\n");
        SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rw");
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(context, "*0x%08X = 0x%08X(%d)\r\n", address, *(uint32_t *)address, *(uint32_t *)address);
    
    return kStatus_SHELL_Success;    
}

static shell_status_t readhalfword(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    char * endp;
      
    if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rh");
         return kStatus_SHELL_Success;
    }

    address= strtoul(argv[1], &endp, 0);
    
    if (argv[1][0] == '-' || ULONG_MAX == address || *endp != 0) 
    {
        SHELL_Printf(context, "Invalid address\r\n");
        SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rh");
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(context, "*0x%08X = 0x%04X(%d)\r\n", address, *(uint16_t *)address, *(uint16_t *)address);;
    
    return kStatus_SHELL_Success;  
}

static shell_status_t readbyte(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    char * endp;
        
    if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rb");
         return kStatus_SHELL_Success;
    }

    address= strtoul(argv[1], &endp, 0);
    
    if (argv[1][0] == '-' || ULONG_MAX == address || *endp != 0) {
        SHELL_Printf(context, "Invalid address\r\n");
        SHELL_Printf(context,"Usage:\r\n    %s address\r\n", "rb");
        return kStatus_SHELL_Error;
    }
    
    SHELL_Printf(context, "*0x%08X = 0x%02X(%d)\r\n", address, *(uint8_t *)address, *(uint8_t *)address);
    
    return kStatus_SHELL_Success; 
}

static shell_status_t writeword(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    uint32_t value;
    char * endp1;
    char * endp2;

    if (argc ==2 && (strcmp(argv[1], HELP_STRING) == 0 || strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "ww");
         return kStatus_SHELL_Success;
    }
    
    if (argc == 3 ) 
    {
        errno = 0;
        address= strtoul(argv[1], &endp1, 0);
        value= strtoul(argv[2], &endp2, 0);
        
        if ((argv[1][0] == '-' || ULONG_MAX == address || *endp1 != 0) || (errno == ERANGE ||  *endp2 != 0)) 
        {
            SHELL_Printf(context, "Invalid address and/or value\r\n");
            return kStatus_SHELL_Error;
        }        
    } 
    else
    {
      SHELL_Printf(context, "Invalid parameter\r\n");
      SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "ww");
      return kStatus_SHELL_Error;
    }

    *(uint32_t *)address = value;
    SHELL_Printf(context, "Word@0x%08X is set 0x%08X(%d)\r\n", address, value, value);
    
    return kStatus_SHELL_Success;    
}

static shell_status_t writehalfword(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    uint32_t value;
    char * endp1;
    char * endp2;
    
    if (argc ==2 && (strcmp(argv[1], HELP_STRING) == 0 || strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "wh");
         return kStatus_SHELL_Success;
    }
    
    if (argc == 3 ) 
    {
        address= strtoul(argv[1], &endp1, 0);
        value= strtoul(argv[2], &endp2, 0);
        
        if ((argv[1][0] == '-' || ULONG_MAX == address || *endp1 != 0) || (USHRT_MAX < value || *endp2 != 0)) 
        {
            SHELL_Printf(context, "Invalid address and/or value\r\n");
            return kStatus_SHELL_Error;
        }        
    } 
    else
    {
      SHELL_Printf(context, "Invalid parameter\r\n");
      SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "wh");
      return kStatus_SHELL_Error;
    }

    *(uint16_t *)address = value;
    SHELL_Printf(context, "Halfword@0x%08X is set 0x%04X(%d)\r\n", address, value, value);
    
    return kStatus_SHELL_Success;   
}

static shell_status_t writebyte(shell_handle_t context, int32_t argc, char **argv)
{
    uint32_t address;
    uint32_t value;
    char * endp1;
    char * endp2;
    
    if (argc ==2 && (strcmp(argv[1], HELP_STRING) == 0 || strcmp(argv[1], HELP_STRING_2) == 0)) 
    {
         SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "wh");
         return kStatus_SHELL_Success;
    }
    
    if (argc == 3 ) 
    {
        address= strtoul(argv[1], &endp1, 0);
        value= strtoul(argv[2], &endp2, 0);
        
        if ((argv[1][0] == '-' || ULONG_MAX == address || *endp1 != 0) || (UCHAR_MAX < value || *endp2!= 0)) 
        {
            SHELL_Printf(context, "Invalid address and/or value\r\n");
            return kStatus_SHELL_Error;
        }        
    }
    else
    {
      SHELL_Printf(context, "Invalid parameter\r\n");
      SHELL_Printf(context,"Usage:\r\n    %s address value \r\n", "wb");
      
      return kStatus_SHELL_Error;
    }

    *(uint8_t *)address = value;
    SHELL_Printf(context, "Byte@0x%08X is set 0x%02X(%d)\r\n", address, value, value);
    
    return kStatus_SHELL_Success;
}

#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1U)

static shell_status_t ps(shell_handle_t context, int32_t argc, char **argv)
{
    UBaseType_t CurTaskNum;
    char *pcWriteBuffer=NULL;

    CurTaskNum = uxTaskGetNumberOfTasks();

    pcWriteBuffer = pvPortMalloc(CurTaskNum * MAX_TASK_MSG_LEN);

    if( pcWriteBuffer != NULL )
    {
        SHELL_Printf(context, HL_PURPLE"Task name\t\tState\tPri\tStack\tNum\r\n"NOCOLOR);
        SHELL_Printf(context, SET_COLOR_STR(HL_BLUE, "-----------------------------------------------------\r\n"));

        vTaskList(pcWriteBuffer);

        if (strlen(pcWriteBuffer) > (CurTaskNum * MAX_TASK_MSG_LEN))
        {
            SHELL_Printf(context, "pcWriteBuffer is not enough !!! \r\n");
            vPortFree(pcWriteBuffer);
            
            return kStatus_SHELL_Error; 
        }
        
        SHELL_Printf(context, "%s\r\n", pcWriteBuffer);
        SHELL_Printf(context, SET_COLOR_STR(HL_BLUE, "-----------------------------------------------------\r\n"));
        SHELL_Printf(context, SET_COLOR_STR(HL_GREEN, "B:Blocked, R:Ready, D:Deleted, S:Suspended, X:Executing\r\n"));

        vPortFree(pcWriteBuffer);
    }
    else
    {
        SHELL_Printf(context, "pvPortMalloc failed !!! \r\n");
    }
	
    return kStatus_SHELL_Success;     
}
 
void CPU_LD_GPT_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(CPU_LD_GPT, kGPT_OutputCompare1Flag);

    /* Increment our counter */
    u32runtimeTimerValue++;
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F, Cortex-M7, Cortex-M7F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
    __DSB();
#endif

}
 
/* Require hi-res timer for FreeRTOS runtime stats  */
void vConfigureTimerForRunTimeStats(void)
{
    uint32_t gptFreq;
    gpt_config_t gptConfig;
    
     GPT_GetDefaultConfig(&gptConfig);

    /* Initialize GPT module */
    GPT_Init(CPU_LD_GPT, &gptConfig);

    /* Get GPT clock frequency */
    gptFreq = CPU_LD_GPT_CLK_FREQ;

    /* Set both GPT modules to 1 second duration */
    GPT_SetOutputCompareValue(CPU_LD_GPT, kGPT_OutputCompare_Channel1, gptFreq / configTICK_RATE_HZ / 20);

    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(CPU_LD_GPT, kGPT_OutputCompare1InterruptEnable);

    /* Enable at the Interrupt */
    EnableIRQ(CPU_LD_GPT_IRQ_ID);
 
    /* Start channel 0 */
    GPT_StartTimer(CPU_LD_GPT);
 
    return;
}

shell_status_t cpuld(shell_handle_t context, int32_t argc, char **argv)
{ 
    UBaseType_t CurTaskNum;
    char *pcWriteBuffer=NULL;

    CurTaskNum = uxTaskGetNumberOfTasks();

    pcWriteBuffer = pvPortMalloc(CurTaskNum * MAX_TASK_MSG_LEN);

    if( pcWriteBuffer != NULL )
    {
        SHELL_Printf(context, HL_PURPLE"Task name\t\tAbs Time\t%%Time\r\n"NOCOLOR);
        SHELL_Printf(context, SET_COLOR_STR(HL_BLUE, "------------------------------------------------\r\n"));

        vTaskGetRunTimeStats(pcWriteBuffer);

        if (strlen(pcWriteBuffer) > (CurTaskNum * MAX_TASK_MSG_LEN))
        {
            SHELL_Printf(context, "pcWriteBuffer is not enough !!! \r\n");
            vPortFree(pcWriteBuffer);
            return kStatus_SHELL_Error; 
        }
        
        SHELL_Printf(context, "%s\r\n", pcWriteBuffer);
        SHELL_Printf(context, SET_COLOR_STR(HL_BLUE, "------------------------------------------------\r\n"));

        vPortFree(pcWriteBuffer);
    }
    else
    {
        SHELL_Printf(context, "pvPortMalloc failed !!! \r\n");
    }
	
    return kStatus_SHELL_Success; 
    
}
#endif /* configUSE_STATS_FORMATTING_FUNCTIONS == 1 */

void SysCmdRegister(void)
{
    for (uint32_t i = 0; i < (sizeof(s_syscmds) / sizeof(s_syscmds[0])); i++) 
    {
        SHELL_RegisterCommand(g_shellHandle, &s_syscmds[i]);
    }
}

