/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "vglite_support.h"
#include "FreeRTOS.h"
#include "task.h"
#include "vg_lite_platform.h"
#include "vglite_window.h"
#include "fsl_debug_console.h"
#include "background.h"
//#include "front_led_on.h"
//#include "secury_50.h"
//#include "belt_32.h"
//#include "belt_32_50x50.h"
//#include "gImage_cir.h"
//#include "cir.h"
//#include "red_black_bg.h"
//#include "led_50x50.h"
//#include "led_50x50.h"
#include "belt.h"
#include "f_light.h"
#include "caffe.h"
#include "odoor.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if !DEMO_BUFFER_FIXED_ADDRESS //!0
//#define WENBO 2
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_frameBuffer[APP_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH][DEMO_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#define DEMO_BUFFER0_ADDR (uint32_t) s_frameBuffer[0]

#if APP_BUFFER_COUNT > 1
#define DEMO_BUFFER1_ADDR (uint32_t) s_frameBuffer[1]
#endif

#if APP_BUFFER_COUNT > 2
#define DEMO_BUFFER2_ADDR (uint32_t) s_frameBuffer[2]
#endif


static const uint32_t s_frameBufferAddress[APP_BUFFER_COUNT] = {DEMO_BUFFER0_ADDR,
#if APP_BUFFER_COUNT > 1
                                                                DEMO_BUFFER1_ADDR,
#endif
#if APP_BUFFER_COUNT > 2
                                                                DEMO_BUFFER2_ADDR,
#endif

};

//#define WENBO 2
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t wenbo_s_frameBuffer[APP_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH][WENBO_DEMO_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#define WENBO_DEMO_BUFFER0_ADDR (uint32_t) wenbo_s_frameBuffer[0]

#if APP_BUFFER_COUNT > 1
#define WENBO_DEMO_BUFFER1_ADDR (uint32_t) wenbo_s_frameBuffer[1]
#endif

#if APP_BUFFER_COUNT > 2 
#define WENBO_DEMO_BUFFER2_ADDR (uint32_t) wenbo_s_frameBuffer[2]
#endif

static const uint32_t wenbo_s_frameBufferAddress[APP_BUFFER_COUNT] = {WENBO_DEMO_BUFFER0_ADDR,
#if APP_BUFFER_COUNT > 1
                                                                WENBO_DEMO_BUFFER1_ADDR,
#endif
#if APP_BUFFER_COUNT > 2
                                                                WENBO_DEMO_BUFFER2_ADDR
#endif
};

//#define WENBO 2
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t wenbo1_s_frameBuffer[APP_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH][WENBO_XRGB8888_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#define WENBO_XRGB8888_BUFFER0_ADDR (uint32_t) wenbo1_s_frameBuffer[0]

#if APP_BUFFER_COUNT > 1
#define WENBO_XRGB8888_BUFFER1_ADDR (uint32_t) wenbo1_s_frameBuffer[1]
#endif

#if APP_BUFFER_COUNT > 2 
#define WENBO_XRGB8888_BUFFER2_ADDR (uint32_t) wenbo1_s_frameBuffer[2]
#endif

static const uint32_t wenbo1_s_frameBufferAddress[APP_BUFFER_COUNT] = {WENBO_XRGB8888_BUFFER0_ADDR,
#if APP_BUFFER_COUNT > 1
                                                                WENBO_XRGB8888_BUFFER1_ADDR,
#endif
#if APP_BUFFER_COUNT > 2
                                                                WENBO_XRGB8888_BUFFER2_ADDR
#endif
};

#define ICON_BUFFER_COUNT 5
#define ICON_BUFFER_WIDTH 50
#define ICON_BUFFER_HEIGHT 50

//store icon:light,belt,door,caffe.. added by wenbo
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t icon_s_frameBuffer[ICON_BUFFER_COUNT][ICON_BUFFER_HEIGHT][ICON_BUFFER_WIDTH][WENBO_XRGB8888_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#define ICON_BUFFER0_ADDR (uint32_t) icon_s_frameBuffer[0]

#if ICON_BUFFER_COUNT > 1
#define ICON_BUFFER1_ADDR (uint32_t) icon_s_frameBuffer[1]
#endif

#if ICON_BUFFER_COUNT > 2 
#define ICON_BUFFER2_ADDR (uint32_t) icon_s_frameBuffer[2]
#endif

#if ICON_BUFFER_COUNT > 3
#define ICON_BUFFER3_ADDR (uint32_t) icon_s_frameBuffer[3]
#endif

#if ICON_BUFFER_COUNT > 4 
#define ICON_BUFFER4_ADDR (uint32_t) icon_s_frameBuffer[4]
#endif


#define DUMP_FRAME_COUNT 10 
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t dump_s_frameBuffer[DUMP_FRAME_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH][DEMO_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#endif 
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static vg_lite_buffer_format_t video_format_to_vglite(video_pixel_format_t format)
{
    vg_lite_buffer_format_t fmt;
    switch (format)
    {
        case kVIDEO_PixelFormatRGB565:
            fmt = VG_LITE_BGR565;
            break;
            
        case kVIDEO_PixelFormatXRGB8888:
            fmt = VG_LITE_BGRA8888;
            break;            
            
        case kVIDEO_PixelFormatXBGR8888:
            fmt = VG_LITE_RGBA8888;
            break;
            
        case kVIDEO_PixelFormatBGR565:
            fmt = VG_LITE_RGB565;
            break;

        default:
            fmt = VG_LITE_RGB565;
            break;
    }

    return fmt;
}
// added by wenbo for bg test
vg_lite_error_t WENBO_VGLITE_CreateDisplay(vg_lite_display_t *display, uint8_t layer) 
{
    if (!display)
    return VG_LITE_INVALID_ARGUMENT;

    //need?
    //BOARD_PrepareDisplayController();
    
    //use layer 1 changed by wenbo for bg test
    //FBDEV_Open(&display->g_fbdev, &g_dc, 0);
    FBDEV_Open(&display->g_fbdev, &g_dc, layer );
    
    return VG_LITE_SUCCESS;
}

uint32_t s_imageStartAddr = 0x30100000;
//const uint32_t s_imageStartAddr = 0x30900000;

vg_lite_error_t WENBO_VGLITE_CreateWindow(vg_lite_display_t *display, vg_lite_window_t *window)
{
    vg_lite_error_t ret = VG_LITE_SUCCESS;
    status_t status;
    void *buffer;
    vg_lite_buffer_t *vg_buffer;
    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);

    window->bufferCount = APP_BUFFER_COUNT;
    window->display     = display;
    window->width       = DEMO_BUFFER_WIDTH;
    window->height      = DEMO_BUFFER_HEIGHT;
    window->current     = -1;
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    g_fbInfo->bufInfo.pixelFormat = DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = window->width;
    g_fbInfo->bufInfo.height      = window->height;
    g_fbInfo->bufInfo.strideBytes = DEMO_BUFFER_STRIDE_BYTE;
#if 1
    g_fbInfo->bufferCount = window->bufferCount;
    for (uint8_t i = 0; i < window->bufferCount; i++)
    {
        vg_buffer            = &(window->buffers[i]);
        g_fbInfo->buffers[i] = (void *)wenbo_s_frameBufferAddress[i];
        vg_buffer->memory    = g_fbInfo->buffers[i];
        vg_buffer->address   = wenbo_s_frameBufferAddress[i];
        vg_buffer->width     = g_fbInfo->bufInfo.width;
        vg_buffer->height    = g_fbInfo->bufInfo.height;
        vg_buffer->stride    = g_fbInfo->bufInfo.strideBytes;
        vg_buffer->format    = video_format_to_vglite(DEMO_BUFFER_PIXEL_FORMAT);

        //add by wenbo
        DbgConsole_Printf(" wenbo g_fbInfo->buffers[i]=0x%x\n", g_fbInfo->buffers[i]);
    }
#endif 
    

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    assert(buffer != NULL);

    memset(buffer, 0x55, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    //memcpy(wenbo_s_frameBufferAddress[0], , sizeof(uint8_t) * BUFFER_SIZE);
    //memcopy(wenbo_s_frameBufferAddress[0], "123123");

    // added by wenbo for test bg
    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_SetFrameBuffer(g_fbdev, (void *)s_imageStartAddr, 0);
    //FBDEV_SetFrameBuffer(g_fbdev, (void *)wenbo_s_frameBufferAddress[0], 0);

    FBDEV_Enable(g_fbdev);

    //by wenbo
    while(1);

    return ret;
}
#define LAYER_0 1 
#define LAYER_1 0 
#define LAYER_2 2 
#define LAYER_3 3 
#define LAYER_4 4
#define LAYER_5 5 

vg_lite_error_t VGLITE_CreateDisplay(vg_lite_display_t *display)
{
    if (!display)
        return VG_LITE_INVALID_ARGUMENT;

    //not multi call added by wenbo,tmp changed for bg test 
    //BOARD_PrepareDisplayController();
    
    //use layer 1 changed by wenbo for bg test
    //FBDEV_Open(&display->g_fbdev, &g_dc, 0);
    FBDEV_Open(&display->g_fbdev, &g_dc, LAYER_1);
    
    return VG_LITE_SUCCESS;
}

//extern static vg_lite_display_t wenbo_display_bg;

void APP_InitDisplay(vg_lite_display_t *display)
{

    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);
    void *buffer;
    status_t status;

    FBDEV_Open(g_fbdev, &g_dc, LAYER_0);

    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.pixelFormat = WENBO_XRGB8888_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = DEMO_BUFFER_WIDTH;
    g_fbInfo->bufInfo.height      = DEMO_BUFFER_HEIGHT;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;
    g_fbInfo->bufInfo.strideBytes = WENBO_XRGB8888_BUFFER_STRIDE_BYTE;

    g_fbInfo->bufferCount = 1;
    //WENBO_DEMO_BUFFER0_ADDR
    //use background data, added by wenbo
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    
    memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)gImage_background, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)gImage_red_black_bg, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    g_fbInfo->buffers[0] = (void *)WENBO_XRGB8888_BUFFER0_ADDR;

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    //add by wenbo
    DbgConsole_Printf(" APP_InitDisplay buffer=0x%x\n", buffer);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return;

}
//light display used lyer 2
void light_InitDisplay(vg_lite_display_t *display)
{

    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);
    void *buffer;
    status_t status;

    FBDEV_Open(g_fbdev, &g_dc, LAYER_2);

    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.pixelFormat = WENBO_XRGB8888_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = ICON_BUFFER_WIDTH;
    g_fbInfo->bufInfo.height      = ICON_BUFFER_HEIGHT;
    g_fbInfo->bufInfo.startX      = 300;
    g_fbInfo->bufInfo.startY      = 420;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;
    g_fbInfo->bufInfo.strideBytes = 50*4;

    g_fbInfo->bufferCount = 1;
    //WENBO_DEMO_BUFFER0_ADDR
    //use background data, added by wenbo
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_cir, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_icon, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    memcpy((uint8_t *)ICON_BUFFER0_ADDR, (uint8_t *)gImage_f_light, sizeof(gImage_f_light));

    g_fbInfo->buffers[0] = (void *)ICON_BUFFER0_ADDR;

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    //add by wenbo
    DbgConsole_Printf(" APP_InitDisplay buffer=0x%x\n", buffer);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return;

}

//belt display used lyer 3
void belt_InitDisplay(vg_lite_display_t *display)
{

    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);
    void *buffer;
    status_t status;

    FBDEV_Open(g_fbdev, &g_dc, LAYER_3);

    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.pixelFormat = WENBO_XRGB8888_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = ICON_BUFFER_WIDTH;
    g_fbInfo->bufInfo.height      = ICON_BUFFER_HEIGHT;
    g_fbInfo->bufInfo.startX      = 300+50+10;
    g_fbInfo->bufInfo.startY      = 420;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;
    g_fbInfo->bufInfo.strideBytes = 50*4;

    g_fbInfo->bufferCount = 1;
    //WENBO_DEMO_BUFFER0_ADDR
    //use background data, added by wenbo
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_cir, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_icon, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    memcpy((uint8_t *)ICON_BUFFER1_ADDR, (uint8_t *)gImage_belt, sizeof(gImage_belt));

    g_fbInfo->buffers[0] = (void *)ICON_BUFFER1_ADDR;

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    //add by wenbo
    DbgConsole_Printf(" APP_InitDisplay buffer=0x%x\n", buffer);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return;
}

