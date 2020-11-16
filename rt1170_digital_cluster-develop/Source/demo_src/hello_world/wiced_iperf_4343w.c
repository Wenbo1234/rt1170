/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include "string.h"

#include "fsl_common.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"

#include "logging.h"
   
#include "timers.h"

#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/apps/lwiperf.h"

#include "wwd.h"
#include "wwd_constants.h"
#include "wwd_wiced.h"
#include "wwd_network.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "wwd_events.h"
#include "wwd_wifi.h"
#include "wwd_management.h"
#include "internal/wwd_internal.h"
#include "wwd_wifi_sleep.h"
#include "internal/wwd_sdpcm.h"
#include "network/wwd_buffer_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* logging tool define */
#define sdio_wifi_cprs_bit LOGGING_SDIO_CPRS_BIT

#define sdio_cprs_log_e(...) log_e(sdio_wifi_cprs, ##__VA_ARGS__)
#define sdio_cprs_log_w(...) log_w(sdio_wifi_cprs, ##__VA_ARGS__)
#define sdio_cprs_log_i(...) log_i(sdio_wifi_cprs, ##__VA_ARGS__)
#define sdio_cprs_log_d(...) log_d(sdio_wifi_cprs, ##__VA_ARGS__)

/* Common WiFi parameters */
#ifndef WIFI_SSID
#define WIFI_SSID "NXPOPEN"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "Use4Internet"
#endif

#ifndef WIFI_SECURITY
#define WIFI_SECURITY WICED_SECURITY_WPA2_MIXED_PSK
#endif

