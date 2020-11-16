/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _I2C_EEPROM_H_
#define _I2C_EEPROM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* @Brief Cluster EEPROM configuration */
#define EEPROM_I2C_BASSADDR LPI2C1
/* Clock divider for LPI2C clock source */
#define EEPROM_I2C_CLOCK_FREQ (CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c1))
#define EEPROM_I2C_INSTANCE 1U
#define EEPROM_BAUDRATE     1000000U

#define EEPROM_ADDR       (0xAF >> 1U)
#define EEPROM_ADDR_SIZE  2U
#define EEPROM_PAGE_SIZE  32U

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Initializes the EEPROM with the user configuration structure.
 */
void EEPROM_I2C_Init(void);

/*!
 * @brief Deinitializes the EEPROM LPI2C.
 */
void EEPROM_I2C_Deinit(void);

/*!
 * @brief Write multi bytes across page in address of EEPROM.
 *
 * Users can write a page or at least a byte data into EEPROM address.
 *
 * @param wBuf        Data need be write.
 * @param wAddr       EEPROM write address.
 * @param size        Number of bytes to write.
 */
status_t EEPROM_Write(uint8_t *wBuf, uint16_t wAddr, uint32_t size);

/*!
 * @brief Read multi Bytes from random address of EEPROM
 *
 * @param rBuf      Data readed buf.
 * @param rAddr     EEPROM read address.
 * @param size      Number of bytes to read.
 */
status_t EEPROM_Read(uint8_t *rBuf, uint16_t rAddr, uint32_t size);


#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */


#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_LPUART_H_ */

