/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DISPLAY_SUPPORT_H_
#define _DISPLAY_SUPPORT_H_
#include "fsl_common.h"
#include "fsl_dc_fb.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PANEL_TM103XDKP13 0
#define DEMO_PANEL_TM070RDHP10 1

#ifndef DEMO_PANEL
#define DEMO_PANEL DEMO_PANEL_TM103XDKP13
#endif

#define DEMO_BUFFER_FIXED_ADDRESS 0

#if DEMO_BUFFER_FIXED_ADDRESS
#define DEMO_BUFFER0_ADDR 0x80000000
#define DEMO_BUFFER1_ADDR 0x80200000
#endif

/* Definitions for the frame buffer. */
#define DEMO_BUFFER_COUNT 2 /* 2 is enough for DPI interface display. */

#define DEMO_BUFFER_PIXEL_FORMAT kVIDEO_PixelFormatRGB565
#define DEMO_BUFFER_BYTE_PER_PIXEL 2

#define WENBO_DEMO_BUFFER_PIXEL_FORMAT kVIDEO_PixelFormatRGB888
#define WENBO_DEMO_BUFFER_BYTE_PER_PIXEL 3


#define WENBO_XRGB8888_BUFFER_PIXEL_FORMAT kVIDEO_PixelFormatXRGB8888
#define WENBO_XRGB8888_BUFFER_BYTE_PER_PIXEL 4

#if (DEMO_PANEL_TM103XDKP13 == DEMO_PANEL)

#define DEMO_PANEL_WIDTH   1280
#define DEMO_PANEL_HEIGHT  480

#elif (DEMO_PANEL_TM070RDHP10 == DEMO_PANEL)

#define DEMO_PANEL_WIDTH   800
#define DEMO_PANEL_HEIGHT  480

#endif

#define DEMO_BUFFER_WIDTH DEMO_PANEL_WIDTH
#define DEMO_BUFFER_HEIGHT DEMO_PANEL_HEIGHT

#define DEMO_BUFFER_STRIDE_BYTE (DEMO_BUFFER_WIDTH * DEMO_BUFFER_BYTE_PER_PIXEL)

#define WENBO_DEMO_BUFFER_STRIDE_BYTE (DEMO_BUFFER_WIDTH * WENBO_DEMO_BUFFER_BYTE_PER_PIXEL)

#define WENBO_XRGB8888_BUFFER_STRIDE_BYTE (DEMO_BUFFER_WIDTH * WENBO_XRGB8888_BUFFER_BYTE_PER_PIXEL)
/* There is not frame buffer aligned requirement, consider the 64-bit AXI data
 * bus width and 32-byte cache line size, the frame buffer alignment is set to
 * 32 byte.
 */
#define FRAME_BUFFER_ALIGN 32

extern const dc_fb_t g_dc;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

status_t BOARD_PrepareDisplayController(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _DISPLAY_SUPPORT_H_ */
