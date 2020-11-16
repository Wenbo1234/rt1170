/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Application:Digital Cluster
 */

 
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "phy_tja110x.h"
#include "fsl_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* PHY functions */
status_t PHY_TJA110x_Init(phy_handle_t *handle, const phy_config_t *config);
status_t PHY_HandleInterrupt(phy_handle_t *handle);
status_t PHY_TJA110x_GetLinkStatus(phy_handle_t *handle, bool* linkUp);
status_t PHY_TJA110x_GetLinkSpeed(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex);
status_t PHY_Sleep(phy_handle_t *handle);
status_t PHY_Wakeup(phy_handle_t *handle);
status_t PHY_SetLoopback(phy_handle_t *handle, phy_loopback_t loopbackMode);
status_t PHY_Resume(phy_handle_t *handle);

/* Internal/Helper functions */
static status_t PHY_RMR(phy_handle_t *handle, uint8_t registerAddr, uint16_t value, uint16_t mask);
static status_t PHY_GoFromStandbyToNormalMode(phy_handle_t *handle);
static status_t PHY_EnableInterrupts(phy_handle_t *handle);
static status_t PHY_GetID(phy_handle_t *handle, phy_id_t *id);
static status_t PHY_HandleInterruptPwon(phy_handle_t *handle);
static status_t PHY_HandleInterruptWakeUp(phy_handle_t *handle);
static void PHY_HandleInterruptLinkDown(phy_handle_t *handle);
static status_t PHY_HandleInterruptLinkUp(phy_handle_t *handle);


/*******************************************************************************
 * Variables
 ******************************************************************************/
const phy_operations_t phytja110x_ops = {.init               = PHY_TJA110x_Init,
                                         .write              = PHY_TJA110x_Write,
                                         .read               = PHY_TJA110x_Read,
                                         .getLinkStatus      = PHY_TJA110x_GetLinkStatus,
                                         .getLinkSpeedDuplex = PHY_TJA110x_GetLinkSpeedDuplex};

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t PHY_TJA110x_Write(phy_handle_t *handle, uint32_t phyReg, uint32_t data)
{
    return MDIO_Write(handle->mdioHandle, handle->phyAddr, phyReg, data);
}

status_t PHY_TJA110x_Read(phy_handle_t *handle, uint32_t phyReg, uint32_t *dataPtr)
{
    return MDIO_Read(handle->mdioHandle, handle->phyAddr, phyReg, dataPtr);
} 

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_EnableLink
 * Description   : Enables the link for the specified PHY device.
 *
 *END**************************************************************************/
static inline status_t PHY_EnableLink(phy_handle_t *handle)
{
    return PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, PHY_EXTENDED_CTRL_LINK_CTRL_MASK, PHY_EXTENDED_CTRL_LINK_CTRL_MASK);
}

/*FUNCTION**********************************************************************
*
* Function Name : PHY_DisableLink
* Description   : Disables the link for the specified PHY device.
*
*END**************************************************************************/
static inline status_t PHY_DisableLink(phy_handle_t *handle)
{
    return PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, 0U, PHY_EXTENDED_CTRL_LINK_CTRL_MASK);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_RMR
 * Description   : Reads and modifies a register of the specified PHY device.
 * Implements    : PHY_RMR_Activity
 *
 *END**************************************************************************/
