/** @file mdev_sdio.c
 *
 *  @brief This file provides  mdev driver for SDIO
 *
 *  Copyright 2008-2020 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code ("Materials") are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

#include <mdev_sdio.h>
#include <wm_os.h>
#include <board.h>
#include <fsl_common.h>
#include <fsl_clock.h>
#include <fsl_sdio.h>
#include <fsl_sdmmc_spec.h>
#include <fsl_usdhc.h>
#include <wifi.h>

#define SDIO_CMD_TIMEOUT 2000

static sdio_card_t wm_g_sd;
static bool sdio_card_init_done = false;

/* sdio mdev and synchronization handles */
static mdev_t mdev_sdio;
static os_mutex_t sdio_mutex;

int sdio_drv_creg_read(mdev_t *dev, int addr, int fn, uint32_t *resp)
{
    int ret;

    ret = os_mutex_get(&sdio_mutex, OS_WAIT_FOREVER);
    if (ret == -WM_FAIL)
    {
        sdio_e("failed to get mutex\r\n");
        return 0;
    }

    if (SDIO_IO_Read_Direct(&wm_g_sd, (sdio_func_num_t)fn, addr, (uint8_t *)resp) != kStatus_Success)
    {
        os_mutex_put(&sdio_mutex);
        return 0;
    }

    os_mutex_put(&sdio_mutex);

    return 1;
}

int sdio_drv_creg_write(mdev_t *dev, int addr, int fn, uint8_t data, uint32_t *resp)
{
    int ret;

    ret = os_mutex_get(&sdio_mutex, OS_WAIT_FOREVER);
    if (ret == -WM_FAIL)
    {
        sdio_e("failed to get mutex\r\n");
        return 0;
    }

    if (SDIO_IO_Write_Direct(&wm_g_sd, (sdio_func_num_t)fn, addr, &data, true) != kStatus_Success)
    {
        os_mutex_put(&sdio_mutex);
        return 0;
    }

    *resp = data;

    os_mutex_put(&sdio_mutex);

    return 1;
}

int sdio_drv_read(mdev_t *dev, uint32_t addr, uint32_t fn, uint32_t bcnt, uint32_t bsize, uint8_t *buf, uint32_t *resp)
{
    int ret;
    uint32_t flags = 0;
    uint32_t param;

    ret = os_mutex_get(&sdio_mutex, OS_WAIT_FOREVER);
    if (ret == -WM_FAIL)
    {
        sdio_e("failed to get mutex\r\n");
        return 0;
    }

    if (bcnt > 1)
    {
        flags |= SDIO_EXTEND_CMD_BLOCK_MODE_MASK;
        param = bcnt;
    }
    else
        param = bsize;

    if (SDIO_IO_Read_Extended(&wm_g_sd, (sdio_func_num_t)fn, addr, buf, param, flags) != kStatus_Success)
    {
        os_mutex_put(&sdio_mutex);
        return 0;
    }

    os_mutex_put(&sdio_mutex);

    return 1;
}

int sdio_drv_write(mdev_t *dev, uint32_t addr, uint32_t fn, uint32_t bcnt, uint32_t bsize, uint8_t *buf, uint32_t *resp)
{
    int ret;
    uint32_t flags = 0;
    uint32_t param;

    ret = os_mutex_get(&sdio_mutex, OS_WAIT_FOREVER);
    if (ret == -WM_FAIL)
    {
        sdio_e("failed to get mutex\r\n");
        return 0;
    }

    if (bcnt > 1)
    {
        flags |= SDIO_EXTEND_CMD_BLOCK_MODE_MASK;
        param = bcnt;
    }
    else
        param = bsize;

    if (SDIO_IO_Write_Extended(&wm_g_sd, (sdio_func_num_t)fn, addr, buf, param, flags) != kStatus_Success)
    {
        os_mutex_put(&sdio_mutex);
        return 0;
    }

    os_mutex_put(&sdio_mutex);

    return 1;
}

static void SDIOCARD_DetectCallBack(bool isInserted, void *userData)
{
}

static void SDIO_CardInterruptCallBack(void *userData)
{
    usdhc_host_t *usdhcHost = (usdhc_host_t *)&(wm_g_sd.host);
    USDHC_DisableInterruptSignal(usdhcHost->base, kUSDHC_CardInterruptFlag);
    handle_cdint(0);
}

