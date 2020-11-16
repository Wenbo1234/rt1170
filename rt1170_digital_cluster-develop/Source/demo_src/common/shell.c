/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsl_common.h"
#include "board.h"
#include "serial_manager.h"
#include "shell_color.h"
#include "fsl_shell.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_debug_console.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "serial_port_usb.h"
#include "usb_phy.h"
#include "syscmd.h"
#include "logging.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_NUMBERS 1U
#define LED_1_INIT() USER_LED_INIT(LOGIC_LED_OFF)
#define LED_1_ON() USER_LED_ON()
#define LED_1_OFF() USER_LED_OFF()
#define USB_COM_RECEIVE_BUFFER_LEN (1024U)
#define CONTROLLER_ID kSerialManager_UsbControllerEhci0
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Led_Init(void);

static shell_status_t LedControl(shell_handle_t shellHandle, int32_t argc, char **argv);

void USB_DeviceClockInit(void);

void BOARD_ConfigUSBMPU(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
SHELL_COMMAND_DEFINE(led,
                     "\""HL_BLUE"led arg1 arg2"NOCOLOR"\":        Trun on or off User LED\r\n",
                     LedControl,
                     SHELL_IGNORE_PARAMETER_COUNT);

static const char ledHelp[] = "\"led arg1 arg2\":\r\n Usage:\r\n    arg1: 1|2|3|4...         "
                     "   Led index\r\n    arg2: on|off                Led status\r\n";
 
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static uint8_t s_serialHandleBuffer[SERIAL_MANAGER_HANDLE_SIZE];
shell_handle_t g_shellHandle;
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
AT_NONCACHEABLE_SECTION(static uint8_t s_readRingBuffer[USB_COM_RECEIVE_BUFFER_LEN]);
#endif
static serial_handle_t s_serialHandle;
/*******************************************************************************
 * Code
 ******************************************************************************/
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
    
    usbClockFreq = 24000000;
    
    if (CONTROLLER_ID == kSerialManager_UsbControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
}

void Led_Init(void)
{
    LED_1_INIT();
}

static shell_status_t LedControl(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int32_t kLedIndex = ((int32_t)atoi(argv[1]));
    char *kLedCommand = argv[2];
    if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING_2) == 0)) {
         SHELL_Printf(shellHandle,"%s", ledHelp);
         return kStatus_SHELL_Success;
    }

    /* Check second argument to control led */
    switch (kLedIndex)
    {
#if defined(LED_NUMBERS) && LED_NUMBERS > 0
        case 1:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED_1_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED_1_OFF();
            }
            else
            {
                SHELL_Printf(shellHandle, "Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 1
        case 2:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED_2_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED_2_OFF();
            }
            else
            {
                SHELL_Printf(shellHandle, "Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 2
        case 3:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED_3_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED_3_OFF();
            }
            else
            {
                SHELL_Printf(shellHandle, "Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 3
        case 4:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED_4_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED_4_OFF();
            }
            else
            {
                SHELL_Printf(shellHandle, "Control conmmand is wrong!\r\n");
            }
            break;
#endif
        default:
            SHELL_Printf(shellHandle, "LED index is wrong\r\n");
            break;
    }
    return kStatus_SHELL_Success;
}

int Vcom_Init(void)
{
    serial_manager_config_t serialConfig;
    status_t status = (status_t)kStatus_SerialManager_Error;
    /* Init SHELL */
 
    serial_port_usb_cdc_config_t usbCdcConfig = {
        .controllerIndex = (serial_port_usb_cdc_controller_index_t)CONTROLLER_ID,
    };
    
    (void)status;
    
    USB_DeviceClockInit();
    
     serialConfig.type = kSerialPort_UsbCdc;
     
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
    serialConfig.ringBuffer     = &s_readRingBuffer[0];
    serialConfig.ringBufferSize = USB_COM_RECEIVE_BUFFER_LEN;
#endif
    
    serialConfig.portConfig = &usbCdcConfig;
    
#if defined(OSA_USED)
#if (defined(SERIAL_MANAGER_USE_COMMON_TASK) && (SERIAL_MANAGER_USE_COMMON_TASK > 0U))
#else
    /* Set the priority, task name and stacksize that're passed to
     * create a serial manager task for shell. The debug port will
     * create another serial manager task with lower priority, another
     * task name and stacksize in the console debug code.
     */
    serialConfig.taskPriority = SHELL_OUT_TASK_PRIORITY;
    serialConfig.tname = (uint8_t *)"SerialManager Shell";
    serialConfig.stacksize = SHELL_OUT_TASK_STACK_SIZE;
#endif
#endif    
    
    s_serialHandle = (serial_handle_t)&s_serialHandleBuffer[0];
    status  = (status_t)SerialManager_Init(s_serialHandle, &serialConfig);

    assert(kStatus_SerialManager_Success == status);
    
    return 0;
}

void Shell_Init(void)
{
    /* Init led */
    Led_Init();

    Vcom_Init();
    
    g_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(g_shellHandle, s_serialHandle, DEFINE_SHELL_PROMPT("SHELL>> "));
    
    /* Add new command to commands list */
    SHELL_RegisterCommand(g_shellHandle, SHELL_COMMAND(led));
    SysCmdRegister();
    LogCmdRegister();
    
    return;  
}
