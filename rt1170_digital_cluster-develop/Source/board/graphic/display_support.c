/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "display_support.h"
#include "fsl_gpio.h"
#include "board.h"
#include "fsl_dc_fb_lcdifv2.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Back light CTL. */
#define LCD_BL_GPIO              GPIO8
#define LCD_BL_GPIO_PIN          (28U)
/* Back light EN. */
#define LCD_EN_GPIO              GPIO12
#define LCD_EN_GPIO_PIN          (9U)
/* RESET. */
#define LCD_RESET_GPIO           GPIO8
#define LCD_RESET_GPIO_PIN       (31U)
/* STBYB. */
#define LCD_STBYB_GPIO           GPIO9
#define LCD_STBYB_GPIO_PIN       (1U)

#define DelayMs                  VIDEO_DelayMs

/*
 * TM103XDKP13 and TM070RDHP10 panels
 */

#if (DEMO_PANEL ==  DEMO_PANEL_TM103XDKP13)
#define DEMO_HSW        (1U)
#define DEMO_HBP        (48U)
#define DEMO_HFP        (16U)
#define DEMO_VSW        (1U)
#define DEMO_VBP        (3U)
#define DEMO_VFP        (5U)
#elif (DEMO_PANEL ==  DEMO_PANEL_TM070RDHP10)
#define DEMO_HSW        (1U)
#define DEMO_HBP        (30U)
#define DEMO_HFP        (30U)
#define DEMO_VSW        (1U)
#define DEMO_VBP        (24U)
#define DEMO_VFP        (24U)
#endif

//* The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.

//changed to kLCDIFV2_DriveDataOnRisingClkEdge for "ÆÁÄ»ÉÏÐÇÐÇµãµã" by wenbo
#if 0 
#define DEMO_LCDIF_POL_FLAGS                                                             \
    (kLCDIFV2_DataEnableActiveHigh | kLCDIFV2_VsyncActiveLow | kLCDIFV2_HsyncActiveLow | \
     kLCDIFV2_DriveDataOnFallingClkEdge)
#endif

#define DEMO_LCDIF_POL_FLAGS                                                             \
    (kLCDIFV2_DataEnableActiveHigh | kLCDIFV2_VsyncActiveLow | kLCDIFV2_HsyncActiveLow | \
     kLCDIFV2_DriveDataOnRisingClkEdge)

#define DEMO_LCDIF LCDIFV2


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void BOARD_InitLcdifClock(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static dc_fb_lcdifv2_handle_t s_dcFbLcdifv2Handle = {0};

static const dc_fb_lcdifv2_config_t s_dcFbLcdifv2Config = {
    .lcdifv2       = DEMO_LCDIF,
    .width         = DEMO_PANEL_WIDTH,
    .height        = DEMO_PANEL_HEIGHT,
    .hsw           = DEMO_HSW,
    .hfp           = DEMO_HFP,
    .hbp           = DEMO_HBP,
    .vsw           = DEMO_VSW,
    .vfp           = DEMO_VFP,
    .vbp           = DEMO_VBP,
    .polarityFlags = DEMO_LCDIF_POL_FLAGS,
    .lineOrder     = kLCDIFV2_LineOrderRGB,
/* CM4 is domain 1, CM7 is domain 0. */
#if (__CORTEX_M <= 4)
    .domain = 1,
#else
    .domain = 0,
#endif
};

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsLcdifv2,
    .prvData = &s_dcFbLcdifv2Handle,
    .config  = &s_dcFbLcdifv2Config,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_InitLcdifClock(void)
{
    /*
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     *
     * For 60Hz frame rate, the TM103XDKP13 pixel clock should be 40MHz.
     * For 60Hz frame rate, the TM070RDHP10 pixel clock should be 27MHz.
     *
     */
    const clock_root_config_t lcdifv2ClockConfig = {
        .clockOff = false,
        .mfn      = 0,
        .mfd      = 0,
        .mux      = 4, /*!< PLL_528. */
#if (DEMO_PANEL == DEMO_PANEL_TM103XDKP13)
        .div = 12,
#elif (DEMO_PANEL ==  DEMO_PANEL_TM070RDHP10)
        .div = 19,
#endif
    };

    CLOCK_SetRootClock(kCLOCK_Root_Lcdifv2, &lcdifv2ClockConfig);
}


static void BOARD_InitLcdPanel(void)
{
    gpio_pin_config_t config = {
        kGPIO_DigitalOutput, 0,
    };

    /* Backlight. */
    GPIO_PinInit(LCD_BL_GPIO, LCD_BL_GPIO_PIN, &config);
    GPIO_PinInit(LCD_EN_GPIO, LCD_EN_GPIO_PIN, &config);

#if (DEMO_PANEL ==  DEMO_PANEL_TM103XDKP13)
    GPIO_PinInit(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN, &config);
    GPIO_PinInit(LCD_STBYB_GPIO, LCD_STBYB_GPIO_PIN, &config);
    
    /* Power LCD on */    
    GPIO_PinWrite(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN, 1);
    DelayMs(2);
    GPIO_PinWrite(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN, 0);
    DelayMs(5);
    GPIO_PinWrite(LCD_RESET_GPIO, LCD_RESET_GPIO_PIN, 1);
    DelayMs(6);
    GPIO_PinWrite(LCD_STBYB_GPIO, LCD_STBYB_GPIO_PIN, 1);
    DelayMs(140);
#endif

    GPIO_PinWrite(LCD_EN_GPIO, LCD_EN_GPIO_PIN, 1);
    GPIO_PinWrite(LCD_BL_GPIO, LCD_BL_GPIO_PIN, 1);
}


status_t BOARD_InitDisplayInterface(void)
{
    /* LCDIF v2 output to RGB LCD. */
    CLOCK_EnableClock(kCLOCK_Video_Mux);
    VIDEO_MUX->VID_MUX_CTRL.SET = VIDEO_MUX_VID_MUX_CTRL_PARA_LCD_SEL_MASK;

    /* Configure the panel. */
    BOARD_InitLcdPanel();
    
    return kStatus_Success;
}

void LCDIF2_IRQHandler(void)
{
    DC_FB_LCDIFV2_IRQHandler(&g_dc);
}

status_t BOARD_VerifyDisplayClockSource(void)
{
    status_t status;
    uint32_t srcClkFreq;

    /*
     * In this implementation, the SYSPLL2 (528M) clock is used as the source
     * of LCDIFV2 pixel clock and MIPI DSI ESC clock. The OSC24M clock is used
     * as the MIPI DSI DPHY PLL reference clock. This function checks the clock
     * source are valid. OSC24M is always valid, so only verify the SYSPLL2.
     */
    srcClkFreq = CLOCK_GetPllFreq(kCLOCK_Pll_SysPll2);
    if (528 != (srcClkFreq / 1000000))
    {
        status = kStatus_Fail;
    }
    else
    {
        status = kStatus_Success;
    }

    return status;
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    status = BOARD_VerifyDisplayClockSource();

    if (status != kStatus_Success)
    {
        PRINTF("Error: Invalid display clock source.\r\n");
        return status;
    }

    BOARD_InitLcdifClock();

    status = BOARD_InitDisplayInterface();

    if (kStatus_Success == status)
    {
        NVIC_SetPriority(LCDIF2_IRQn, 3);
        EnableIRQ(LCDIF2_IRQn);
    }

    return kStatus_Success;
}