static const sdmmchost_detect_card_t wm_s_sdCardDetect = {
    .cdType       = kSDMMCHOST_DetectCardByHostDATA3,      // kSDMMCHOST_DetectCardByGpioCD
    .cdTimeOut_ms = (~0U),
    .cardInserted = SDIOCARD_DetectCallBack,
    .cardRemoved  = SDIOCARD_DetectCallBack,
};

static const sdmmchost_card_int_t wm_s_sdioCardInt = {
    .userData      = NULL,
    .cardInterrupt = SDIO_CardInterruptCallBack,
};

void sdio_enable_interrupt(void)
{
    if (wm_g_sd.isHostReady)
    {
        usdhc_host_t *usdhcHost = (usdhc_host_t *)&(wm_g_sd.host);
        USDHC_EnableInterruptSignal(usdhcHost->base, kUSDHC_CardInterruptFlag);
    }
}

void sdio_io_strength(uint32_t busClock_HZ)
{
    (void)busClock_HZ;
#if defined(SD8977)
    SDMMCHOST_CONFIG_SD_IO(CARD_BUS_FREQ_100MHZ1, CARD_BUS_STRENGTH_4);
#elif defined(SD8801)
    SDMMCHOST_CONFIG_SD_IO(CARD_BUS_FREQ_100MHZ1, CARD_BUS_STRENGTH_6);
#endif
}

static void sdio_controller_init(void)
{
    memset(&wm_g_sd, 0, sizeof(sdio_card_t));
    /* Save host information. */
    wm_g_sd.host.base           = SD_HOST_BASEADDR;
    wm_g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
    /* card detect type */
    wm_g_sd.usrParam.cd = &wm_s_sdCardDetect;
    /* call back function for card interrupt */
    wm_g_sd.usrParam.cardInt = &wm_s_sdioCardInt;

    //wm_g_sd.usrParam.ioStrength = &sdio_io_strength;
}

static int sdio_card_reset(void)
{
    if (sdio_card_init_done)
    {
        SDIO_HostReset(&wm_g_sd.host);
        return SDIO_CardReset(&wm_g_sd);
    }
    return WM_SUCCESS;
}

static int sdio_card_init(void)
{
    int ret = WM_SUCCESS;
    uint32_t resp;

    ret = SDIO_Init(&wm_g_sd);

    if (ret != WM_SUCCESS)
        return ret;

    sdio_drv_creg_read(&mdev_sdio, 0x0, 0, &resp);

    sdio_d("Card Version - (0x%x)", resp & 0xff);

    /* Mask interrupts in card */
    sdio_drv_creg_write(&mdev_sdio, 0x4, 0, 0x3, &resp);
    /* Enable IO in card */
    sdio_drv_creg_write(&mdev_sdio, 0x2, 0, 0x2, &resp);

    SDIO_SetBlockSize(&wm_g_sd, (sdio_func_num_t)0, 256);
    SDIO_SetBlockSize(&wm_g_sd, (sdio_func_num_t)1, 256);
    SDIO_SetBlockSize(&wm_g_sd, (sdio_func_num_t)2, 256);

    sdio_card_init_done = true;

    return ret;
}

int sdio_drv_init(void (*cd_int)(int))
{
    int ret;

    ret = os_mutex_create(&sdio_mutex, "sdio-mutex", OS_MUTEX_INHERIT);
    if (ret == -WM_FAIL)
    {
        sdio_e("Failed to create mutex\r\n");
        return -WM_FAIL;
    }

    /* Set the SDIO interrupt priority */
    NVIC_SetPriority(SD_HOST_IRQ, 5U);

    sdio_controller_init();

    if (sdio_card_reset() != WM_SUCCESS)
    {
        sdio_e("Card reset failed");
        return -WM_FAIL;
    }
    else
        sdio_d("Card reset successful");

    if (sdio_card_init() != WM_SUCCESS)
    {
        sdio_e("Card initialization failed");
        return -WM_FAIL;
    }
    else
        sdio_d("Card initialization successful");

    return WM_SUCCESS;
}