//belt display used lyer 3
void odoor_InitDisplay(vg_lite_display_t *display)
{

    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);
    void *buffer;
    status_t status;

    FBDEV_Open(g_fbdev, &g_dc, LAYER_4);

    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.pixelFormat = WENBO_XRGB8888_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = ICON_BUFFER_WIDTH;
    g_fbInfo->bufInfo.height      = ICON_BUFFER_HEIGHT;
    g_fbInfo->bufInfo.startX      = 300+50+10+50+10;
    g_fbInfo->bufInfo.startY      = 420;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;
    g_fbInfo->bufInfo.strideBytes = 50*4;

    g_fbInfo->bufferCount = 1;
    //WENBO_DEMO_BUFFER0_ADDR
    //use background data, added by wenbo
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_cir, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_icon, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    memcpy((uint8_t *)ICON_BUFFER2_ADDR, (uint8_t *)gImage_odoor, sizeof(gImage_odoor));

    g_fbInfo->buffers[0] = (void *)ICON_BUFFER2_ADDR;

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    //add by wenbo
    DbgConsole_Printf(" APP_InitDisplay buffer=0x%x\n", buffer);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return;

}

//belt display used lyer 3
void caffe_InitDisplay(vg_lite_display_t *display)
{

    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);
    void *buffer;
    status_t status;

    FBDEV_Open(g_fbdev, &g_dc, LAYER_5);

    //FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.pixelFormat = WENBO_XRGB8888_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = ICON_BUFFER_WIDTH;
    g_fbInfo->bufInfo.height      = ICON_BUFFER_HEIGHT;
    g_fbInfo->bufInfo.startX      = 300+50+10+50+10+50+10;
    g_fbInfo->bufInfo.startY      = 420;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;
    g_fbInfo->bufInfo.strideBytes = 50*4;

    g_fbInfo->bufferCount = 1;
    //WENBO_DEMO_BUFFER0_ADDR
    //use background data, added by wenbo
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER0_ADDR, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_cir, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //memcpy((uint8_t *)WENBO_XRGB8888_BUFFER1_ADDR, (uint8_t *)gImage_icon, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    memcpy((uint8_t *)ICON_BUFFER3_ADDR, (uint8_t *)gImage_caffe, sizeof(gImage_caffe));

    g_fbInfo->buffers[0] = (void *)ICON_BUFFER3_ADDR;

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    //add by wenbo
    DbgConsole_Printf(" APP_InitDisplay buffer=0x%x\n", buffer);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return;

}

