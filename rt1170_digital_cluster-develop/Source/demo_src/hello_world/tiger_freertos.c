/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"
#include "board.h"
#include "vglite_support.h"
#include "vglite_window.h"
#include "tiger_paths.h"
#include "needle.h"
/*-----------------------------------------------------------*/
#include "vg_lite.h"
#include "pin_mux.h"
#include "display_support.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEFAULT_SIZE 256.0f;
#define DEMO_SIZE   4
#define TESSAL_BUF_SIZE 1024 /* Must be a multiple of 16 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void vglite_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static vg_lite_display_t display;
static vg_lite_display_t wenbo_display_bg;
static vg_lite_display_t light_display;
static vg_lite_display_t belt_display;
static vg_lite_display_t odoor_display;
static vg_lite_display_t caffe_display;
static vg_lite_window_t window;
static vg_lite_window_t wenbo_window_bg;
int disbg    = 1;

static int zoomOut    = 0;
static int scaleCount = 0;
static vg_lite_matrix_t matrix;
static vg_lite_matrix_t matrix_needle2;
static vg_lite_matrix_t wenbo_matrix;
static vg_lite_matrix_t matrix_arrow;
int wenboDegree = 0;
int leftDegree = 0;
int rightDegree = 0;
int direction = 1;
int rdirection = 1;
int wenbo_init_pos = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void cleanup(void)
{
    uint8_t i;
    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }

    vg_lite_close();
}

static vg_lite_error_t wenbo_init_vg_lite(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int fb_width, fb_height;

    error = WENBO_VGLITE_CreateDisplay(&wenbo_display_bg, 0);
    if (error)
    {
        PRINTF("VGLITE_CreateDisplay failed: VGLITE_CreateDisplay() returned error %d\n", error);
        return error;
    }
    // Initialize the window.
    error = WENBO_VGLITE_CreateWindow(&wenbo_display_bg, &wenbo_window_bg);
    if (error)
    {
        PRINTF("VGLITE_CreateWindow failed: VGLITE_CreateWindow() returned error %d\n", error);
        return error;
    }
    // Initialize the draw.
    //error = vg_lite_init(1280, 480);
    //wenbo

    return error;
}

