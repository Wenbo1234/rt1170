/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v7.0
processor: MIMXRT1176xxxxx
package_id: MIMXRT1176DVMAA
mcu_data: ksdk2_0
processor_version: 0.0.2
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */   
#include "fsl_common.h"
#include "fsl_iomuxc.h" 
#include "fsl_lpspi.h"
#include "fsl_gpio.h"   
#include "led.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/      
#if Example_Num==12
uint16_t EXAMPLE_LEDdata[12] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800};
#elif Example_Num==2
uint16_t EXAMPLE_LEDdata[2] = {0x003F, 0x0FC0};
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LED_SPI_InitPins(void)
{
  CLOCK_EnableClock(kCLOCK_Iomuxc);           /* LPCG on: LPCG is ON. */
  
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_LPSR_10_LPSPI6_SCK,           
      0U);                                    
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_LPSR_09_GPIO12_IO09,       
      0U);                                   
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_LPSR_11_LPSPI6_SOUT,          
      0U);                                                          
   IOMUXC_SetPinMux(
      IOMUXC_GPIO_LPSR_12_GPIO12_IO12,         
      1U);  
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_LPSR_13_GPIO12_IO13,         
      1U);   
  
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_LPSR_10_LPSPI6_SCK,        
      0x02U);  
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_LPSR_09_GPIO12_IO09,         
      0x02U);                                
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_LPSR_11_LPSPI6_SOUT,        
      0x02U);                                 
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_LPSR_12_GPIO12_IO12,            
      0x02U); 
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_LPSR_13_GPIO12_IO13,            
      0x02U); 
}

void LED_SpiInitHardware(void)
{
    LED_SPI_InitPins();
    
    gpio_pin_config_t LED_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(APP_SPI_LED_RCK_GPIO, APP_SPI_LED_RCK_PIN, &LED_config);
    GPIO_PinInit(APP_SPI_LED_CLR_GPIO, APP_SPI_LED_CLR_PIN, &LED_config);
    GPIO_PinInit(APP_SPI_LED_G_GPIO, APP_SPI_LED_G_PIN, &LED_config);
    
    GPIO_PinWrite(APP_SPI_LED_RCK_GPIO, APP_SPI_LED_RCK_PIN, 0U);
    GPIO_PinWrite(APP_SPI_LED_CLR_GPIO, APP_SPI_LED_CLR_PIN, 1U);  
    GPIO_PinWrite(APP_SPI_LED_G_GPIO, APP_SPI_LED_G_PIN, 1U);  
}

void LED_Write(uint16_t *led_data)
{
    uint8_t masterTxData[2];
    lpspi_transfer_t transfer;
        
    masterTxData[0] = *led_data;
    masterTxData[1] = *led_data>>8;
    transfer.dataSize = 2;
    transfer.rxData = NULL;
    transfer.txData = masterTxData;
    transfer.configFlags = kLPSPI_MasterPcs0;
    while(kStatus_Success != LPSPI_MasterTransferBlocking(LED_LPSPI_MASTER_BASEADDR, &transfer));
    RCK_Toggle();
}

void RCK_Toggle(void)
{
    GPIO_PinWrite(APP_SPI_LED_RCK_GPIO, APP_SPI_LED_RCK_PIN, 1U);
    SDK_DelayAtLeastUs(1, CoreClock_Hz);
    GPIO_PinWrite(APP_SPI_LED_RCK_GPIO, APP_SPI_LED_RCK_PIN, 0U);
}

void SPICS_Control(bool enable)
{
    if (!enable)
    {
        GPIO_PinWrite(APP_SPI_LED_G_GPIO, APP_SPI_LED_G_PIN, 1U); 
    }
    else
    {
        GPIO_PinWrite(APP_SPI_LED_G_GPIO, APP_SPI_LED_G_PIN, 0U); 
    }
}

void LED_SpiInit(void)
{
    uint32_t srcClock_Hz;
    lpspi_master_config_t masterConfig;
    
//    clock_root_config_t rootCfg = {0};
//    rootCfg.mux = 1;
//    rootCfg.div = 1;
//    CLOCK_SetRootClock(kCLOCK_Root_Lpspi6, &rootCfg);
    
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    
    /*Master config*/
    masterConfig.baudRate     = LED_LPSPI_TRANSFER_BAUDRATE;
    masterConfig.bitsPerFrame = 12;
    masterConfig.cpol         = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha         = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction    = kLPSPI_MsbFirst;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;

    srcClock_Hz = LED_LPSPI_MASTER_CLK_FREQ;
    
    LPSPI_MasterInit(LED_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

    /*Set the RX and TX watermarks to reduce the ISR times.*/
    LPSPI_SetFifoWatermarks(LED_LPSPI_MASTER_BASEADDR, 0, 0);
}
