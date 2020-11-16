/*
 * Copyright (c) 2015 Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "i2c_eeprom.h"
#include "fsl_iomuxc.h"
#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
lpi2c_master_config_t eepromConfig = {
    .enableMaster            = true,
    .enableDoze              = true,
    .debugEnable             = false,
    .ignoreAck               = false,
    .pinConfig               = kLPI2C_2PinOpenDrain,
    .baudRate_Hz             = EEPROM_BAUDRATE,
    .busIdleTimeout_ns       = 0,
    .pinLowTimeout_ns        = 0,   
    .sdaGlitchFilterWidth_ns = 0,  
    .sclGlitchFilterWidth_ns = 0,
    .hostRequest.enable      = false,
    .hostRequest.source      = kLPI2C_HostRequestExternalPin,
    .hostRequest.polarity    = kLPI2C_HostRequestPinActiveHigh,
};

lpi2c_master_handle_t eeprom_handle;

volatile bool g_MasterCompletionFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void EEPROM_I2C_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(
        IOMUXC_GPIO_AD_08_LPI2C1_SCL,
        1U);
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_AD_09_LPI2C1_SDA,           
        1U);
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_AD_08_LPI2C1_SCL,
        0x10U);
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_AD_09_LPI2C1_SDA,
        0x10U);
}


/* Callback for eeprom transfer*/
void lpi2c_eeprom_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_MasterCompletionFlag = true;
    }
}

void EEPROM_I2C_Init(void)
{
    EEPROM_I2C_InitPins();
  
    LPI2C_MasterInit(EEPROM_I2C_BASSADDR, &eepromConfig, EEPROM_I2C_CLOCK_FREQ);
    /* Create the LPI2C handle for the non-blocking transfer */
    LPI2C_MasterTransferCreateHandle(EEPROM_I2C_BASSADDR, &eeprom_handle, lpi2c_eeprom_callback, NULL);
}

void EEPROM_I2C_Deinit(void)
{
    LPI2C_MasterDeinit(EEPROM_I2C_BASSADDR);
}

status_t EEPROM_Write_Byte(uint8_t *wBuf, uint16_t wAddr)
{    
    lpi2c_master_transfer_t xfer;
    
    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = EEPROM_ADDR;
    xfer.direction      = kLPI2C_Write;
    xfer.subaddress     = wAddr;
    xfer.subaddressSize = EEPROM_ADDR_SIZE;
    xfer.data           = wBuf;
    xfer.dataSize       = 1;

    return LPI2C_MasterTransferNonBlocking(EEPROM_I2C_BASSADDR, &eeprom_handle, &xfer);
}

status_t EEPROM_Write_Page(uint8_t *wBuf, uint16_t wAddr, uint32_t size)
{
    /*Judge if the number of write bytes crossed the page boundary*/
    if(size <= (EEPROM_PAGE_SIZE - wAddr % EEPROM_PAGE_SIZE))
    {
        lpi2c_master_transfer_t xfer;
        
        xfer.flags          = kLPI2C_TransferDefaultFlag;
        xfer.slaveAddress   = EEPROM_ADDR;
        xfer.direction      = kLPI2C_Write;
        xfer.subaddress     = wAddr;
        xfer.subaddressSize = EEPROM_ADDR_SIZE;
        xfer.data           = wBuf;
        xfer.dataSize       = size;

        return LPI2C_MasterTransferNonBlocking(EEPROM_I2C_BASSADDR, &eeprom_handle, &xfer);
    }
    else
    {
        return kStatus_OutOfRange;
    }
}

status_t EEPROM_Write(uint8_t *wBuf, uint16_t wAddr, uint32_t size)
{
    uint32_t pageNum = 0;
    uint32_t singleNum = 0;
    uint16_t addrOff = 0;
    uint16_t count = 0; 

    addrOff = wAddr % EEPROM_PAGE_SIZE;
    count = EEPROM_PAGE_SIZE - addrOff;
    pageNum =  size / EEPROM_PAGE_SIZE;
    singleNum = size % EEPROM_PAGE_SIZE;
 
    /* If WriteAddress is EEPROM_PAGE_SIZE aligned  */
    if(addrOff == 0) 
    {
        /* If size < EEPROM_PAGE_SIZE */
        if(pageNum == 0) 
        {
            EEPROM_Write_Page(wBuf, wAddr, singleNum);
        }
        /* If size > EEPROM_PAGE_SIZE */
        else  
        {
            while(pageNum--)
            {
                EEPROM_Write_Page(wBuf, wAddr, EEPROM_PAGE_SIZE); 
                wAddr +=  EEPROM_PAGE_SIZE;
                wBuf  += EEPROM_PAGE_SIZE;
                SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            }
            if(singleNum != 0)
            {
                EEPROM_Write_Page(wBuf, wAddr, singleNum);
            }
        }
    }
    /* If WriteAddress is not EEPROM_PAGE_SIZE aligned  */
    else 
    {
        /* If size < EEPROM_PAGE_SIZE */
        if(pageNum== 0) 
        {
            if(size < count)
            {
                EEPROM_Write_Page(wBuf, wAddr, singleNum);
            }
            else
            {
                EEPROM_Write_Page(wBuf, wAddr, count);
                wAddr += count;
                wBuf += count;
                SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
                EEPROM_Write_Page(wBuf, wAddr, (size - count));
            }
        }
        /* If size > EEPROM_PAGE_SIZE */
        else
        {
            size -= count;
            pageNum =  size / EEPROM_PAGE_SIZE;
            singleNum = size % EEPROM_PAGE_SIZE;	
          
            if(count != 0)
            {  
                EEPROM_Write_Page(wBuf, wAddr, count);
                wAddr += count;
                wBuf += count;
                SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            } 
   
            while(pageNum--)
            {
                EEPROM_Write_Page(wBuf, wAddr, EEPROM_PAGE_SIZE);
                wAddr +=  EEPROM_PAGE_SIZE;
                wBuf += EEPROM_PAGE_SIZE;  
                SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            }
            if(singleNum != 0)
            {
                EEPROM_Write_Page(wBuf, wAddr, singleNum); 
            }
        }
    }
    
    return kStatus_Success;
}

status_t EEPROM_Read(uint8_t *rBuf, uint16_t rAddr, uint32_t size)
{
    lpi2c_master_transfer_t xfer;
    
    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = EEPROM_ADDR;
    xfer.direction      = kLPI2C_Read;
    xfer.subaddress     = rAddr;
    xfer.subaddressSize = EEPROM_ADDR_SIZE;
    xfer.data           = rBuf;
    xfer.dataSize       = size;

    return LPI2C_MasterTransferNonBlocking(EEPROM_I2C_BASSADDR, &eeprom_handle, &xfer);
}