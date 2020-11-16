/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _PHY_TJA110X_H_
#define _PHY_TJA110x_H_

#include "fsl_phy.h"

/*!
 * @addtogroup phy_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines the PHY registers. */
#define PHY_BASIC_CONTROL_REG 0x00U      /*!< The PHY basic control register. */
#define PHY_BASIC_STATUS_REG 0x01U       /*!< The PHY basic status register. */
#define PHY_ID1_REG 0x02U                /*!< The PHY ID one register. */
#define PHY_ID2_REG 0x03U                /*!< The PHY ID two register. */
#define PHY_EXTENDED_STATUS_REG 0x0FU    /*!< The PHY extend status register. */
#define PHY_EXTENDED_CTRL_REG 0x11U      /*!< The PHY extend control register. */
#define PHY_CONFIG1_REG 0x12U            /*!< The PHY configuration 1 register. */
#define PHY_IRQ_STATUS_REG 0x15U         /*!< The PHY Interrupt status register. */
#define PHY_IRQ_ENABLE_REG 0x16U         /*!< The PHY Interrupt enable register. */
#define PHY_COMM_STATUS_REG 0x17U        /*!< The PHY Communication status register. */
#define PHY_GENERAL_STATUS_REG 0x18U     /*!< The PHY General status register. */
#define PHY_COMM_CONFIG_REG 0x1B

#define PHY_BASIC_CONTROL_PDOWN_MASK             0x0800U
#define PHY_BASIC_CTRL_LOOPBACK_MASK             0x4000U

#define PHY_EXTENDED_CTRL_LINK_CTRL_MASK         0x8000U
#define PHY_EXTENDED_CTRL_LOOPBACK_MODE_MASK     0x0018U
#define PHY_EXTENDED_CTRL_LOOPBACK_MODE_INTERNAL 0x0000U
#define PHY_EXTENDED_CTRL_LOOPBACK_MODE_EXTERNAL 0x0008U
#define PHY_EXTENDED_CTRL_LOOPBACK_MODE_REMOTE   0x0018U
#define PHY_EXTENDED_CTRL_POWER_MODE_MASK        0x7800U
#define PHY_EXTENDED_CTRL_POWER_MODE_SLEEP_REQ   0x5800U
#define PHY_EXTENDED_CTRL_POWER_MODE_NORMAL_REQ  0x1800U
#define PHY_EXTENDED_CTRL_POWER_MODE_STANDBY     0x6000U
#define PHY_EXTENDED_CTRL_POWER_MODE_SLEEP       0x5000U
#define PHY_EXTENDED_CTRL_WAKE_REQ_MASK          0x0001U
#define PHY_EXTENDED_CTRL_CONFIG_EN_MASK         0x0004U

#define PHY_CONFIG1_MASTERSLAVE_MASK             0x8000U
#define PHY_COMM_CONFIG_AUTO_OP_MASK             0x8000U

#define PHY_BASIC_STATUS_LINK_MASK               0x00000004U
#define PHY_BASIC_STATUS_REMOTE_FAULT_MASK       0x00000010U
#define PHY_IRQ_PWON_MASK                        0x00008000U
#define PHY_IRQ_WAKEUP_MASK                      0x00004000U
#define PHY_IRQ_LINKUP_MASK                      0x00000200U
#define PHY_IRQ_LINKDOWN_MASK                    0x00000400U

#define PHY_BASIC_CONTROL_SPEED_LSB_MASK         0x00002000U
#define PHY_BASIC_CONTROL_SPEED_MSB_MASK         0x00000040U

#define PHY_COMM_STATUS_LINKUP_FLAG              0x00008000U

#define PHY_GENERAL_STATUS_PLL_LOCKED_MASK       0x00004000U


   
#define ID_2_OUI_MASK   0xFC00U
#define ID_2_OUI_SHIFT  10U
#define ID_2_TYPE_MASK  0x03F0U
#define ID_2_TYPE_SHIFT 4U
#define ID_2_REV_MASK   0x000FU

#define OUI_3_TO_18_SHIFT   6U
   