static vg_lite_error_t init_vg_lite(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int fb_width, fb_height;

    error = VGLITE_CreateDisplay(&display);
    if (error)
    {
        PRINTF("VGLITE_CreateDisplay failed: VGLITE_CreateDisplay() returned error %d\n", error);
        return error;
    }
    // Initialize the window.
    error = VGLITE_CreateWindow(&display, &window);
    if (error)
    {
        PRINTF("VGLITE_CreateWindow failed: VGLITE_CreateWindow() returned error %d\n", error);
        return error;
    }
    // Initialize the draw.
    //error = vg_lite_init(1280, 480);
    //wenbo
    error = vg_lite_init(800, 480);
    if (error)
    {
        PRINTF("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return error;
    }

    // Setup a scale at center of buffer.
    fb_width  = window.width;
    fb_height = window.height;
    vg_lite_identity(&matrix);
    vg_lite_identity(&matrix_needle2);
    vg_lite_identity(&matrix_arrow);

    vg_lite_translate(fb_width / 4.0, fb_height / 2.0, &matrix);
    //add a new matrix for needle2
    vg_lite_translate(fb_width / 1.35, fb_height / 2.0, &matrix_needle2);
    //vg_lite_translate(fb_width / 2.0, fb_height / 2.0, &matrix);
    vg_lite_translate(fb_width / 2.0, fb_height / 2.0, &matrix_arrow);

    vg_lite_scale(DEMO_SIZE, DEMO_SIZE, &matrix);
    vg_lite_scale(DEMO_SIZE, DEMO_SIZE, &matrix_needle2);
    vg_lite_scale(DEMO_SIZE, DEMO_SIZE, &matrix_arrow);

    return error;
}
#define ROATEDRE  12
void animateTiger()
{
    #if 0
    if (zoomOut)
    {
        vg_lite_scale(1.0101010101, 1.01010101, &matrix);
        if (0 == --scaleCount)
            zoomOut = 0;
    }
    else
    {
        vg_lite_scale(0.99, 0.99, &matrix);
        if (50 == ++scaleCount)
            zoomOut = 1;
    }
    #endif


    #if 0  //wenbo no loop
    if( leftDegree <= 0 )
    {
        direction = 1;
        //PRINTF("leftDegree =leftDegree <= 0==%d\n", leftDegree);
    }else if (leftDegree >= 220) {
        //PRINTF("leftDegree =leftDegree >= 210==%d\n", leftDegree);
        direction = -1;
    }

    if ( (direction > 0 ) && (leftDegree < 220) ) {
        //PRINTF("leftDegree =(direction > 0 ) && (leftDegree < 210)==%d\n", leftDegree);
        leftDegree += 1;
    }else if ( (direction < 0) && (leftDegree > 0) ) {
        leftDegree -= 1;
    }

    if( rightDegree <= 0 )
    {
        rdirection = ROATEDRE;
    }else if (rightDegree >= 216) {
        rdirection = -ROATEDRE;
    }

    if ( (rdirection > 0) && (rightDegree < 216) ) {
        rightDegree += ROATEDRE;
    }else if ( (rdirection < 0) && (rightDegree > 0) ) {
        rightDegree -= ROATEDRE;
    }
    #endif 

    //PRINTF("leftDegree ===%d\n", leftDegree);
    //PRINTF("rightDegree ======%d\n", rightDegree);

    //wenboDegree = wenboDegree%180;
    //vg_lite_rotate(direction, &matrix);
    vg_lite_rotate(rdirection, &matrix_needle2);
}
int g_color = 0;

static void redraw()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint8_t count;
    vg_lite_buffer_t *rt = VGLITE_GetRenderTarget(&window);
    if (rt == NULL)
    {
        PRINTF("vg_lite_get_renderTarget error\r\n");
        while (1)
            ;
    }

    // Draw the path using the matrix.
    //vg_lite_clear(rt, NULL, 0xFFFF0000);
    //changed by wenbo for test show bg 
    if ( wenbo_init_pos == 0) { 
        //PRINTF("wenbo_init_pos ===%d\n", wenbo_init_pos);
        wenbo_init_pos = 1;
        vg_lite_rotate(-20, &matrix);
        vg_lite_rotate(-20, &matrix_needle2);
        
    }
    vg_lite_clear(rt, NULL, 0x0);
    //vg_lite_clear(rt, NULL, 0xFFFF0000);

    //memset(rt->memory, 0, rt->height * rt->stride);
    //vTaskDelay(20);
    //PRINTF("vTaskDelay(50)==\n");
    //memcpy((uint8_t *)buffer, (uint8_t *)s_imageStartAddr, DEMO_BUFFER_HEIGHT * WENBO_DEMO_BUFFER_STRIDE_BYTE);
    if ( g_color == 0 ) {
        g_color = 1;
        needle2_color_data[0] = 0xFC00FC00;//green 
    } else if ( g_color == 1 ) {   
        g_color = 2;
        needle2_color_data[0] = 0x001F001F; //blue
    }else if ( g_color == 2 ){ 
        g_color = 3;
        needle2_color_data[0] = 0x03E003E0;//red
    }else { //white
        g_color = 0;
        needle2_color_data[0] = 0xFFFFFFFF;     
    }

    //PRINTF("g_color 0x%x , needle2_color_data = 0x%x \n", g_color, needle2_color_data[0]);

    error = vg_lite_draw(rt, &path_needle2[0], VG_LITE_FILL_NON_ZERO, &matrix_needle2, VG_LITE_BLEND_SRC_OVER, needle2_color_data[0]);

#if 0 //wenbo 
    for (count = 0; count < pathCount; count++)
    {   
        // changed by wenbo for blend mode 
        //error = vg_lite_draw(rt, &path[count], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[count]);
        //error = vg_lite_draw(rt, &path[count], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER, color_data[count]);
        error = vg_lite_draw(rt, &path_needle2[count], VG_LITE_FILL_NON_ZERO, &matrix_needle2, VG_LITE_BLEND_SRC_OVER, needle2_color_data[count]);
        //error = vg_lite_draw(rt, &path_left_arrow[count], VG_LITE_FILL_EVEN_ODD, &matrix_arrow, VG_LITE_BLEND_SRC_OVER, arrow_color_data[count]);
        //error = vg_lite_draw(rt, &path_rgiht_arrow[count], VG_LITE_FILL_EVEN_ODD, &matrix_arrow, VG_LITE_BLEND_SRC_OVER, arrow_color_data[count]);
        if (error)
        {
            PRINTF("vg_lite_draw() returned error %d\n", error);
            cleanup();
            return;
        }
    }
#endif     
    animateTiger();
    VGLITE_SwapBuffers(&window);

    return;
}