#define WICED_WIFI_OPTIMIZE_BANDWIDTH_ROAMING_TRIGGER   ( 1 )
#define WICED_WIFI_ROAMING_TRIGGER_MODE             ( WICED_WIFI_OPTIMIZE_BANDWIDTH_ROAMING_TRIGGER )
#define WICED_WIFI_ROAMING_TRIGGER_DELTA_IN_DBM     ( 5 )
#define WICED_WIFI_ROAMING_SCAN_PERIOD_IN_SECONDS   ( 10 )

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void add_wlan_interface(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static wiced_country_code_t WICED_DEEP_SLEEP_SAVED_VAR( country_code ) = WICED_COUNTRY_UNITED_STATES;

/*******************************************************************************
 * Code
 ******************************************************************************/
wiced_result_t wiced_wlan_connectivity_init( void )
{
    wiced_result_t              result;
    uint32_t                    wlan_rand;
    wiced_bool_t                random_seed_needed = WICED_TRUE;

    wiced_mac_t  mac;
    char         version[200];
    
    result = ( wiced_result_t )wwd_management_wifi_on( country_code );
    /* e.g. user requested an abort or other error */
    if ( result != WICED_SUCCESS )
    {
        sdio_cprs_log_e("Interface initialization FAILED: result %d \r\n", result);
        return result;
    }

    /* get a seed, using hardware on wlan module (if supported); otherwise, seed comes due to difference in cycles for wiced_init completion */
    result = ( wiced_result_t )wwd_wifi_get_iovar_value( IOVAR_STR_RAND, &wlan_rand, WWD_STA_INTERFACE );

    /* Not always a supported operation */
    if ( result == WICED_SUCCESS )
    {
        random_seed_needed = WICED_FALSE;
    }
    /* else generate a seed using elapsed time of wiced_init; at end of function */

    memset( &wlan_rand, 0, sizeof( wlan_rand ) );
    
    wwd_wifi_get_and_cache_mac_address( WWD_STA_INTERFACE );

    if ( wwd_wifi_get_mac_address( &mac, WWD_STA_INTERFACE ) == WWD_SUCCESS )
    {
        sdio_cprs_log_d("WLAN MAC Address:%02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);
    }

    if ( wwd_wifi_get_wifi_version( version, sizeof( version ) ) == WWD_SUCCESS )
    {
        sdio_cprs_log_d("WLAN Firmware:%s\r\n", version);
    }

    /* minimize bootloader usage and start time from UART output */
    if ( wwd_wifi_get_clm_version( version, sizeof( version )) == WWD_SUCCESS )
    {
        sdio_cprs_log_d("WLAN CLM:%s\r\n", version);
    }

    /* Configure roaming parameters */
    wwd_wifi_set_roam_trigger( WICED_WIFI_ROAMING_TRIGGER_MODE );
    wwd_wifi_set_roam_delta( WICED_WIFI_ROAMING_TRIGGER_DELTA_IN_DBM );
    wwd_wifi_set_roam_scan_period( WICED_WIFI_ROAMING_SCAN_PERIOD_IN_SECONDS );

    /* if unable to get random from Wi-Fi iovar, then do seeding here */
    if ( WICED_TRUE == random_seed_needed )
    {
        /* Small amount of variance in cycles taken to complete wiced_init; seed of PRNG */
    }

    return WICED_SUCCESS;
}

void add_wlan_interface(void)
{
    static struct netif fsl_netif0;
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    struct dhcp *pdhcp;

    IP4_ADDR(&fsl_netif0_ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&fsl_netif0_netmask, 0, 0, 0, 0);
    IP4_ADDR(&fsl_netif0_gw, 0, 0, 0, 0);

    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, (void *)WWD_STA_INTERFACE,
              wlanif_init, tcpip_input);

    netif_set_default(&fsl_netif0);
    netif_set_up(&fsl_netif0);

    sdio_cprs_log_i("Getting IP address from DHCP server\r\n");
    dhcp_start(&fsl_netif0);
    pdhcp = (struct dhcp *)netif_get_client_data(&fsl_netif0, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    while (pdhcp->state != DHCP_STATE_BOUND)
    {
        vTaskDelay(1000);
    }

    sdio_cprs_log_d("IPv4 Address got from DHCP  : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0.ip_addr.addr)[0],
           ((u8_t *)&fsl_netif0.ip_addr.addr)[1], ((u8_t *)&fsl_netif0.ip_addr.addr)[2],
           ((u8_t *)&fsl_netif0.ip_addr.addr)[3]);
}

int BOARD_InitNetwork()
{
    wiced_result_t wiced_result = WICED_SUCCESS;
    wwd_result_t wwd_result     = WWD_SUCCESS;

    wiced_ssid_t ap_ssid = {
        .length = sizeof(WIFI_SSID) - 1,
        .value  = WIFI_SSID,
    };

    sdio_cprs_log_i("Initializing WiFi connection... \r\n");
    wiced_result = wiced_wlan_connectivity_init();
    if (wiced_result != WICED_SUCCESS)
    {
        sdio_cprs_log_e("Could not initialize WiFi module %d\r\n", (uint32_t)wiced_result);
        return -1;
    }
    else
    {
        sdio_cprs_log_d("Successfully initialized WiFi module\r\n");
    }
    
    /* Client mode */
    sdio_cprs_log_i("Joining: " WIFI_SSID "\r\n");
    (void)host_rtos_delay_milliseconds((uint32_t)1000);
    wwd_result = wwd_wifi_join(&ap_ssid, WIFI_SECURITY, (uint8_t *)WIFI_PASSWORD, sizeof(WIFI_PASSWORD) - 1, NULL,
                               WWD_STA_INTERFACE);
    if (wwd_result != WWD_SUCCESS)
    {
        sdio_cprs_log_e("Failed to join: " WIFI_SSID "\r\n");
        return -1;
    }
    else
    {
        sdio_cprs_log_d("Successfully joined: " WIFI_SSID "\r\n");
        (void)host_rtos_delay_milliseconds((uint32_t)1000);
        add_wlan_interface();
    }

    return 0;
}


/*!
 * so far the digital cluster board has no power reset for sdmmc driver
 * and the wifi task has no available reset function
 * need pull-out the wifi module or power-on again the board if reset action needed
 */
void iperf_4343w_task(void *arg)
{
    sdio_cprs_log_i("digital cluster 4343w wifi task\r\n");

    tcpip_init(NULL, NULL);
    
    if (BOARD_InitNetwork() != 0)
    {
        sdio_cprs_log_e("No connectivity.\r\n");
        vTaskSuspend(NULL);
    }
    
    while(true)
    {
        vTaskDelay(10000);
    }
}