#define OUI                0x006037U /**< Expected OUI across all devices covered by this driver */
   
   
/*!
 * @brief PHY ID.
 *
 * Implements : phy_id_t_Class
 */
typedef struct
{
    uint32_t oui;         /**< 24-bit Organizationally Unique Identifier (OUI) */
    uint8_t  typeNo;      /**< 6-bit manufacturer type number */
    uint8_t  revisionNo;  /**< Four-bit manufacturer revision number */
} phy_id_t;

/*!
 * @brief Loopback modes.
 *
 * Implements : phy_loopback_t_Class
 */
typedef enum
{
    PHY_LOOPBACK_REMOTE   = 3,  /**< Remote loopback */
    PHY_LOOPBACK_EXTERNAL = 2,  /**< External loopback */
    PHY_LOOPBACK_INTERNAL = 1,  /**< Internal loopback */
    PHY_LOOPBACK_NONE     = 0   /**< No loopback */
} phy_loopback_t;

/*!
 * @brief Phy device mode.
 *
 * Implements : phy_device_mode_t_Class
 */
typedef enum
{
    PHY_SLAVE_MODE  = 0,  /**< PHY cofigured as Slave */
    PHY_MASTER_MODE = 1   /**< PHY cofigured as Master */
} phy_device_mode_t;

/*! @brief ENET MDIO operations structure. */
extern const phy_operations_t phytja110x_ops;
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name PHY Driver
 * @{
 */

/*!
 * @brief Initializes PHY.
 *
 *  This function initialize PHY.
 *
 * @param handle       PHY device handle.
 * @param config       Pointer to structure of phy_config_t.
 * @retval kStatus_Success  PHY initializes success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 * @retval kStatus_PHY_AutoNegotiateFail  PHY auto negotiate fail
 */
status_t PHY_TJA110x_Init(phy_handle_t *handle, const phy_config_t *config);

/*!
 * @brief PHY Write function. This function writes data over the SMI to
 * the specified PHY register. This function is called by all PHY interfaces.
 *
 * @param handle       PHY device handle.
 * @param phyReg  The PHY register.
 * @param data    The data written to the PHY register.
 * @retval kStatus_Success     PHY write success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 */
status_t PHY_TJA110x_Write(phy_handle_t *handle, uint32_t phyReg, uint32_t data);

/*!
 * @brief Enables/disables PHY loopback.
 *
 * @param handle     PHY device handle.
 * @param phyReg     The PHY register.
 * @param dataPtr    The data where to save.
 * @retval kStatus_Success  PHY loopback success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 */
status_t PHY_TJA110x_Read(phy_handle_t *handle, uint32_t phyReg, uint32_t *dataPtr);

/*!
 * @brief Enables/disables PHY loopback.
 *
 * @param handle       PHY device handle.
 * @param loopbackMode Set the loopback mode.
 * @retval kStatus_Success  PHY loopback success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 */
status_t PHY_SetLoopback(phy_handle_t *handle, phy_loopback_t loopbackMode);

/*!
 * @brief Gets the PHY link status.
 *
 * @param handle       PHY device handle.
 * @param status   The link up or down status of the PHY.
 *         - true the link is up.
 *         - false the link is down.
 * @retval kStatus_Success   PHY gets link status success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 */
status_t PHY_TJA110x_GetLinkStatus(phy_handle_t *handle, bool *status);

/*!
 * @brief Gets the PHY link speed and duplex.
 *
 * @param handle       PHY device handle.
 * @param speed    The address of PHY link speed.
 * @param duplex   The link duplex of PHY.
 * @retval kStatus_Success   PHY gets link speed and duplex success
 * @retval kStatus_PHY_SMIVisitTimeout  PHY SMI visit time out
 */
status_t PHY_TJA110x_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex);

/*!
 * @brief Set the mode of the PHY device
 *
 * @param handle   PHY device handle.
 * @param mode     The mode of PHY.
 * @retval kStatus_Success   PHY gets link speed and duplex success
 */
status_t PHY_TJA110x_SetMasterSlave(phy_handle_t *handle, phy_device_mode_t mode);

/* @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_PHYKSZ8081_H_ */