vg_lite_error_t VGLITE_CreateWindow(vg_lite_display_t *display, vg_lite_window_t *window)
{
    vg_lite_error_t ret = VG_LITE_SUCCESS;
    status_t status;
    void *buffer;
    vg_lite_buffer_t *vg_buffer;
    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);

    window->bufferCount = APP_BUFFER_COUNT;
    window->display     = display;
    window->width       = DEMO_BUFFER_WIDTH;
    window->height      = DEMO_BUFFER_HEIGHT;
    window->current     = -1;
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    //changed by wenbo for test
    g_fbInfo->bufInfo.pixelFormat = DEMO_BUFFER_PIXEL_FORMAT;
    //g_fbInfo->bufInfo.pixelFormat = WENBO_DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.width       = window->width;
    g_fbInfo->bufInfo.height      = window->height;
    g_fbInfo->bufInfo.strideBytes = DEMO_BUFFER_STRIDE_BYTE;
    //g_fbInfo->bufInfo.strideBytes = WENBO_DEMO_BUFFER_STRIDE_BYTE;

    g_fbInfo->bufferCount = window->bufferCount;
    for (uint8_t i = 0; i < window->bufferCount; i++)
    {
        vg_buffer            = &(window->buffers[i]);
        g_fbInfo->buffers[i] = (void *)s_frameBufferAddress[i];
        //g_fbInfo->buffers[i] = (void *)wenbo_s_frameBufferAddress[i];
        vg_buffer->memory    = g_fbInfo->buffers[i];
        vg_buffer->address   = s_frameBufferAddress[i];
        //vg_buffer->address   = wenbo_s_frameBufferAddress[i];
        vg_buffer->width     = g_fbInfo->bufInfo.width;
        vg_buffer->height    = g_fbInfo->bufInfo.height;
        vg_buffer->stride    = g_fbInfo->bufInfo.strideBytes;
        vg_buffer->format    = video_format_to_vglite(DEMO_BUFFER_PIXEL_FORMAT);
        //vg_buffer->format    = video_format_to_vglite(WENBO_DEMO_BUFFER_PIXEL_FORMAT);

        //add by wenbo
        DbgConsole_Printf(" g_fbInfo->buffers[i]=0x%x\n", g_fbInfo->buffers[i]);
    }

    

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    assert(buffer != NULL);

    memset(buffer, 0, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
    //added by wenbo
    //memcpy((uint8_t *)buffer, (uint8_t *)s_imageStartAddr, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return ret;
}