uint32_t getTime()
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

#if 0 
void disBg(void){

    vg_lite_error_t error = VG_LITE_SUCCESS;

    error = VGLITE_CreateDisplay(&wenbo_display_bg);
    if (error)
    {
        PRINTF("VGLITE_CreateDisplay failed: VGLITE_CreateDisplay() returned error %d\n", error);
        return error;
    }
    // Initialize the window.
    error = VGLITE_CreateWindow(&wenbo_display_bg, &wenbo_window_bg);
    if (error)
    {
        PRINTF("VGLITE_CreateWindow failed: VGLITE_CreateWindow() returned error %d\n", error);
        return error;
    }

}
#endif 

//const uint32_t s_imageStartAddr = 0x30100000;
extern void APP_InitDisplay(vg_lite_display_t *display);
extern void light_InitDisplay(vg_lite_display_t *display);
extern void belt_InitDisplay(vg_lite_display_t *display);
extern void odoor_InitDisplay(vg_lite_display_t *display);
extern void caffe_InitDisplay(vg_lite_display_t *display);

StackType_t *wenbo_pxStack;
char test_map1[1024]; //1024*1
//char test_map2[1024*1024]; //1024*4
//char test_map3[1024*1024]; //1024*4
//char test_map4[1024*1024]; //1024*4
void vglite_task(void *pvParameters)
{
    status_t status;
    vg_lite_error_t error;

    PRINTF("vglite_task test_map1=%p\r\n", test_map1);
    //PRINTF("vglite_task test_map2=%p\r\n", test_map2);
    ///PRINTF("vglite_task test_map3=%p\r\n", test_map3);
    //PRINTF("vglite_task test_map4=%p\r\n", test_map4);

    status = BOARD_PrepareVGLiteController();
    if (status != kStatus_Success)
    {
        PRINTF("Prepare VGlite contolor error\r\n");
        while (1)
            ;
    }
    BOARD_PrepareDisplayController();

    //APP_InitDisplay(&wenbo_display_bg);
    //light_InitDisplay(&light_display);
    //belt_InitDisplay(&belt_display);
    //odoor_InitDisplay(&odoor_display);
    //caffe_InitDisplay(&caffe_display);
    //while(1);

#if 0
   error = wenbo_init_vg_lite();
    if (error)
    {
        PRINTF("init_vg_lite failed: init_vg_lite() returned error %d\n", error);
        while (1)
            ;
    }

    PRINTF("before wenbo_redraw\n");

    //while (1) {
    wenbo_redraw();
    //}
    
#endif

    error = init_vg_lite();
    if (error)
    {
        PRINTF("init_vg_lite failed: init_vg_lite() returned error %d\n", error);
        while (1)
            ;
    }

#if 0
    error = wenbo_init_vg_lite();
    if (error)
    {
        PRINTF("init_vg_lite failed: init_vg_lite() returned error %d\n", error);
        while (1)
            ;
    }
#endif 

    uint32_t startTime, time, n = 0;
    startTime = getTime();

    wenbo_pxStack = pvPortMalloc( ( ( ( size_t ) 1024*1024 ) * sizeof( StackType_t ) ) );
    //StackType_t *pxStack = wenbo_pxStack;
    PRINTF("wenbo_pxStack=%p\r\n", wenbo_pxStack);

    while (1)
    {   
        //wenboDegree += 1;
        //wenboDegree = wenboDegree%180;
        redraw();
        n++;
        if (n >= 60)
        {
            time = getTime() - startTime;
            PRINTF("%d frames in %d seconds: %d fps\r\n", n, time / 1000, n * 1000 / time);
            n         = 0;
            startTime = getTime();
        }
    }
}
