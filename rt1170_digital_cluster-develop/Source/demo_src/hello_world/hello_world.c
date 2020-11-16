/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "fsl_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_soc_src.h"
#include "logging.h"
#include "flexcan_loopback.h"
#include "audio.h"
#include "led.h"
#include "i2c_eeprom.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (LOGGING_MAIN_BIT > 31)
#error Wrong bit is set
#endif

#define main_task_bit LOGGING_MAIN_BIT

#define main_log_e(...) log_e(main_task, ##__VA_ARGS__)
#define main_log_w(...) log_w(main_task, ##__VA_ARGS__)
#define main_log_i(...) log_i(main_task, ##__VA_ARGS__)
#define main_log_d(...) log_d(main_task, ##__VA_ARGS__)

#define peer_task_bit LOGGING_PEER_BIT

#define peer_log_e(...) log_e(peer_task, ##__VA_ARGS__)
#define peer_log_w(...) log_w(peer_task, ##__VA_ARGS__)
#define peer_log_i(...) log_i(peer_task, ##__VA_ARGS__)
#define peer_log_d(...) log_d(peer_task, ##__VA_ARGS__)

#define can_rx_print_task_bit LOGGING_CAN_RX_PRINT_BIT

#define can_rx_print_log_e(...) log_e(can_rx_print_task, ##__VA_ARGS__)
#define can_rx_print_log_w(...) log_w(can_rx_print_task, ##__VA_ARGS__)
#define can_rx_print_log_i(...) log_i(can_rx_print_task, ##__VA_ARGS__)
#define can_rx_print_log_d(...) log_d(can_rx_print_task, ##__VA_ARGS__)
/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 2)
#define peer_task_PRIORITY (configMAX_PRIORITIES - 3)
#define can_task_PRIORITY (configMAX_PRIORITIES - 2)
#define can_rx_print_task_PRIORITY (configMAX_PRIORITIES - 1)
#define audio_task_PRIORITY (configMAX_PRIORITIES - 2)
#define led_task_PRIORITY (configMAX_PRIORITIES - 5) 
#define eeprom_rw_task_PRIORITY (configMAX_PRIORITIES - 6)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
static void peer_task(void *pvParameters);
static void can_task(void *pvParameters);
static void audio_task(void *pvParameters);
static void led_task(void *pvParameters);
static void can_rx_print_task(void *pvParameters);
static void eeprom_rw_task(void *pvParameters);
void Shell_Init(void);
void BOARD_InitHardware(void);

#ifdef GPU_TIGER    
extern void vglite_task(void *pvParameters);
extern void wenbo_vglite_task(void *pvParameters);
#define tiger_task_PRIORITY (configMAX_PRIORITIES - 1)
#endif

#if (USE_USDHC1 == SD_CARD)
extern void sdcard_task(void *pvParameters);
#define sdcard_task_PRIORITY (configMAX_PRIORITIES - 5)
TaskHandle_t sd_card;
#elif (USE_USDHC1 == WIFI_CPRS)
extern void iperf_4343w_task(void *pvParameters);
#define iperf_4343w_task_PRIORITY (configMAX_PRIORITIES - 7)
#elif (USE_USDHC1 == WIFI_MRVL)  
extern void iperf_sd8801_task(void *pvParameters);
#define iperf_sd8801_task_PRIORITY (configMAX_PRIORITIES - 7)
#endif

#ifdef GPS_ENABLED
extern void lpuart_gps_task(void *pvParameters);
#define lpuart_gps_task_PRIORITY (configMAX_PRIORITIES - 9)
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_USDHCClockConfiguration(void)
{
#if (USE_USDHC1 == SD_CARD)
    
    clock_root_config_t rootCfg = {0};
    rootCfg.mux = 2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

#elif (USE_USDHC1 == WIFI_CPRS)
  
    clock_root_config_t rootCfg = {0};
    rootCfg.mux = 0;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);
    
#elif (USE_USDHC1 == WIFI_MRVL)

    CLOCK_InitPfd(kCLOCK_Pll_SysPll2, kCLOCK_Pfd0, 24);
    clock_root_config_t rootCfg = {0};
    rootCfg.mux = 5;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);
    
#endif
}


static void BOARD_ResetDisplayMix(void)
{
    /*
     * Reset the displaymix, otherwise during debugging, the
     * debugger may not reset the display, then the behavior
     * is not right.
     */
    SRC_AssertSliceSoftwareReset(SRC, kSRC_DisplaySlice);
    while (kSRC_SliceResetInProcess == SRC_GetSliceResetState(SRC, kSRC_DisplaySlice))
    {
    }
}

