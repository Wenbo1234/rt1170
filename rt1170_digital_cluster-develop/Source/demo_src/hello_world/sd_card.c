/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "fsl_common.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "logging.h"
#include "semphr.h"
#include "ff.h"
#include "diskio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* logging tool define */
#define sd_card_bit LOGGING_SD_CARD_BIT

#define sd_card_log_e(...) log_e(sd_card, ##__VA_ARGS__)
#define sd_card_log_w(...) log_w(sd_card, ##__VA_ARGS__)
#define sd_card_log_i(...) log_i(sd_card, ##__VA_ARGS__)
#define sd_card_log_d(...) log_d(sd_card, ##__VA_ARGS__)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SDCARD_DetectCallBack(bool isInserted, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* HostCD detect is not available on RT1170, for pin GPIO_AD_03 has no such function */
static const sdmmchost_detect_card_t s_sdCardDetect = {
    .cdType = kSDMMCHOST_DetectCardByGpioCD,
    .cdTimeOut_ms = (~0U),
    .cardInserted = SDCARD_DetectCallBack,
    .cardRemoved  = SDCARD_DetectCallBack,
};

static volatile bool s_cardInserted     = false;
static volatile bool s_cardInsertStatus = false;

static SemaphoreHandle_t s_CardDetectSemaphore = NULL;

static FATFS g_fileSystem; /* File system object */
/*******************************************************************************
 * Code
 ******************************************************************************/
extern TaskHandle_t sd_card;
static void SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    s_cardInsertStatus = isInserted;
    
    if(s_cardInsertStatus)
    {
        xTaskResumeFromISR(sd_card);
        xSemaphoreGiveFromISR(s_CardDetectSemaphore, NULL);
    }
    else
    {
        sd_card_log_w("Wait for a card to insert\r\n");
        vTaskSuspend(sd_card);
    }
        
}


static status_t DEMO_MakeFileSystem(void)
{
    FRESULT error;
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
    BYTE work[FF_MAX_SS];

    if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
    {
        sd_card_log_e("Mount volume failed.\r\n");
        return kStatus_Fail;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        sd_card_log_e("Change drive failed.\r\n");
        return kStatus_Fail;
    }
#endif

#if FF_USE_MKFS
    sd_card_log_i("Make file system......The time may be long if the card capacity is big.\r\n");
    if (f_mkfs(driverNumberBuffer, FM_ANY, 0U, work, sizeof work))
    {
        sd_card_log_e("Make file system failed.\r\n");
        return kStatus_Fail;
    }
#endif /* FF_USE_MKFS */
    
    sd_card_log_i("Create directory......\r\n");
    error = f_mkdir(_T("/dir_test"));
    if (error == FR_OK)
    {
        sd_card_log_d("Create directory dir_test success......\r\n");
    }
    else
    {
        sd_card_log_e("Make directory failed.reason %d\r\n", error);
        return kStatus_Fail;
    }

    return kStatus_Success;
}

void sdcard_task(void *pvParameters)
{
    NVIC_SetPriority(BOARD_SD_HOST_IRQ, 5U);
    sd_card_log_i("sd card fs task start.....\r\n");
    
    s_CardDetectSemaphore = xSemaphoreCreateBinary();
    
    /* Save host information. */
    g_sd.host.base           = SD_HOST_BASEADDR;
    g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
    /* card detect type */
    g_sd.usrParam.cd = &s_sdCardDetect;
    
    if(SD_HostInit(&g_sd) != kStatus_Success)
    {
        sd_card_log_e("SD host init fail\r\n");
        vTaskSuspend(NULL);
    }
    
    while(true)
    {
        if (xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY) == pdTRUE)
        {
            if (s_cardInserted != s_cardInsertStatus)
            {
                s_cardInserted = s_cardInsertStatus;
                /* power off card */
                SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
                if (s_cardInserted)
                {
                    sd_card_log_d("Card inserted.\r\n");
                    /* power on the card */
                    SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
                        
                    /* Init card. */
                    if (SD_CardInit(&g_sd) != kStatus_Success)
                    {
                        sd_card_log_e("SD card init failed.\r\n");
                        vTaskSuspend(NULL);
                    }
                    else
                    {
                        //start your sdcard application here
                        if(DEMO_MakeFileSystem() != kStatus_Success)
                        {
                            sd_card_log_e("Make filesystem failed.\r\n");
                            vTaskSuspend(NULL);
                        }   
                    }
                }
            }
        }
        else
        {   
            sd_card_log_e("Failed to take semaphore.\r\n");
        }
    }
}


