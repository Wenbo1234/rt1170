/*
 * Copyright 2020 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_CONFIG_H_
#define __APP_CONFIG_H_

#define GPU_TIGER
  
/* Definition of symbols for preproccessor */
#define CPU_MIMXRT1176DVMAA_cm7
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
#define XIP_BOOT_HEADER_ENABLE 1
#define XIP_BOOT_HEADER_DCD_ENABLE 1
#endif
#define USB_STACK_FREERTOS
#define USB_STACK_FREERTOS_HEAP_SIZE 32768
#define DATA_SECTION_IS_CACHEABLE 1
#define FSL_RTOS_FREE_RTOS
#define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING
#define SERIAL_PORT_TYPE_USBCDC 1
#define PRINTF_FLOAT_ENABLE 0
#define SCANF_FLOAT_ENABLE 0
#define PRINTF_ADVANCED_ENABLE 1
#define SCANF_ADVANCED_ENABLE 0
#define SERIAL_PORT_TYPE_UART 1
#define OSA_USED
#define USE_SDRAM
#define FSL_OSA_TASK_ENABLE 1
#define DEBUG_CONSOLE_UART_INDEX 11

/* Definition of usdhc1 multi functions  */
#define NONE                            (0)
#define SD_CARD                         (1)
#define WIFI_CPRS                       (2)
#define WIFI_MRVL                       (3)

/* change the following macro to use usdhc1 for sdcard or cypress wifi or marvell wifi */
/* when use marvell wifi, change the macro in wifi_config.h for varied module type, default is SD8977 */
#define USE_USDHC1 NONE

#if defined(USE_USDHC1) && (USE_USDHC1 == SD_CARD)
  #define FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL 1
#elif defined(USE_USDHC1) && (USE_USDHC1 == WIFI_CPRS)
  #define FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL 1
  #define HIGH_SPEED_SDIO_CLOCK
  #define LWIP_NETIF_API 1
  #define WIFI_WICED
#elif defined(USE_USDHC1) && (USE_USDHC1 == WIFI_MRVL)
  #define FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL 1
#endif

/* Definition for audio playback and change the following macros to select MQS or CODEC */
#define AUDIO_PLAY
#define AUDIO_CODEC 0
#define AUDIO_MQS 1
#define AUDIO_SELECL 1

/* Definition for driving led output */
/* NOTICE: PCS PIN conflicts with Backlight_EN PIN */

//#define LED_DRIVE

/* connect the gps module pins with JP1 on board to use lpuart11 to print the gps info.
   when enable gps, lpuart11 shall NOT be used as debug port to print log anymore.
   use semihosted and change the SDK_DEBUGCONSOLE in the fsl_debug_console.h to print log.
   moreover, to print float type variables, change the Library Option1->Printf formatter to Full */
//#define GPS_ENABLED

/* Definition for driving eeprom rw test */
#define EEPROM_DRIVE

/* Definition to enable system logging */
#define LOGGING_CONFIG_ENABLE
#define LOGGING_MAIN_BIT                (31U) /* most significant bit for main task */
#define LOGGING_PEER_BIT                (30U)
#define LOGGING_CAN_RX_PRINT_BIT        (29U)
#define LOGGING_SD_CARD_BIT             (28U)
#define LOGGING_SDIO_CPRS_BIT           (27U)
#define LOGGING_SDIO_MRVL_BIT           (26U)
#define LOGGING_LPUART_GPS_BIT          (25U)

/* Define the display panel */
//#define DEMO_PANEL                DEMO_PANEL_TM103XDKP13
#define DEMO_PANEL                DEMO_PANEL_TM070RDHP10

/* Definition for the GPT used for task run time status */  
#define CPU_LD_GPT_IRQ_ID         GPT2_IRQn
#define CPU_LD_GPT                GPT2
#define CPU_LD_GPT_IRQHandler     GPT2_IRQHandler
/* Get source clock for GPT driver (GPT prescaler = 0) */
#define CPU_LD_GPT_CLK_FREQ       CLOCK_GetFreq(kCLOCK_OscRc48MDiv2)



/* Task priority definition */
#define LOGGING_OUT_TASK_PRIORITY               (tskIDLE_PRIORITY + 1)
#define LOGGING_OUT_TASK_STACK_SIZE             (1024U)

#define SHELL_OUT_TASK_PRIORITY                 (tskIDLE_PRIORITY + 2)
#define SHELL_OUT_TASK_STACK_SIZE               (1024U)

#define SHELL_TASK_PRIORITY                     (tskIDLE_PRIORITY + 3)
#define SHELL_TASK_STACK_SIZE                   (1024U)

  
#endif /* __APP_CONFIG_H_ */