/*!
 * @brief Application main task entry point called by the main() in os abstract layer.
 */

void main_task(void const *argument)
{
    if (xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 10, NULL, hello_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(peer_task, "Peer_task", configMINIMAL_STACK_SIZE + 10, NULL, peer_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
  
    if (xTaskCreate(can_rx_print_task, "Can_rx_print_task", configMINIMAL_STACK_SIZE * 3, NULL, peer_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    
    if (xTaskCreate(can_task, "Can_task", configMINIMAL_STACK_SIZE * 3, NULL, peer_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
	
#ifdef AUDIO_PLAY     
    if (xTaskCreate(audio_task, "Audio_task", configMINIMAL_STACK_SIZE * 3, NULL, audio_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
#endif    
 
#ifdef LED_DRIVE     
    if (xTaskCreate(led_task, "LED_task", configMINIMAL_STACK_SIZE + 10, NULL, led_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
#endif
    
#ifdef GPU_TIGER   
#if 0
    if (xTaskCreate(wenbo_vglite_task, "wenbo_vglite_task", configMINIMAL_STACK_SIZE + 200, NULL, tiger_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task wenbo_vglite_task creation failed!.\r\n");
        while (1);
    }  
#else 
    if (xTaskCreate(vglite_task, "vglite_task", configMINIMAL_STACK_SIZE + 200, NULL, tiger_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1);
    }  
#endif 
#endif

#if (USE_USDHC1 == SD_CARD)

    if (xTaskCreate(sdcard_task, "sdcard_task", 300, NULL, sdcard_task_PRIORITY, &sd_card) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
#elif (USE_USDHC1 == WIFI_CPRS)
    
    if (xTaskCreate(iperf_4343w_task, "iperf_4343w_task", 2000, NULL, iperf_4343w_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }
#elif (USE_USDHC1 == WIFI_MRVL)     
    if (xTaskCreate(iperf_sd8801_task, "iperf_sd8801_task", 2000, NULL, iperf_sd8801_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }  
#endif

#ifdef GPS_ENABLED
    if (xTaskCreate(lpuart_gps_task, "lpuart_gps_task", 500, NULL, lpuart_gps_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }  
#endif
    
#ifdef EEPROM_DRIVE
    //if (xTaskCreate(eeprom_rw_task, "eeprom_rw_task", configMINIMAL_STACK_SIZE + 10, NULL, eeprom_rw_task_PRIORITY, NULL) != pdPASS)
    //if (xTaskCreate(eeprom_rw_task, "eeprom_rw_task", configMINIMAL_STACK_SIZE + 1000, NULL, eeprom_rw_task_PRIORITY, NULL) != pdPASS)
    //changed by wenbo for test 
    if (xTaskCreate(eeprom_rw_task, "eeprom_rw_task", configMINIMAL_STACK_SIZE + 1000, NULL, eeprom_rw_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
#endif    
    Shell_Init();
    
    vTaskDelete(NULL);
}


/*!
 * @brief Task responsible for printing of "Print the receved data from CAN" message.
 */
extern flexcan_fd_frame_t rxFrame;
extern SemaphoreHandle_t CAN_RX_Semaphr;
static void can_rx_print_task(void *pvParameters)
{
  for(;;)
  {
    if(CAN_RX_Semaphr != NULL)
    {
      xSemaphoreTake(CAN_RX_Semaphr,portMAX_DELAY);
      PRINTF("\r\nReceived message from MB%d\r\n", RX_MESSAGE_BUFFER_NUM);
      for (uint32_t i = 0; i < 16; i++)
      {
          can_rx_print_log_d("rx word%d = 0x%x\r\n", i, rxFrame.dataWord[i]);
      }
    }
    else
    {
      vTaskDelay(1000);
    }
  }
}

/*!
 * @brief Task responsible for printing of "Init Can task and send data for test" message.
 */

extern flexcan_data_buff_t CAN_TX;
static void can_task(void *pvParameters)
{
  CAN_RX_Semaphr = xSemaphoreCreateBinary();
  if (CAN_RX_Semaphr == NULL)
    PRINTF("\r\nCAN_RX_Semaphr Created Failed!!!!!!\r\n");
  uint32_t i = 0;
  for( i = 0; i < 16; i++)
  {
    CAN_TX.DATA[i] = CAN_WORD0_DATA_BYTE_0(0x55) | CAN_WORD0_DATA_BYTE_1(0x55) | CAN_WORD0_DATA_BYTE_2(0x55) |
                      CAN_WORD0_DATA_BYTE_3(0x55);
  }
   for (i = 0; i < 16; i++)
    {
        PRINTF("tx word%d = 0x%x\r\n", i, CAN_TX.DATA[i]);
    }
    PRINTF("CAN task.....\r\n");
    CANFD_Init();
    CANFD_RX_MailBox_Config(RX_MESSAGE_BUFFER_NUM, RX_ID);
    CANFD_TX_MailBox_Config(TX_MESSAGE_BUFFER_NUM, TX_ID);
    
    for (;;)
    {
      CANFD_TX_SendData(TX_MESSAGE_BUFFER_NUM, TX_ID,CAN_TX);
      vTaskDelay(1000);
    }
}

/*!
 * @brief Task responsible for audio playback.
 */
static void audio_task(void *pvParameters)
{
    uint8_t can_flag=2;
    audio_select_t audio_playback_select;
    
#if (AUDIO_SELECL == AUDIO_CODEC)
    Codec_InitHardware();
    Codec_InitClock();
    audio_playback_select = CODEC_playback;
#elif (AUDIO_SELECL == AUDIO_MQS)
    MQS_InitHardware();
    MQS_InitClock();
    audio_playback_select = MQS_playback;
#endif
    while(1)
    {    
        switch(can_flag)
        {
        case 1:
            Audio_PlayBack(audio_playback_select, kAUDIO_Type_0, kAUDIO_play_stop);
            can_flag=0;
            break;
        case 2:
            Audio_PlayBack(audio_playback_select, kAUDIO_Type_1, kAUDIO_play_start);
            can_flag=0;
            break;  
        case 3:
            Audio_PlayBack(audio_playback_select, kAUDIO_Type_2, kAUDIO_play_start);
            can_flag=0;
            break; 
        default:
            break;
        }
        vTaskDelay(1000);
    }
}

/*!
 * @brief Task responsible for driving led output.
 */
static void led_task(void *pvParameters)
{    
    uint8_t i = 0;
    LED_SpiInitHardware();
    LED_SpiInit();
    SPICS_Control(true);
    while(1)
    {
        i = (i + 1) % Example_Num;
        LED_Write(EXAMPLE_LEDdata+i);
        vTaskDelay(50);
    }
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task(void *pvParameters)
{
    uint32_t i = 0;

    //PRINTF("Hello world.....\r\n");    
    PRINTF("WENBO IN hello_task Hello world.....\r\n");    
    for (;;)
    {
       
        switch (i++) 
        {
        case 0:
            main_log_d("Test for debug %d", i);
            break;
        case 1:
            main_log_i("Test for info %d", i);
            break;
        case 2:
            main_log_w("Test for warn %d", i);
            break;
        case 3:
            main_log_e("Test for erro %d", i);
            break;
        default:
            i = 0;
            PRINTF("Hello world.\r\n");
            break;
        }

        vTaskDelay(100);
    }
}


static void peer_task(void *pvParameters)
{
    uint32_t i = 0;
    PRINTF("Peer task.....\r\n");    
    for (;;)
    {
       
        switch (i++) 
        {
        case 0:
            peer_log_d("Test for debug %d", i);
            break;
        case 1:
            peer_log_i("Test for info %d", i);
            break;
        case 2:
            peer_log_w("Test for warn %d", i);
            break;
        case 3:
            peer_log_e("Test for erro %d", i);
            break;
        default:
            i = 0;
            PRINTF("Peer task.\r\n");
            break;
        }

        vTaskDelay(100);
    }
}

static void eeprom_rw_task(void *pvParameters)
{
    uint8_t result = 0;
    uint8_t g_eeprom_wBuff[70] = {0};
    uint8_t g_eeprom_rBuff[70] = {0};
    uint32_t i = 1U;
    g_eeprom_wBuff[0] = 70 - 1U;
    for(i=1U; i<70; i++)
    {
        g_eeprom_wBuff[i] = i - 1;
    }

    EEPROM_I2C_Init();
     
    EEPROM_Write(g_eeprom_wBuff, 0x0031, 70);

    SDK_DelayAtLeastUs(40000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    EEPROM_Read(g_eeprom_rBuff, 0x0031,70);

    SDK_DelayAtLeastUs(40000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    result = memcmp(g_eeprom_rBuff, g_eeprom_wBuff,70);
    if(result != 0)
    {
        PRINTF("EEPROM RW failed!.\r\n");
    }
    else
    {
        PRINTF("EEPROM RW success!.\r\n");
    }
    
    while(1)
    {
        vTaskDelay(50);
    }

}


/*!
 * @brief Routine to init board hardware.
 */
void BOARD_InitHardware(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    
    BOARD_ResetDisplayMix();

#ifndef GPS_ENABLED
    BOARD_InitDebugConsole();
#endif
    BOARD_USDHCClockConfiguration();
}