vg_lite_error_t VGLITE_DestoryWindow(void)
{
    return VG_LITE_SUCCESS;
}

vg_lite_error_t VGLITE_DestroyDisplay(void)
{
    return VG_LITE_SUCCESS;
}

vg_lite_buffer_t *VGLITE_GetRenderTarget(vg_lite_window_t *window)
{
    vg_lite_buffer_t *rt = NULL;
    void *memory         = FBDEV_GetFrameBuffer(&window->display->g_fbdev, 0);
    for (uint8_t i = 0; i < window->bufferCount; i++)
    {
        rt = &(window->buffers[i]);
        if (memory == rt->memory)
        {
            window->current = i;
            return rt;
        }
    }
    return NULL;
}

int dump_index = 0;
void VGLITE_SwapBuffers(vg_lite_window_t *window)
{
    vg_lite_buffer_t *rt;
    fbdev_fb_info_t *g_fbInfo = &(window->display->g_fbInfo);

    //FBDEV_SetFrameBuffer(&window->display->g_fbdev, rt->memory, 0);

    if (window->current >= 0 && window->current < window->bufferCount)
        rt = &(window->buffers[window->current]);
    else
        return;

    vg_lite_finish();

    //rt->memory
    //s_imageStartAddr
    #if 0 // dump framebuffer
    if(dump_index < 10) {
        memcpy(dump_s_frameBuffer[dump_index], rt->memory, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
        DbgConsole_Printf(" dump_s_frameBuffer[%d]=0x%p\n", (dump_index)%10, dump_s_frameBuffer[(dump_index)%10]);
        dump_index++;
    }
    #endif 

    #if 0 
    if(dump_index < 1) {
        memset(dump_s_frameBuffer[0], 0, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
        //DbgConsole_Printf(" dump_s_frameBuffer[%d]=0x%p\n", (dump_index)%10, dump_s_frameBuffer[(dump_index)%10]);
        dump_index = 2;
    }
    if (dump_index > 1 ) {
        FBDEV_SetFrameBuffer(&window->display->g_fbdev, dump_s_frameBuffer[0], 0);
        DbgConsole_Printf("set null\n");
        dump_index = 1; 
    }else {
        dump_index = 2;
        FBDEV_SetFrameBuffer(&window->display->g_fbdev, rt->memory, 0);
        DbgConsole_Printf("set rt->memory\n");
    }
    #endif 
    if (dump_index < 1) {
        dump_index = 1;
    }
    else
    {
        dump_index = 0;
        //memset(rt->memory, 0, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);
        //DbgConsole_Printf("set null\n");
        //DbgConsole_Printf(" rt->memory=0x%p\n", rt->memory);
        /* code */
    }
    
    FBDEV_SetFrameBuffer(&window->display->g_fbdev, rt->memory, 0);
    //DbgConsole_Printf(" g_fbInfo->buffers[i]=0x%p\n", rt->memory);
}