static status_t PHY_RMR(phy_handle_t *handle, uint8_t registerAddr, uint16_t value, uint16_t mask)
{
    status_t phyStatus;
    uint32_t reg;

    /* Read */
    phyStatus = PHY_Read(handle, registerAddr, &reg);
    reg &=0x0000FFFF;
    if (phyStatus == kStatus_Success)
    {
        /* Modify */
        reg &= (uint16_t)~mask;           /* Clear masked bits */
        reg |= (uint16_t)(mask & value);  /* Set masked bits to value */

        /* Write */
        phyStatus = PHY_Write(handle, registerAddr, reg);
    }

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_GoFromStandbyToNormalMode
 * Description   : Requests the specified PHY device to enter normal mode.
 *
 *END**************************************************************************/
static status_t PHY_GoFromStandbyToNormalMode(phy_handle_t *handle)
{
    status_t phyStatus;
    uint32_t generalStatus;
    uint32_t phyMode;

    phyStatus = PHY_Read(handle, PHY_EXTENDED_CTRL_REG, &phyMode);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    if ((phyMode & PHY_EXTENDED_CTRL_POWER_MODE_MASK) == PHY_EXTENDED_CTRL_POWER_MODE_NORMAL_REQ)
    {
        return kStatus_Success;
    }
    
    phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, PHY_EXTENDED_CTRL_POWER_MODE_NORMAL_REQ, PHY_EXTENDED_CTRL_POWER_MODE_MASK);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }
    do
    {
        phyStatus = PHY_Read(handle, PHY_GENERAL_STATUS_REG, &generalStatus);
        /* Wait for PLL to be locked */
    } while (((generalStatus & PHY_GENERAL_STATUS_PLL_LOCKED_MASK) == 0U) && (phyStatus == kStatus_Success));

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_EnableInterrupts
 * Description   : Enables the interrupts for the specified PHY device.
 *
 *END**************************************************************************/
static status_t PHY_EnableInterrupts(phy_handle_t *handle)
{
    uint16_t irqs;

    irqs = (uint16_t)(PHY_IRQ_LINKUP_MASK | PHY_IRQ_LINKDOWN_MASK 
                      | PHY_IRQ_WAKEUP_MASK | PHY_IRQ_PWON_MASK);

    return PHY_RMR(handle, PHY_IRQ_ENABLE_REG, irqs, irqs);
}

 /*FUNCTION**********************************************************************
 *
 * Function Name : PHY_getID
 * Description   : Gets the ID of the specified PHY device.
 *
 *END**************************************************************************/
static status_t PHY_GetID(phy_handle_t *handle, phy_id_t *id)
{
    status_t phyStatus;
    uint32_t reg;
    uint32_t oui3to18;
    uint32_t oui19to24;

    /* Get PHY ID from device */
    phyStatus = PHY_Read(handle, PHY_ID1_REG, &oui3to18);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }
    phyStatus = PHY_Read(handle, PHY_ID2_REG, &reg);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    oui19to24      = (uint16_t)((reg & ID_2_OUI_MASK)  >> ID_2_OUI_SHIFT);
    id->typeNo     = (uint8_t)((reg & ID_2_TYPE_MASK) >> ID_2_TYPE_SHIFT);
    id->revisionNo = (uint8_t)(reg & ID_2_REV_MASK);

    /* construct OUI */
    id->oui  = (uint32_t) oui19to24;
    id->oui |= ((uint32_t) oui3to18) << OUI_3_TO_18_SHIFT;
    
    return kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_HandleInterruptPwon
 * Description   : Handles the power-on interrupt for the specified PHY device.
 *
 *END**************************************************************************/
static status_t PHY_HandleInterruptPwon(phy_handle_t *handle)
{
    status_t phyStatus;

    /* After power-on, make the necessary settings to the device */
    phyStatus = PHY_EnableInterrupts(handle);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    phyStatus = PHY_GoFromStandbyToNormalMode(handle);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    phyStatus = PHY_EnableLink(handle);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_HandleInterruptWakeUp
 * Description   : Handles the wakeup interrupt for the specified PHY device.
 *
 *END**************************************************************************/
static status_t PHY_HandleInterruptWakeUp(phy_handle_t *handle)
{
    status_t phyStatus;

    phyStatus = PHY_GoFromStandbyToNormalMode(handle);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }
    phyStatus = PHY_EnableLink(handle);

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_HandleInterruptLinkDown
 * Description   : Handles the link down interrupt for the specified PHY device.
 *
 *END**************************************************************************/
static void PHY_HandleInterruptLinkDown(phy_handle_t *handle)
{
    
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_HandleInterruptLinkUp
 * Description   : Handles the link up interrupt for the specified PHY device.
 *
 *END**************************************************************************/
static status_t PHY_HandleInterruptLinkUp(phy_handle_t *handle)
{
    status_t phyStatus;
    uint32_t reg;
    bool linkUp;

    /* Check if the link is still up. Do not report link up if it has gone down meanwhile */
    phyStatus = PHY_Read(handle, PHY_COMM_STATUS_REG, &reg);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }    

    linkUp = ((reg & PHY_COMM_STATUS_LINKUP_FLAG) != 0U) ? (true) : (false);

    if (linkUp == true)
    {
        /* Can set a callback to control the led */
    }

    return kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_HandleInterrupt
 * Description   : Handles the interrupts for the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_HandleInterrupt(phy_handle_t *handle)
{
    status_t phyStatus;
    uint32_t irqs;

    phyStatus = PHY_Read(handle, PHY_IRQ_STATUS_REG, &irqs);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    /* Interrupt handling */
    if ((irqs & PHY_IRQ_PWON_MASK) != 0U)
    {
        phyStatus = PHY_HandleInterruptPwon(handle);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
    }

    if ((irqs & PHY_IRQ_WAKEUP_MASK) != 0U)
    {
        phyStatus = PHY_HandleInterruptWakeUp(handle);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
    }

    if ((irqs & PHY_IRQ_LINKDOWN_MASK) != 0U)
    {
        PHY_HandleInterruptLinkDown(handle);
    }

    if ((irqs & PHY_IRQ_LINKUP_MASK) != 0U)
    {
        phyStatus = PHY_HandleInterruptLinkUp(handle);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
    }

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : init
 * Description   : Initializes the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_TJA110x_Init(phy_handle_t *handle, const phy_config_t *config)
{
    phy_id_t idReg;
    status_t result  = kStatus_Success;

    /* Init MDIO interface. */
    MDIO_Init(handle->mdioHandle);

    /* Initialization after PHY stars to work. */
    result = PHY_GetID(handle, &idReg);
    if (result != kStatus_Success)
    {
        return result;
    }

    result = ((idReg.oui != OUI) ? (kStatus_Fail) : (kStatus_Success));

    if (result == kStatus_Success)
    {
        /* Handle any interrupts pending before initialization */
        return PHY_HandleInterrupt(handle);
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_TJA110x_GetLinkStatus
 * Description   : Gets the link status for the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_TJA110x_GetLinkStatus(phy_handle_t *handle, bool* linkUp)
{
    status_t phyStatus;
    uint32_t reg;

    phyStatus = PHY_Read(handle, PHY_BASIC_STATUS_REG, &reg);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    *linkUp = ((reg & PHY_BASIC_STATUS_LINK_MASK) != 0U) ? (true) : (false);
    *linkUp = ((reg & PHY_BASIC_STATUS_REMOTE_FAULT_MASK) != 0U) ? (false) : (*linkUp);

    return kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_TJA110x_GetLinkSpeed
 * Description   : Gets the link speed for the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_TJA110x_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    status_t phyStatus;
    uint32_t reg;

    phyStatus = PHY_Read(handle, PHY_BASIC_CONTROL_REG, &reg);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    if ((reg & PHY_BASIC_CONTROL_SPEED_MSB_MASK) != 0U)
    {
        *speed = kPHY_Speed1000M;
        if ((reg & PHY_BASIC_CONTROL_SPEED_LSB_MASK) != 0U)
        {
            /*reserved*/
        }
    }
    else
    {
        *speed = kPHY_Speed10M;
        if ((reg & PHY_BASIC_CONTROL_SPEED_LSB_MASK) != 0U)
        {
            *speed = kPHY_Speed100M;
        }
    }

    return kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_Sleep
 * Description   : Sends a sleep request to the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_Sleep(phy_handle_t *handle)
{
    return PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, 
                   PHY_EXTENDED_CTRL_POWER_MODE_SLEEP_REQ, PHY_EXTENDED_CTRL_POWER_MODE_MASK);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_Wakeup
 * Description   : Wakes up the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_Wakeup(phy_handle_t *handle)
{
    status_t phyStatus;

    uint32_t extCtrl;

    phyStatus = PHY_Read(handle, PHY_EXTENDED_CTRL_REG, &extCtrl);

    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    if ((extCtrl & PHY_EXTENDED_CTRL_POWER_MODE_MASK) == PHY_EXTENDED_CTRL_POWER_MODE_SLEEP)
    {
        phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, PHY_EXTENDED_CTRL_POWER_MODE_STANDBY,
                            PHY_EXTENDED_CTRL_POWER_MODE_MASK);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
    }
    else
    {
        phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, PHY_EXTENDED_CTRL_WAKE_REQ_MASK,
                            PHY_EXTENDED_CTRL_WAKE_REQ_MASK);
    }

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_SetLoopback
 * Description   : Sets the loopback mode for the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_SetLoopback(phy_handle_t *handle, phy_loopback_t loopbackMode)
{
    status_t phyStatus = kStatus_Success;
    uint16_t extCtrl = 0;
    uint16_t basicCtrl = PHY_BASIC_CTRL_LOOPBACK_MASK;

    switch (loopbackMode)
    {
        case PHY_LOOPBACK_INTERNAL:
            extCtrl = PHY_EXTENDED_CTRL_LOOPBACK_MODE_INTERNAL;
            break;
        case PHY_LOOPBACK_EXTERNAL:
            extCtrl = PHY_EXTENDED_CTRL_LOOPBACK_MODE_EXTERNAL;
            break;
        case PHY_LOOPBACK_REMOTE:
            extCtrl = PHY_EXTENDED_CTRL_LOOPBACK_MODE_REMOTE;
            break;
        case PHY_LOOPBACK_NONE:
            extCtrl = 0;
            basicCtrl = 0;
            break;
        default:
            phyStatus = kStatus_Fail;
            break;
    }

    if (phyStatus == kStatus_Success)
    {
        /* Disable link control firstly */
        phyStatus = PHY_DisableLink(handle);
        
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
        /* Set loopback mode :internal/external/remote */
        phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, extCtrl, PHY_EXTENDED_CTRL_LOOPBACK_MODE_MASK);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
        /* Enable loopback mode */
        phyStatus = PHY_RMR(handle, PHY_BASIC_CONTROL_REG, basicCtrl, PHY_BASIC_CTRL_LOOPBACK_MASK);
        if (phyStatus != kStatus_Success)
        {
            return phyStatus;
        }
        /* Enable link control */
        phyStatus = PHY_EnableLink(handle);
    }

    return phyStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PHY_Resume
 * Description   : Resumes the specified PHY device.
 *
 *END**************************************************************************/
status_t PHY_Resume(phy_handle_t *handle)
{
    status_t phyStatus;

    phyStatus = PHY_RMR(handle, PHY_BASIC_CONTROL_REG, 0, PHY_BASIC_CONTROL_PDOWN_MASK);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    phyStatus = PHY_GoFromStandbyToNormalMode(handle);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }

    return PHY_EnableLink(handle);
}

status_t PHY_EnableManageOP(phy_handle_t *handle)
{
    return PHY_RMR(handle, PHY_COMM_CONFIG_REG, 0, 0x8000U);
}

status_t PHY_TJA110x_SetMasterSlave(phy_handle_t *handle, phy_device_mode_t mode)
{
    status_t phyStatus;
    
    /* Enable configuration register access */
    phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, PHY_EXTENDED_CTRL_CONFIG_EN_MASK, PHY_EXTENDED_CTRL_CONFIG_EN_MASK);
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }    

    /* Set mode */
    if(mode == PHY_SLAVE_MODE)
    {
        phyStatus = PHY_RMR(handle, PHY_CONFIG1_REG, 0, PHY_CONFIG1_MASTERSLAVE_MASK);
    }
    else
    {
        phyStatus = PHY_RMR(handle, PHY_CONFIG1_REG, PHY_CONFIG1_MASTERSLAVE_MASK, PHY_CONFIG1_MASTERSLAVE_MASK);        
    }
    if (phyStatus != kStatus_Success)
    {
        return phyStatus;
    }
    
    /* Disable configuration register access */
    phyStatus = PHY_RMR(handle, PHY_EXTENDED_CTRL_REG, 0, PHY_EXTENDED_CTRL_CONFIG_EN_MASK);

    return phyStatus;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
